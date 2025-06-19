# toolchain host triple
export HOST=${HOST:-$(./default-host.sh)}
export MAKE=${MAKE:-make}

# compiler tools
export AR=${HOST}-ar
export AS=${HOST}-as
export CC=${HOST}-gcc

# clean guest sysroot
export SYSROOT="$(pwd)/sysroot"
export INCLUDEDIR="$SYSROOT/usr/include"
export LIBDIR="$SYSROOT/usr/lib"

# clean flags
export CFLAGS="-O2 -g -ffreestanding -nostdlib -I$INCLUDEDIR"
export LDFLAGS="-L$LIBDIR"

# sysroot-aware compiler
export CC="$CC --sysroot=$SYSROOT"

# make -isystem use sysroot, not host junk
if echo "$HOST" | grep -Eq -- '-elf($|-)'; then
  export CC="$CC -isystem=$INCLUDEDIR"
fi
