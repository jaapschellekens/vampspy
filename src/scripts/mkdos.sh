#!/bin/csh
setenv CFLAGS "-O3 -Wall -m486 -funroll-loops -fomit-frame-pointer"
setenv CC "gcc-go32  -I/home/schj/src/slang.099/src"
setenv AR i386-go32-ar
setenv RANLIB i386-go32-ranlib
setenv LDFLAGS -L/home/schj/lib/x86
./configure
make lib |& tee mk_lib.log
make all |& tee mk_all.log
