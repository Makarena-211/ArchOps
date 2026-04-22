from __future__ import annotations

import json
import time
from dataclasses import dataclass
from typing import Any, Dict, List, Optional, Tuple

import psycopg2
import requests


def pretty(obj: Any) -> str:
    return json.dumps(obj, ensure_ascii=False, indent=2)


@dataclass
class ApiResponse:
    status: int
    body: Any
    text: str
    headers: Dict[str, str]


def show(title: str, resp: ApiResponse) -> None:
    print("\n" + "=" * 110)
    print(title)
    print(f"HTTP {resp.status}")
    if resp.body is not None:
        print(pretty(resp.body))
    else:
        print(resp.text)


def assert_http(resp: ApiResponse, expected: int | Tuple[int, ...], title: str) -> None:
    if isinstance(expected, int):
        ok = resp.status == expected
        exp = str(expected)
    else:
        ok = resp.status in expected
        exp = ", ".join(map(str, expected))

    if not ok:
        raise RuntimeError(
            f"[FAIL] {title}: expected HTTP {exp}, got {resp.status}. Body/Text: {resp.body or resp.text}"
        )


class MyServiceClient:
    def __init__(self, base_url: str = "http://localhost:41519/", timeout: float = 10.0):
        self.base_url = base_url.rstrip("/")
        self.timeout = timeout

    def call_endpoint(
        self,
        method: str,
        path: str,
        *,
        token: Optional[str] = None,
        params: Optional[Dict[str, Any]] = None,
        json_body: Optional[Dict[str, Any]] = None,
    ) -> ApiResponse:
        url = f"{self.base_url}{path}"
        headers: Dict[str, str] = {"Accept": "application/json"}
        if token:
            headers["Authorization"] = f"Bearer {token}"

        r = requests.request(
            method=method,
            url=url,
            headers=headers,
            params=params,
            json=json_body,
            timeout=self.timeout,
        )

        try:
            body = r.json()
        except Exception:
            body = None

        return ApiResponse(
            status=r.status_code,
            body=body,
            text=r.text,
            headers=dict(r.headers),
        )

    # system
    def ping(self) -> ApiResponse:
        return self.call_endpoint("GET", "/ping")

    def health(self) -> ApiResponse:
        return self.call_endpoint("GET", "/health")

    # auth
    def register(self, email: str, password: str) -> ApiResponse:
        return self.call_endpoint(
            "POST",
            "/api/auth/register",
            json_body={"email": email, "password": password},
        )

    def login(self, email: str, password: str) -> ApiResponse:
        return self.call_endpoint(
            "POST",
            "/api/auth/login",
            json_body={"email": email, "password": password},
        )

    # old records API (kept)
    def create_record_old(
        self,
        *,
        token: str,
        patient_id: int,
        doctor_id: int,
        diagnosis: str,
        notes: str,
    ) -> ApiResponse:
        return self.call_endpoint(
            "POST",
            "/api/records",
            token=token,
            json_body={
                "patientId": patient_id,
                "doctorId": doctor_id,
                "diagnosis": diagnosis,
                "notes": notes,
            },
        )

    def list_records_old(self, *, limit: int = 50, offset: int = 0) -> ApiResponse:
        return self.call_endpoint("GET", "/api/records", params={"limit": limit, "offset": offset})

    def put_record_old(self, *, record_id: int, diagnosis: str, notes: str) -> ApiResponse:
        return self.call_endpoint(
            "PUT",
            f"/api/records/{record_id}",
            json_body={"diagnosis": diagnosis, "notes": notes},
        )

    def patch_record_old(
        self,
        *,
        record_id: int,
        diagnosis: Optional[str] = None,
        notes: Optional[str] = None,
    ) -> ApiResponse:
        body: Dict[str, Any] = {}
        if diagnosis is not None:
            body["diagnosis"] = diagnosis
        if notes is not None:
            body["notes"] = notes

        return self.call_endpoint(
            "PATCH",
            f"/api/records/{record_id}",
            json_body=body,
        )

    def delete_record_old(self, *, token: str, record_id: int) -> ApiResponse:
        return self.call_endpoint("DELETE", f"/api/records/{record_id}", token=token)

    # NEW: patients
    def create_patient_profile(
        self,
        *,
        token: str,
        first_name: str,
        last_name: str,
        middle_name: Optional[str] = None,
        birth_date: Optional[str] = None,  # YYYY-MM-DD
    ) -> ApiResponse:
        body: Dict[str, Any] = {"firstName": first_name, "lastName": last_name}
        if middle_name is not None:
            body["middleName"] = middle_name
        if birth_date is not None:
            body["birthDate"] = birth_date
        return self.call_endpoint("POST", "/api/patients", token=token, json_body=body)

    def search_patients(self, *, mask: str, limit: int = 50, offset: int = 0) -> ApiResponse:
        return self.call_endpoint(
            "GET",
            "/api/patients/search",
            params={"mask": mask, "limit": limit, "offset": offset},
        )

    # NEW: patient records
    def add_record_to_patient(
        self,
        *,
        token: str,
        patient_id: int,
        diagnosis: str,
        notes: str,
    ) -> ApiResponse:
        return self.call_endpoint(
            "POST",
            f"/api/patients/{patient_id}/records",
            token=token,
            json_body={"diagnosis": diagnosis, "notes": notes},
        )

    def list_patient_records(
        self, *, patient_id: int, limit: int = 50, offset: int = 0
    ) -> ApiResponse:
        return self.call_endpoint(
            "GET",
            f"/api/patients/{patient_id}/records",
            params={"limit": limit, "offset": offset},
        )

    # NEW: record by id
    def get_record_by_id(self, *, record_id: int) -> ApiResponse:
        return self.call_endpoint("GET", f"/api/records/{record_id}")

    def get_user_by_login(self, *, login: str) -> ApiResponse:
        return self.call_endpoint("GET", "/api/users/by-login", params={"login": login})

    def search_users_by_name(self, *, mask: str, limit: int = 50, offset: int = 0) -> ApiResponse:
        return self.call_endpoint(
            "GET", "/api/users/search", params={"mask": mask, "limit": limit, "offset": offset}
        )

    # ===== Mongo records API =====
    def mongo_create_record(
        self,
        *,
        token: str,
        patient_oid: str,
        doctor_oid: str,
        diagnosis: str,
        notes: str,
    ) -> ApiResponse:
        return self.call_endpoint(
            "POST",
            "/api/mongo/records",
            token=token,
            json_body={
                "patientId": patient_oid,
                "doctorId": doctor_oid,
                "diagnosis": diagnosis,
                "notes": notes,
            },
        )

    def mongo_list_records(self, *, limit: int = 50, offset: int = 0) -> ApiResponse:
        return self.call_endpoint(
            "GET", "/api/mongo/records", params={"limit": limit, "offset": offset}
        )

    def mongo_get_record(self, *, record_oid: str) -> ApiResponse:
        return self.call_endpoint("GET", f"/api/mongo/records/{record_oid}")

    def mongo_put_record(self, *, record_oid: str, diagnosis: str, notes: str) -> ApiResponse:
        return self.call_endpoint(
            "PUT",
            f"/api/mongo/records/{record_oid}",
            json_body={"diagnosis": diagnosis, "notes": notes},
        )

    def mongo_patch_record(
        self,
        *,
        record_oid: str,
        diagnosis: Optional[str] = None,
        notes: Optional[str] = None,
    ) -> ApiResponse:
        body: Dict[str, Any] = {}
        if diagnosis is not None:
            body["diagnosis"] = diagnosis
        if notes is not None:
            body["notes"] = notes
        return self.call_endpoint(
            "PATCH",
            f"/api/mongo/records/{record_oid}",
            json_body=body,
        )

    def mongo_delete_record(self, *, token: str, record_oid: str) -> ApiResponse:
        return self.call_endpoint("DELETE", f"/api/mongo/records/{record_oid}", token=token)


def extract_token_or_none(login_resp: ApiResponse) -> Optional[str]:
    if login_resp.body and isinstance(login_resp.body, dict):
        token = login_resp.body.get("accessToken")
        if isinstance(token, str) and token:
            return token
    return None


@dataclass
class DbConfig:
    host: str = "localhost"
    port: int = 5432
    dbname: str = "medicine"
    user: str = "medicine"
    password: str = "secret"


def db_connect(cfg: DbConfig):
    return psycopg2.connect(
        host=cfg.host,
        port=cfg.port,
        dbname=cfg.dbname,
        user=cfg.user,
        password=cfg.password,
    )


def db_set_role(cfg: DbConfig, *, email: str, role: str) -> Tuple[int, str]:
    if role not in ("patient", "doctor", "admin"):
        raise ValueError("role must be one of: patient, doctor, admin")

    conn = db_connect(cfg)
    try:
        conn.autocommit = True
        with conn.cursor() as cur:
            cur.execute(
                "UPDATE users SET role=%s WHERE email=%s RETURNING id, role", (role, email)
            )
            row = cur.fetchone()
            if not row:
                raise RuntimeError(f"User not found by email: {email}")
            return int(row[0]), str(row[1])
    finally:
        conn.close()


def db_list_users(cfg: DbConfig) -> List[Dict[str, Any]]:
    conn = db_connect(cfg)
    try:
        with conn.cursor() as cur:
            cur.execute("SELECT id, email, role, created_at FROM users ORDER BY id ASC")
            rows = cur.fetchall()
            out: List[Dict[str, Any]] = []
            for uid, email, role, created_at in rows:
                out.append(
                    {
                        "id": int(uid),
                        "email": str(email),
                        "role": str(role),
                        "created_at": str(created_at),
                    }
                )
            return out
    finally:
        conn.close()


def run_all() -> None:
    client = MyServiceClient(base_url="http://localhost:8080/", timeout=10.0)
    db_cfg = DbConfig(host="postgres", port=5432)

    # 1) system
    resp = client.ping()
    show("GET /ping", resp)
    assert_http(resp, 200, "GET /ping")

    resp = client.health()
    show("GET /health", resp)
    assert_http(resp, 200, "GET /health")

    # 2) register patient-user
    uniq = int(time.time())
    email = f"user{uniq}@example.com"
    password = "1234"

    reg = client.register(email=email, password=password)
    show("POST /api/auth/register", reg)
    assert_http(reg, (200, 409), "POST /api/auth/register")

    # 3) set role -> doctor (so we can create records)
    users_before = db_list_users(db_cfg)
    print("\nUsers in DB (snapshot):")
    print(pretty(users_before))

    user_id, new_role = db_set_role(db_cfg, email=email, role="doctor")
    print(f"\nDB: updated role for email={email} -> user_id={user_id}, role={new_role}")

    # 4) login (doctor)
    login = client.login(email=email, password=password)
    show("POST /api/auth/login", login)
    assert_http(login, 200, "POST /api/auth/login")

    doctor_token = extract_token_or_none(login)
    if not doctor_token:
        raise RuntimeError("No accessToken in login response")

    # 5) Create second user as PATIENT and login as patient to create profile
    patient_email = f"patient{uniq}@example.com"
    patient_password = "1234"

    reg2 = client.register(email=patient_email, password=patient_password)
    show("POST /api/auth/register (patient user)", reg2)
    assert_http(reg2, (200, 409), "POST /api/auth/register (patient user)")

    # ensure role patient
    db_set_role(db_cfg, email=patient_email, role="patient")

    patient_login = client.login(email=patient_email, password=patient_password)
    show("POST /api/auth/login (patient)", patient_login)
    assert_http(patient_login, 200, "POST /api/auth/login (patient)")

    patient_token = extract_token_or_none(patient_login)
    if not patient_token:
        raise RuntimeError("No accessToken in patient login response")

    # 6) Create patient profile (POST /api/patients)
    create_patient = client.create_patient_profile(
        token=patient_token,
        first_name="Иван",
        last_name="Иванов",
        middle_name="Иванович",
        birth_date="1990-01-10",
    )
    show("POST /api/patients (create patient profile)", create_patient)
    assert_http(create_patient, (200, 409), "POST /api/patients")

    patient_id: Optional[int] = None
    if isinstance(create_patient.body, dict) and "patientId" in create_patient.body:
        patient_id = int(create_patient.body["patientId"])

    # 6.1) Get user by login (login == email)
    u_by_login = client.get_user_by_login(login=patient_email)
    show("GET /api/users/by-login?login=<patient_email>", u_by_login)
    assert_http(u_by_login, 200, "GET /api/users/by-login")

    # 6.2) Search users by name mask (through patients join)
    u_search = client.search_users_by_name(mask="Иван", limit=10, offset=0)
    show("GET /api/users/search?mask=Иван", u_search)
    assert_http(u_search, 200, "GET /api/users/search")

    # 7) Search patients by mask
    search = client.search_patients(mask="Иван", limit=10, offset=0)
    show("GET /api/patients/search?mask=Иван", search)
    assert_http(search, 200, "GET /api/patients/search")

    if patient_id is None:
        if isinstance(search.body, dict) and isinstance(search.body.get("items"), list):
            for item in search.body["items"]:
                if isinstance(item, dict) and item.get("email") == patient_email:
                    patient_id = int(item["patientId"])
                    break

    if patient_id is None:
        raise RuntimeError("Could not determine patientId (neither from create nor from search)")

    # 8) Add medical record to patient (doctor token)
    add_rec = client.add_record_to_patient(
        token=doctor_token,
        patient_id=patient_id,
        diagnosis="ОРВИ",
        notes="Покой, обильное питьё",
    )
    show(f"POST /api/patients/{patient_id}/records (add record)", add_rec)
    assert_http(add_rec, 200, f"POST /api/patients/{patient_id}/records")

    # 9) List patient records (history)
    hist = client.list_patient_records(patient_id=patient_id, limit=10, offset=0)
    show(f"GET /api/patients/{patient_id}/records (history)", hist)
    assert_http(hist, 200, f"GET /api/patients/{patient_id}/records")

    # 10) OLD endpoints check (create/list/put/patch/delete)
    create_old = client.create_record_old(
        token=doctor_token,
        patient_id=patient_id,
        doctor_id=user_id,
        diagnosis="Flu-old",
        notes="Old endpoint check",
    )
    show("POST /api/records (OLD create)", create_old)
    assert_http(create_old, 200, "POST /api/records (OLD create)")

    list_old = client.list_records_old(limit=10, offset=0)
    show("GET /api/records (OLD list)", list_old)
    assert_http(list_old, 200, "GET /api/records (OLD list)")

    old_record_id: Optional[int] = None
    if (
        isinstance(list_old.body, dict)
        and isinstance(list_old.body.get("items"), list)
        and list_old.body["items"]
    ):
        old_record_id = int(list_old.body["items"][0]["id"])

    if old_record_id is None:
        raise RuntimeError("Could not determine recordId for old PUT/PATCH/DELETE flow")

    put_old = client.put_record_old(
        record_id=old_record_id, diagnosis="Updated diagnosis", notes="Updated notes"
    )
    show(f"PUT /api/records/{old_record_id} (OLD put)", put_old)
    assert_http(put_old, 200, f"PUT /api/records/{old_record_id}")

    patch_old = client.patch_record_old(record_id=old_record_id, diagnosis="Patched diagnosis")
    show(f"PATCH /api/records/{old_record_id} (OLD patch)", patch_old)
    assert_http(patch_old, 200, f"PATCH /api/records/{old_record_id}")

    del_forbidden = client.delete_record_old(token=doctor_token, record_id=old_record_id)
    show(f"DELETE /api/records/{old_record_id} (doctor -> expected 403)", del_forbidden)
    assert_http(del_forbidden, (403, 401), f"DELETE /api/records/{old_record_id} as doctor")

    # Promote doctor -> admin and delete
    db_set_role(db_cfg, email=email, role="admin")
    admin_login = client.login(email=email, password=password)
    show("POST /api/auth/login (admin)", admin_login)
    assert_http(admin_login, 200, "POST /api/auth/login (admin)")
    admin_token = extract_token_or_none(admin_login)
    if not admin_token:
        raise RuntimeError("No accessToken in admin login response")

    del_ok = client.delete_record_old(token=admin_token, record_id=old_record_id)
    show(f"DELETE /api/records/{old_record_id} (admin -> expected 200)", del_ok)
    assert_http(del_ok, 200, f"DELETE /api/records/{old_record_id} as admin")

    # =========================
    # MONGO CRUD TEST
    # =========================
    print("\n" + "=" * 110)
    print("MONGO CRUD TEST (/api/mongo/records)")
    print("=" * 110)

    patient_oid = "661e3f9c2b9f0c3a9bd4e701"                                                                                                                        
    doctor_oid = "661e3f9c2b9f0c3a9bd4e702"  
    
    create_m = client.mongo_create_record(
        token=admin_token,
        patient_oid=patient_oid,
        doctor_oid=doctor_oid,
        diagnosis="Mongo-Diagnosis",
        notes="Mongo-Notes",
    )
    show("POST /api/mongo/records (create)", create_m)
    assert_http(create_m, 200, "POST /api/mongo/records")

    if not isinstance(create_m.body, dict) or "recordId" not in create_m.body:
        raise RuntimeError("Mongo create did not return recordId")
    rec_oid = str(create_m.body["recordId"])
    if not rec_oid:
        raise RuntimeError("Mongo recordId is empty")

    lst_m = client.mongo_list_records(limit=10, offset=0)
    show("GET /api/mongo/records (list)", lst_m)
    assert_http(lst_m, 200, "GET /api/mongo/records")

    get_m = client.mongo_get_record(record_oid=rec_oid)
    show(f"GET /api/mongo/records/{rec_oid} (get)", get_m)
    assert_http(get_m, 200, "GET /api/mongo/records/{id}")

    put_m = client.mongo_put_record(record_oid=rec_oid, diagnosis="Mongo-PUT", notes="Mongo-PUT-Notes")
    show(f"PUT /api/mongo/records/{rec_oid} (put)", put_m)
    assert_http(put_m, 200, "PUT /api/mongo/records/{id}")

    patch_m = client.mongo_patch_record(record_oid=rec_oid, diagnosis="Mongo-PATCH")
    show(f"PATCH /api/mongo/records/{rec_oid} (patch)", patch_m)
    assert_http(patch_m, 200, "PATCH /api/mongo/records/{id}")

    del_m = client.mongo_delete_record(token=admin_token, record_oid=rec_oid)
    show(f"DELETE /api/mongo/records/{rec_oid} (delete)", del_m)
    assert_http(del_m, 200, "DELETE /api/mongo/records/{id}")

    get_deleted = client.mongo_get_record(record_oid=rec_oid)
    show(f"GET /api/mongo/records/{rec_oid} (after delete, expected 404)", get_deleted)
    assert_http(get_deleted, 404, "GET /api/mongo/records/{id} after delete")

    print("\nALL CHECKS PASSED (including Mongo CRUD)")


if __name__ == "__main__":
    run_all()