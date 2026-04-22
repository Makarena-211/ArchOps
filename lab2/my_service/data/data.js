function iso(s) { return new Date(s); }

db.users.drop();
db.patients.drop();
db.medical_records.drop();

db.createCollection("users");
db.createCollection("patients");
db.createCollection("medical_records");

// ---- USERS (>=10) ----
db.users.insertMany([
  { email: "admin@example.com", role: "admin", passwordHash: "sha256:admin", createdAt: iso("2026-04-01T10:00:00Z"), isActive: true },
  { email: "doc1@example.com", role: "doctor", passwordHash: "sha256:doc1", createdAt: iso("2026-04-01T10:01:00Z"), isActive: true },
  { email: "doc2@example.com", role: "doctor", passwordHash: "sha256:doc2", createdAt: iso("2026-04-01T10:02:00Z"), isActive: true },
  { email: "doc3@example.com", role: "doctor", passwordHash: "sha256:doc3", createdAt: iso("2026-04-01T10:03:00Z"), isActive: true },

  { email: "user1@example.com", role: "patient", passwordHash: "sha256:user1", createdAt: iso("2026-04-01T10:10:00Z"), isActive: true },
  { email: "user2@example.com", role: "patient", passwordHash: "sha256:user2", createdAt: iso("2026-04-01T10:11:00Z"), isActive: true },
  { email: "user3@example.com", role: "patient", passwordHash: "sha256:user3", createdAt: iso("2026-04-01T10:12:00Z"), isActive: true },
  { email: "user4@example.com", role: "patient", passwordHash: "sha256:user4", createdAt: iso("2026-04-01T10:13:00Z"), isActive: true },
  { email: "user5@example.com", role: "patient", passwordHash: "sha256:user5", createdAt: iso("2026-04-01T10:14:00Z"), isActive: false },
  { email: "user6@example.com", role: "patient", passwordHash: "sha256:user6", createdAt: iso("2026-04-01T10:15:00Z"), isActive: true },
  { email: "user7@example.com", role: "patient", passwordHash: "sha256:user7", createdAt: iso("2026-04-01T10:16:00Z"), isActive: true },
  { email: "user8@example.com", role: "patient", passwordHash: "sha256:user8", createdAt: iso("2026-04-01T10:17:00Z"), isActive: true },
  { email: "user9@example.com", role: "patient", passwordHash: "sha256:user9", createdAt: iso("2026-04-01T10:18:00Z"), isActive: true },
  { email: "user10@example.com", role: "patient", passwordHash: "sha256:user10", createdAt: iso("2026-04-01T10:19:00Z"), isActive: true }
]);

const u = {};
db.users.find({}).forEach(x => { u[x.email] = x._id; });

// ---- PATIENTS (>=10) ----
db.patients.insertMany([
  { userId: u["user1@example.com"], emailSnapshot: "user1@example.com", name: { first: "Иван", last: "Иванов", middle: "Иванович" }, birthDate: iso("1990-01-10T00:00:00Z"), contacts: { phone: "+7-900-111-22-33", address: "Москва, ул. Пример, 1" }, insurance: { provider: "ОМС", policyNumber: "1000-0000-0001" }, createdAt: iso("2026-04-01T12:00:00Z") },
  { userId: u["user2@example.com"], emailSnapshot: "user2@example.com", name: { first: "Пётр", last: "Петров", middle: "Петрович" }, birthDate: iso("1985-05-20T00:00:00Z"), contacts: { phone: "+7-900-222-22-33", address: "СПб, Невский, 10" }, insurance: { provider: "ДМС", policyNumber: "2000-0000-0002" }, createdAt: iso("2026-04-01T12:01:00Z") },
  { userId: u["user3@example.com"], emailSnapshot: "user3@example.com", name: { first: "Анна", last: "Смирнова", middle: "Олеговна" }, birthDate: iso("1992-11-03T00:00:00Z"), contacts: { phone: "+7-900-333-22-33", address: "Казань, Центр, 5" }, insurance: { provider: "ОМС", policyNumber: "3000-0000-0003" }, createdAt: iso("2026-04-01T12:02:00Z") },
  { userId: u["user4@example.com"], emailSnapshot: "user4@example.com", name: { first: "Мария", last: "Кузнецова", middle: "Игоревна" }, birthDate: iso("1979-07-14T00:00:00Z"), contacts: { phone: "+7-900-444-22-33", address: "Новосибирск, Ленина, 7" }, insurance: { provider: "ОМС", policyNumber: "4000-0000-0004" }, createdAt: iso("2026-04-01T12:03:00Z") },
  { userId: u["user5@example.com"], emailSnapshot: "user5@example.com", name: { first: "Сергей", last: "Попов", middle: "Андреевич" }, birthDate: iso("1988-02-28T00:00:00Z"), contacts: { phone: "+7-900-555-22-33", address: "Екатеринбург, Мира, 3" }, insurance: { provider: "ДМС", policyNumber: "5000-0000-0005" }, createdAt: iso("2026-04-01T12:04:00Z") },
  { userId: u["user6@example.com"], emailSnapshot: "user6@example.com", name: { first: "Елена", last: "Васильева", middle: "Сергеевна" }, birthDate: iso("1995-09-09T00:00:00Z"), contacts: { phone: "+7-900-666-22-33", address: "Пермь, Садовая, 9" }, insurance: { provider: "ОМС", policyNumber: "6000-0000-0006" }, createdAt: iso("2026-04-01T12:05:00Z") },
  { userId: u["user7@example.com"], emailSnapshot: "user7@example.com", name: { first: "Алексей", last: "Морозов", middle: "Викторович" }, birthDate: iso("1981-12-12T00:00:00Z"), contacts: { phone: "+7-900-777-22-33", address: "Томск, Кирова, 11" }, insurance: { provider: "ОМС", policyNumber: "7000-0000-0007" }, createdAt: iso("2026-04-01T12:06:00Z") },
  { userId: u["user8@example.com"], emailSnapshot: "user8@example.com", name: { first: "Ольга", last: "Новикова", middle: "Павловна" }, birthDate: iso("1993-03-30T00:00:00Z"), contacts: { phone: "+7-900-888-22-33", address: "Уфа, Октября, 20" }, insurance: { provider: "ДМС", policyNumber: "8000-0000-0008" }, createdAt: iso("2026-04-01T12:07:00Z") },
  { userId: u["user9@example.com"], emailSnapshot: "user9@example.com", name: { first: "Никита", last: "Фёдоров", middle: "Дмитриевич" }, birthDate: iso("2000-04-04T00:00:00Z"), contacts: { phone: "+7-900-999-22-33", address: "Самара, Победы, 2" }, insurance: { provider: "ОМС", policyNumber: "9000-0000-0009" }, createdAt: iso("2026-04-01T12:08:00Z") },
  { userId: u["user10@example.com"], emailSnapshot: "user10@example.com", name: { first: "Татьяна", last: "Соколова", middle: "Ильинична" }, birthDate: iso("1987-06-06T00:00:00Z"), contacts: { phone: "+7-900-101-22-33", address: "Воронеж, Маяковского, 8" }, insurance: { provider: "ОМС", policyNumber: "9000-0000-0010" }, createdAt: iso("2026-04-01T12:09:00Z") }
]);

const p = {};
db.patients.find({}).forEach(x => { p[x.emailSnapshot] = x._id; });

// ---- MEDICAL RECORDS (>=10) ----
db.medical_records.insertMany([
  { patientId: p["user1@example.com"], doctorUserId: u["doc1@example.com"], diagnosis: "ОРВИ", notes: "Покой, питьё", tags: ["respiratory","cold"], status: "final", visit: { type: "offline", clinic: "Clinic #1", room: "101" }, attachments: [], createdAt: iso("2026-03-10T09:00:00Z"), updatedAt: iso("2026-03-10T09:00:00Z") },
  { patientId: p["user1@example.com"], doctorUserId: u["doc2@example.com"], diagnosis: "Грипп", notes: "Жаропонижающее", tags: ["respiratory","flu"], status: "final", visit: { type: "online", clinic: "Telemed", room: "" }, attachments: [{ name: "analysis.pdf", contentType: "application/pdf", sizeBytes: NumberInt(120034), uploadedAt: iso("2026-03-20T10:00:00Z") }], createdAt: iso("2026-03-20T10:05:00Z"), updatedAt: iso("2026-03-20T10:05:00Z") },
  { patientId: p["user2@example.com"], doctorUserId: u["doc1@example.com"], diagnosis: "Гастрит", notes: "Диета, ИПП 14 дней", tags: ["gastro"], status: "final", visit: { type: "offline", clinic: "Clinic #2", room: "12" }, attachments: [], createdAt: iso("2026-02-10T08:00:00Z"), updatedAt: iso("2026-02-10T08:00:00Z") },
  { patientId: p["user3@example.com"], doctorUserId: u["doc2@example.com"], diagnosis: "Аллергия", notes: "Антигистаминные", tags: ["allergy"], status: "final", visit: { type: "offline", clinic: "Clinic #1", room: "102" }, attachments: [], createdAt: iso("2026-03-01T11:00:00Z"), updatedAt: iso("2026-03-01T11:00:00Z") },
  { patientId: p["user4@example.com"], doctorUserId: u["doc3@example.com"], diagnosis: "Гипертония", notes: "Контроль давления", tags: ["cardio"], status: "final", visit: { type: "offline", clinic: "Clinic #3", room: "5" }, attachments: [], createdAt: iso("2026-01-15T07:30:00Z"), updatedAt: iso("2026-01-15T07:30:00Z") },
  { patientId: p["user5@example.com"], doctorUserId: u["doc1@example.com"], diagnosis: "Травма", notes: "Рентген, покой", tags: ["trauma"], status: "draft", visit: { type: "offline", clinic: "ER", room: "1" }, attachments: [], createdAt: iso("2026-04-10T15:00:00Z"), updatedAt: iso("2026-04-10T15:00:00Z") },
  { patientId: p["user6@example.com"], doctorUserId: u["doc2@example.com"], diagnosis: "COVID-19", notes: "Изоляция", tags: ["respiratory","covid"], status: "final", visit: { type: "online", clinic: "Telemed", room: "" }, attachments: [], createdAt: iso("2025-12-01T16:00:00Z"), updatedAt: iso("2025-12-01T16:00:00Z") },
  { patientId: p["user7@example.com"], doctorUserId: u["doc3@example.com"], diagnosis: "Ангина", notes: "Антибиотик", tags: ["respiratory","throat"], status: "final", visit: { type: "offline", clinic: "Clinic #1", room: "103" }, attachments: [], createdAt: iso("2026-03-28T09:10:00Z"), updatedAt: iso("2026-03-28T09:10:00Z") },
  { patientId: p["user8@example.com"], doctorUserId: u["doc1@example.com"], diagnosis: "Мигрень", notes: "НПВС по требованию", tags: ["neuro"], status: "final", visit: { type: "offline", clinic: "Clinic #2", room: "7" }, attachments: [], createdAt: iso("2026-02-22T13:00:00Z"), updatedAt: iso("2026-02-22T13:00:00Z") },
  { patientId: p["user8@example.com"], doctorUserId: u["doc2@example.com"], diagnosis: "Профосмотр", notes: "Рекомендации", tags: ["checkup"], status: "final", visit: { type: "offline", clinic: "Clinic #2", room: "8" }, attachments: [{ name: "summary.txt", contentType: "text/plain", sizeBytes: NumberInt(1200), uploadedAt: iso("2026-04-02T08:00:00Z") }], createdAt: iso("2026-04-02T08:05:00Z"), updatedAt: iso("2026-04-02T08:05:00Z") }
]);

// ---- Indexes ----
db.users.createIndex({ email: 1 }, { unique: true });
db.patients.createIndex({ userId: 1 }, { unique: true });
db.medical_records.createIndex({ patientId: 1, createdAt: -1 });
db.medical_records.createIndex({ doctorUserId: 1, createdAt: -1 });
db.medical_records.createIndex({ tags: 1 });

print("[mongo] Seed completed");