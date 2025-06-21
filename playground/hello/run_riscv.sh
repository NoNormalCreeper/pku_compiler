clang hello_riscv.S -c -o hello_riscv.o -target riscv32-unknown-linux-elf -march=rv32im -mabi=ilp32
ld.lld hello_riscv.o -L$CDE_LIBRARY_PATH/riscv32 -lsysy -o hello_riscv
qemu-riscv32-static hello_riscv