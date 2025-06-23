#!/bin/bash

# 通过用户名判断是否在 docker 环境中

if [ "$(id -u)" -eq 0 ]; then
    # 如果用户 ID 是 0，则表示在 Docker 环境中运行
    DOCKER_ENV=1
else
    DOCKER_ENV=0
fi

if [ "$DOCKER_ENV" -eq 1 ]; then
    echo -e "\033[33mRunning autotest inside Docker container.\033[0m"
    autotest -koopa -s lv1 /root/compiler
else
    echo -e "\033[33mRunning autotest in the current environment.\033[0m"
    # 获取脚本文件所在目录的绝对路径
    MOUNT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)
    echo "Mounting directory: $MOUNT_DIR"
    docker run -it --rm -v "$MOUNT_DIR":/root/compiler maxxing/compiler-dev \
        autotest -koopa -s lv1 /root/compiler
fi
