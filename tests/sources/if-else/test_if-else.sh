# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 遍历脚本所在目录下每一个文件
for file in "$SCRIPT_DIR"/*; do
    if [ -f "$file" ]; then
        # 获取文件名（不包含路径）
        filename="$(basename "$file")"
        
        # 检查文件是否以.c结尾
        if [[ "$filename" != *.c ]]; then
            continue
        fi
        
        # 输出文件名
        echo -e "\033[1;32m正在处理测试点: $filename\033[0m"
        cat "$file"
        echo
        
        # 运行编译命令
        echo -e "\033[1;34m运行编译命令...\033[0m"
        build/compiler -riscv "$file" -o $SCRIPT_DIR/../../build/$filename.S
        
        echo -e "\033[1;33m$filename 测试完成\033[0m"
        echo "----------------------------------------"
    fi
done