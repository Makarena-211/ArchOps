import json
import os
import time

import psycopg2
from confluent_kafka import Consumer

KAFKA_BOOTSTRAP = os.getenv("KAFKA_BOOTSTRAP", "kafka:9092")
KAFKA_TOPIC = os.getenv("KAFKA_TOPIC", "medical.events")
KAFKA_GROUP = os.getenv("KAFKA_GROUP", "readmodel-projector")

PGHOST = os.getenv("PGHOST", "postgres")
PGPORT = int(os.getenv("PGPORT", "5432"))
PGDATABASE = os.getenv("PGDATABASE", "medicine")
PGUSER = os.getenv("PGUSER", "medicine")
PGPASSWORD = os.getenv("PGPASSWORD", "secret")


def pg_connect():
    return psycopg2.connect(
        host=PGHOST, port=PGPORT, dbname=PGDATABASE, user=PGUSER, password=PGPASSWORD
    )


def upsert_read_model(cur, payload):
    cur.execute(
        """
        INSERT INTO medical_records_read(record_id, patient_id, doctor_id, diagnosis, notes, created_at)
        VALUES (%s,%s,%s,%s,%s,%s::timestamptz)
        ON CONFLICT (record_id) DO UPDATE SET
          patient_id=EXCLUDED.patient_id,
          doctor_id=EXCLUDED.doctor_id,
          diagnosis=EXCLUDED.diagnosis,
          notes=EXCLUDED.notes,
          created_at=EXCLUDED.created_at
        """,
        (
            payload["recordId"],
            payload["patientId"],
            payload["doctorId"],
            payload["diagnosis"],
            payload["notes"],
            payload["createdAt"],
        ),
    )


def delete_read_model(cur, payload):
    cur.execute("DELETE FROM medical_records_read WHERE record_id=%s", (payload["recordId"],))


def main():
    c = Consumer(
        {
            "bootstrap.servers": KAFKA_BOOTSTRAP,
            "group.id": KAFKA_GROUP,
            "auto.offset.reset": "earliest",
            "enable.auto.commit": False,
        }
    )
    c.subscribe([KAFKA_TOPIC])

    conn = None
    while conn is None:
        try:
            conn = pg_connect()
        except Exception as e:
            print("Waiting for Postgres...", e)
            time.sleep(1)

    conn.autocommit = False
    print(f"Consumer started. topic={KAFKA_TOPIC} group={KAFKA_GROUP}")

    try:
        while True:
            msg = c.poll(1.0)
            if msg is None:
                continue
            if msg.error():
                print("Kafka error:", msg.error())
                continue

            try:
                event = json.loads(msg.value().decode("utf-8"))
                et = event.get("eventType")
                payload = event.get("payload") or {}

                with conn.cursor() as cur:
                    if et in ("MedicalRecordCreated", "MedicalRecordUpdated"):
                        upsert_read_model(cur, payload)
                    elif et == "MedicalRecordDeleted":
                        delete_read_model(cur, payload)
                    else:
                        # ignore unknown
                        pass
                conn.commit()
                c.commit(message=msg, asynchronous=False)
            except Exception as e:
                conn.rollback()
                print("Processing failed, will retry (no commit):", e)
    finally:
        c.close()
        if conn:
            conn.close()


if __name__ == "__main__":
    main()
