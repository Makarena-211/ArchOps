print(`[mongo] Applying validation rules in DB: ${db.getName()}`);

db.runCommand({
  collMod: "users",
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["email", "role", "passwordHash", "createdAt", "isActive"],
      additionalProperties: true,
      properties: {
        email: {
          bsonType: "string",
          description: "User email",
          minLength: 3,
          pattern: "^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$",
        },
        role: {
          bsonType: "string",
          enum: ["patient", "doctor", "admin"],
        },
        passwordHash: {
          bsonType: "string",
          minLength: 3,
        },
        createdAt: { bsonType: "date" },
        isActive: { bsonType: "bool" },
      },
    },
  },
  validationLevel: "moderate",
  validationAction: "error",
});

// patients validation
db.runCommand({
  collMod: "patients",
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: ["userId", "name", "createdAt"],
      additionalProperties: true,
      properties: {
        userId: { bsonType: "objectId" },
        emailSnapshot: { bsonType: "string" },
        name: {
          bsonType: "object",
          required: ["first", "last"],
          properties: {
            first: { bsonType: "string", minLength: 1, maxLength: 64 },
            last: { bsonType: "string", minLength: 1, maxLength: 64 },
            middle: { bsonType: ["string", "null"], maxLength: 64 },
          },
        },
        birthDate: { bsonType: ["date", "null"] },
        contacts: {
          bsonType: ["object", "null"],
          properties: {
            phone: { bsonType: ["string", "null"], pattern: "^\\+?[0-9\\-\\s]{7,20}$" },
            address: { bsonType: ["string", "null"], maxLength: 256 },
          },
        },
        insurance: {
          bsonType: ["object", "null"],
          properties: {
            provider: { bsonType: ["string", "null"], maxLength: 64 },
            policyNumber: { bsonType: ["string", "null"], maxLength: 64 },
          },
        },
        createdAt: { bsonType: "date" },
      },
    },
  },
  validationLevel: "moderate",
  validationAction: "error",
});

db.runCommand({
  collMod: "medical_records",
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: [
        "patientId",
        "doctorUserId",
        "diagnosis",
        "notes",
        "status",
        "createdAt",
        "updatedAt",
      ],
      additionalProperties: true,
      properties: {
        patientId: { bsonType: "objectId" },
        doctorUserId: { bsonType: "objectId" },
        diagnosis: { bsonType: "string", minLength: 1, maxLength: 256 },
        notes: { bsonType: "string", minLength: 1, maxLength: 5000 },
        tags: {
          bsonType: ["array", "null"],
          items: { bsonType: "string", maxLength: 32 },
        },
        status: { bsonType: "string", enum: ["draft", "final", "canceled"] },
        visit: {
          bsonType: ["object", "null"],
          properties: {
            type: { bsonType: "string", enum: ["offline", "online"] },
            clinic: { bsonType: "string", maxLength: 128 },
            room: { bsonType: "string", maxLength: 32 },
          },
        },
        attachments: {
          bsonType: ["array", "null"],
          items: {
            bsonType: "object",
            required: ["name", "contentType", "sizeBytes", "uploadedAt"],
            properties: {
              name: { bsonType: "string", minLength: 1, maxLength: 128 },
              contentType: { bsonType: "string", minLength: 3, maxLength: 128 },
              sizeBytes: { bsonType: "int", minimum: 0, maximum: 104857600 },
              uploadedAt: { bsonType: "date" },
            },
          },
        },
        createdAt: { bsonType: "date" },
        updatedAt: { bsonType: "date" },
      },
    },
  },
  validationLevel: "moderate",
  validationAction: "error",
});

print("[mongo] Validation rules applied");

print("[mongo] Validation test: inserting invalid medical_records (should FAIL)...");
try {
  db.medical_records.insertOne({
    patientId: "NOT_OBJECT_ID",
    doctorUserId: ObjectId(),
    diagnosis: "",
    notes: 123,
    status: "unknown",
    createdAt: new Date(),
    updatedAt: new Date(),
  });
  print("[mongo] ERROR: invalid document inserted, validation did not work");
} catch (e) {
  print("[mongo] OK: validation rejected invalid insert");
  print(e.message);
}