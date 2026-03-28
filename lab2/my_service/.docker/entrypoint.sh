#!/usr/bin/env bash
set -euo pipefail

echo "Waiting for PostgreSQL at ${PGHOST:-postgres}:${PGPORT:-5432}..."
for i in {1..60}; do
  if (echo > /dev/tcp/${PGHOST:-postgres}/${PGPORT:-5432}) >/dev/null 2>&1; then
    echo "PostgreSQL is accepting connections"
    break
  fi
  sleep 1
done

cat > /app/configs/config_vars.yaml <<EOF
server-port: ${SERVER_PORT:-8080}
logger-level: ${LOGGER_LEVEL:-info}
worker-threads: ${WORKER_THREADS:-2}
worker-fs-threads: ${WORKER_FS_THREADS:-2}
is-testing: ${IS_TESTING:-false}
EOF

exec "$@" --config_vars /app/configs/config_vars.yaml