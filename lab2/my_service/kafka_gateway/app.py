import json
import os
import uuid
from datetime import datetime, timezone

from confluent_kafka import Producer
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel

KAFKA_BOOTSTRAP = os.getenv("KAFKA_BOOTSTRAP", "kafka:9092")
KAFKA_TOPIC = os.getenv("KAFKA_TOPIC", "medical.events")
ACKS = os.getenv("ACKS", "all")

producer = Producer(
    {
        "bootstrap.servers": KAFKA_BOOTSTRAP,
        "acks": ACKS,
    }
)

app = FastAPI()


class PublishReq(BaseModel):
    key: str | None = None
    eventType: str
    payload: dict


@app.post("/publish")
def publish(req: PublishReq):
    event = {
        "eventId": str(uuid.uuid4()),
        "eventType": req.eventType,
        "occurredAt": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "producer": "my_service",
        "payload": req.payload,
    }
    data = json.dumps(event, ensure_ascii=False).encode("utf-8")
    k = (req.key or "").encode("utf-8") if req.key is not None else None

    delivered = {"ok": False, "err": None}

    def cb(err, msg):
        if err:
            delivered["err"] = str(err)
        else:
            delivered["ok"] = True

    producer.produce(KAFKA_TOPIC, value=data, key=k, callback=cb)
    producer.flush(5.0)

    if not delivered["ok"]:
        raise HTTPException(status_code=502, detail=f"Kafka publish failed: {delivered['err']}")
    return {"status": "ok", "topic": KAFKA_TOPIC}
