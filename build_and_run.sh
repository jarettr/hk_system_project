#!/bin/bash
APP_NAME="hk_system"

echo "🧹 Очистка..."
rm -rf build && mkdir build

echo "🏗 Запуск сборки..."
docker run --rm \
    -v $(pwd):/host \
    -w /host/build \
    trustworthysystems/camkes:latest \
    bash -c "
        echo '📦 Установка зависимостей с проверкой связи...' && \
        # Добавляем флаги для автоматического повтора при ошибках сети
        apt-get update && \
        DEBIAN_FRONTEND=noninteractive apt-get install -y \
            -o Acquire::Retries=3 \
            haskell-stack llvm-dev libclang-dev && \
        
        pip install ply==3.11 plyplus==0.7.5 aenum sortedcontainers ordered-set jinja2 six && \
        
        ../init-build.sh \
            -DPLATFORM=x86_64 \
            -DCAMKES_APP=$APP_NAME \
            -DSIMULATION=TRUE && \
        ninja
    "
