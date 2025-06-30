#!/bin/sh
qemu-system-x86_64   -kernel output/images/bzImage --enable-kvm   -cpu host   -hda output/images/rootfs.ext2   -append "root=/dev/sda console=ttyS0"   -serial mon:stdio -m 6144