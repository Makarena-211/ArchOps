FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg-dev:latest AS builder

WORKDIR /app
COPY . /app

RUN cmake -S . -B build-release -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build-release --target my_service -j

FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg-dev:latest AS runtime

WORKDIR /app

COPY --from=builder /app/build-release/my_service /app/my_service
COPY --from=builder /app/configs /app/configs
COPY --from=builder /app/postgresql /app/postgresql
COPY --from=builder /app/.docker/entrypoint.sh /app/entrypoint.sh

RUN chmod +x /app/entrypoint.sh

EXPOSE 8080

ENTRYPOINT ["/app/entrypoint.sh"]
CMD ["/app/my_service", "--config", "/app/configs/static_config.yaml"]