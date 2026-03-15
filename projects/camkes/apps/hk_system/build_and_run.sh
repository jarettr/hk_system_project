#!/bin/bash
APP_NAME="hk_system"

echo "🧹 Удаляем старый билд..."
rm -rf build && mkdir build

echo "📥 Загрузка свежего образа и сборка..."
# Мы принудительно делаем pull, чтобы обновить слои
docker pull trustworthysystems/sel4:latest

docker run --rm \
    -v $(pwd):/host \
    -w /host/build \
    trustworthysystems/sel4:latest \
    bash -c "
        ../init-build.sh \
            -DPLATFORM=x86_64 \
            -DCAMKES_APP=$APP_NAME \
            -DSIMULATION=TRUE && \
        ninja
    "
