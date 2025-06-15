#!/bin/sh

cd kernel/
make clean
make
cd ..
qemu-system-i386   -kernel kernel/kernel.elf   -m 512M   -chardev socket,path=./socket.sock,server=on,wait=on,id=aiport   -device virtio-serial-pci   -device virtserialport,chardev=aiport,name=org.qemu.console.OS   -machine pc,smm=off -no-reboot -serial mon:stdio   -d int

