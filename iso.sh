#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/myos.kernel isodir/boot/myos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "myos" {
	multiboot /boot/myos.kernel
	module /boot/stories15M.bin stories15M.bin
	module /boot/tokenizer.bin tokenizer.bin
}
EOF
grub-mkrescue -o myos.iso isodir
