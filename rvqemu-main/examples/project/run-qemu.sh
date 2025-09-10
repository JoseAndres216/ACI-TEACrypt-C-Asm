#!/bin/bash

# Run QEMU with GDB server for C+assembly project
echo "Starting QEMU with GDB server on port 1234..."
echo "In another terminal, run: gdb-multiarch project.elf"
echo "Then in GDB: target remote :1234"
echo ""
echo "Useful GDB commands for this project:"
echo "  break _start          - break at C program start"
echo "  break sum_to_n        - break at assembly function"
echo "  info registers        - show register values"
echo "  step / next           - step through code"

qemu-system-riscv32 \
    -machine virt \
    -nographic \
    -bios none \
    -kernel project.elf \
    -S \
    -gdb tcp::1234