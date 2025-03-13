# llvm-pass-skeleton

A completely useless LLVM pass.
It's for LLVM 17.

Build:

    $ cd llvm-pass-skeleton
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

(the particle-sim/ directory is not included in this repo since it's from a CS 5220 assignment)

Run:

    $ make -C build && `brew --prefix llvm`/bin/clang++ -O3 -fpass-plugin=build/skeleton/SkeletonPass.dylib particle-sim/main.cpp particle-sim/serial.cpp

You can add the -S and -emit-llvm flags as necessary to generate asm and LLVM IR.
