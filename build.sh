#!/bin/sh
set -e
. ./headers.sh

export PREFIX="$HOME/cibiz/CrossCompiler/"          
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"


for PROJECT in $PROJECTS; do
  (cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)
done
