#!/bin/bash
set -e # 如果任何命令失败，则立即退出

# 在脚本所在目录执行
cd "$(dirname "$0")"

rm -r ./build || true

# 配置并构建项目
cmake -DCMAKE_BUILD_TYPE=Debug -B build && cmake --build build -j24

echo -e "\033[33mBuild complete. Executable is at: build/compiler\033[0m"