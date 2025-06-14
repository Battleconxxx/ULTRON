#!/bin/sh

cd kernel/
make clean
make
cd ..
qemu-system-i386   -kernel kernel/kernel.elf   -m 512M   -chardev socket,path=/tmp/ai.sock,server=on,wait=off,id=aiport   -device virtio-serial   -device virtserialport,chardev=aiport,name=ai   -machine smm=off -s -S -no-reboot   -serial mon:stdio   -d int

