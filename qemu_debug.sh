#!/bin/sh

cd kernel/
make clean
make
cd ..
qemu-system-i386 -kernel kernel/kernel.elf -s -S -no-reboot -machine smm=off -d int
