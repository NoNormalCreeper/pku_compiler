#!/bin/bash
set -e # 如果任何命令失败，则立即退出

# 在脚本所在目录执行
cd "$(dirname "$0")"

# 配置并构建项目
cmake -B build && cmake --build build

echo "Build complete. Executable is at: build/compiler"