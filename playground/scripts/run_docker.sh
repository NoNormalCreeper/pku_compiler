#!/bin/bash

# 获取脚本文件所在目录的绝对路径
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

# 计算出 ../.. 对应的绝对路径
MOUNT_DIR=$(realpath "$SCRIPT_DIR/../..")

echo "Mounting directory: $MOUNT_DIR"

# 使用绝对路径作为卷挂载来运行 Docker 容器, 并在容器内执行 chmod 命令
docker run -it --rm -v "$MOUNT_DIR":/root/compiler maxxing/compiler-dev bash -c "chmod +x -R /root/compiler; exec bash"