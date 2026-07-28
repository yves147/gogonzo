# Build
FROM debian:bookworm-slim AS build
RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential make && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN make -C bot clean release

# Runtime
FROM debian:bookworm-slim
WORKDIR /bot
COPY --from=build /src/bot/bot ./bot
ENTRYPOINT ["./bot"]