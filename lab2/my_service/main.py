from __future__ import annotations

import json
import time
from dataclasses import dataclass
from typing import Any, Dict, Optional, List, Tuple

import requests
import psycopg2


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


class MyServiceClient:
    def __init__(self, base_url: str = "http://localhost:8080", timeout: float = 10.0):
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
        """
        Универсальная функция под "отдельный эндпоинт":
        можно дергать любые ручки вашего сервиса.
        """
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


    def ping(self) -> ApiResponse:
        return self.call_endpoint("GET", "/ping")

    def health(self) -> ApiResponse:
        return self.call_endpoint("GET", "/health")


    def register_patient(self, email: str, password: str) -> ApiResponse:
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


    def create_record(
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

    def list_records(self, *, limit: int = 50, offset: int = 0) -> ApiResponse:
        return self.call_endpoint("GET", "/api/records", params={"limit": limit, "offset": offset})

    def put_record(self, *, record_id: int, diagnosis: str, notes: str) -> ApiResponse:
        return self.call_endpoint(
            "PUT",
            f"/api/records/{record_id}",
            json_body={"diagnosis": diagnosis, "notes": notes},
        )

    def patch_record(
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

    def delete_record(self, *, token: str, record_id: int) -> ApiResponse:
        return self.call_endpoint("DELETE", f"/api/records/{record_id}", token=token)


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


def db_list_users(cfg: DbConfig) -> List[Dict[str, Any]]:
    """
    Возвращает список пользователей из таблицы users.
    """
    conn = db_connect(cfg)
    try:
        with conn.cursor() as cur:
            cur.execute("SELECT id, email, role, created_at FROM users ORDER BY id ASC")
            rows = cur.fetchall()
            out: List[Dict[str, Any]] = []
            for (uid, email, role, created_at) in rows:
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


def db_set_role(cfg: DbConfig, *, email: str, role: str) -> Tuple[int, str]:
    """
    Меняет роль пользователю по email. Возвращает (user_id, role).
    """
    if role not in ("patient", "doctor", "admin"):
        raise ValueError("role must be one of: patient, doctor, admin")

    conn = db_connect(cfg)
    try:
        conn.autocommit = True
        with conn.cursor() as cur:
            cur.execute("UPDATE users SET role=%s WHERE email=%s RETURNING id, role", (role, email))
            row = cur.fetchone()
            if not row:
                raise RuntimeError(f"User not found by email: {email}")
            return int(row[0]), str(row[1])
    finally:
        conn.close()


def db_set_role_by_id(cfg: DbConfig, *, user_id: int, role: str) -> Tuple[int, str]:
    """
    Меняет роль пользователю по id. Возвращает (user_id, role).
    """
    if role not in ("patient", "doctor", "admin"):
        raise ValueError("role must be one of: patient, doctor, admin")

    conn = db_connect(cfg)
    try:
        conn.autocommit = True
        with conn.cursor() as cur:
            cur.execute("UPDATE users SET role=%s WHERE id=%s RETURNING id, role", (role, user_id))
            row = cur.fetchone()
            if not row:
                raise RuntimeError(f"User not found by id: {user_id}")
            return int(row[0]), str(row[1])
    finally:
        conn.close()



def run_all() -> None:
    # Настройки
    client = MyServiceClient(base_url="http://localhost:8080", timeout=10.0)

    db_cfg = DbConfig(host="localhost", port=5432)

    # 1) базовые эндпоинты
    show("GET /ping", client.ping())
    show("GET /health", client.health())

    # 2) регистрация
    uniq = int(time.time())
    email = f"user{uniq}@example.com"
    password = "1234"

    show("POST /api/auth/register", client.register_patient(email=email, password=password))

    # 3) посмотреть таблицу users
    users_before = db_list_users(db_cfg)
    print("\nUsers in DB (before role change):")
    print(pretty(users_before))

    # 4) сменить роль в БД (например, чтобы тестить защищенные эндпоинты)
    #    create_record у вас разрешен только doctor/admin — поэтому поднимем роль на doctor
    uid, new_role = db_set_role(db_cfg, email=email, role="doctor")
    print(f"\nDB: updated role for email={email} -> user_id={uid}, role={new_role}")

    users_after = db_list_users(db_cfg)
    print("\nUsers in DB (after role change):")
    print(pretty(users_after))

    # 5) логин (ВАЖНО: после смены роли логинимся заново, чтобы JWT содержал новую роль)
    login_resp = client.login(email=email, password=password)
    show("POST /api/auth/login", login_resp)

    token = extract_token_or_none(login_resp)
    if not token:
        print("\nНе удалось получить accessToken из /api/auth/login — дальше records тесты пропускаю.")
        return

    # 6) records: create/list/put/patch/delete
    create_resp = client.create_record(
        token=token,
        patient_id=1,
        doctor_id=uid,  # просто пример: ставим doctorId = id текущего "врача"
        diagnosis="Flu",
        notes="Rest, drink water",
    )
    show("POST /api/records (create)", create_resp)

    list_resp = client.list_records(limit=10, offset=0)
    show("GET /api/records (list)", list_resp)

    # Возьмем id первой записи из списка
    record_id: Optional[int] = None
    if list_resp.body and isinstance(list_resp.body, dict):
        items = list_resp.body.get("items")
        if isinstance(items, list) and items:
            first = items[0]
            if isinstance(first, dict) and "id" in first:
                record_id = int(first["id"])

    if record_id is None:
        print("\nНе нашёл записи для PUT/PATCH/DELETE (возможно create не создал запись).")
        return

    show(
        f"PUT /api/records/{record_id}",
        client.put_record(record_id=record_id, diagnosis="Updated diagnosis", notes="Updated notes"),
    )

    show(
        f"PATCH /api/records/{record_id}",
        client.patch_record(record_id=record_id, diagnosis="Patched diagnosis"),
    )

    show(
        f"DELETE /api/records/{record_id}",
        client.delete_record(token=token, record_id=record_id),
    )


if __name__ == "__main__":
    # pip install requests psycopg2-binary
    run_all()
