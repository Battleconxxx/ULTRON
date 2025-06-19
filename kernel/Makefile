DEFAULT_HOST!=../default-host.sh
HOST?=$(DEFAULT_HOST)
HOSTARCH!=../target-triplet-to-arch.sh $(HOST)

CFLAGS?=-O0 -g -m32
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -fno-pic -fno-pie
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -Iinclude -I../libc
LDFLAGS:=$(LDFLAGS) -L../libc -Wl,-no-pie -Wl,--build-id=none
LIBS:=$(LIBS) -nostdlib -lk -lgcc
CXX = i686-elf-g++


ARCHDIR=arch/$(HOSTARCH)

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
kernel/kernel.o \
$(ARCHDIR)/gdt_flush.o \
$(ARCHDIR)/tss_flush.o \
$(ARCHDIR)/gdt.o \
interrupts/interrupts.o \
interrupts/idt.o \
interrupts/isr.o \
user/user.o \
memory/memory.o \
FS/fs.o \
$(ARCHDIR)/keyboard.o \
initrd/initrd.o \


OBJS=\
$(ARCHDIR)/crti.o \
$(KERNEL_OBJS) \
$(ARCHDIR)/crtn.o \

LINK_LIST=\
$(LDFLAGS) \
$(ARCHDIR)/crti.o \
$(KERNEL_OBJS) \
$(LIBS) \
$(ARCHDIR)/crtn.o \

.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .S

all: myos.kernel

myos.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
	grub-file --is-x86-multiboot myos.kernel

# all: kernel.elf

# kernel.elf: $(OBJS) $(ARCHDIR)/linker.ld
# 	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
# 	grub-file --is-x86-multiboot kernel.elf

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

%.o: %.asm
	nasm -f elf32 $< -o $@


.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.cpp.o:
	$(CXX) -MD -c $< -o $A -std=c++17 $(CFLAGS) $(CPPFLAGS) -fno-exceptions -fno-rtti

clean:
	rm -f myos.kernel
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-kernel: myos.kernel
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp myos.kernel $(DESTDIR)$(BOOTDIR)

# install-kernel: kernel.elf
# 	mkdir -p $(DESTDIR)$(BOOTDIR)
# 	cp kernel.elf $(DESTDIR)$(BOOTDIR)


-include $(OBJS:.o=.d)
