# ZeOS 💾

Welcome to my own take on the ZeOS. This OS was developed while I was studying the Operative Systems 2 subject in FIB.

It has multitasking support, a basic libc and basic IO. On the inside it has two different execution modes, user mode and system (aka kernel) mode.

It is build using a structure similar to the **Linux Kernel 2.x**.

## How can I use it? ⌨

First of all you need to have installed **gcc**, **make** and the **Bochs emulator**.

Compile it using `make`, and run it using `make emul` (starts the Bochs emulator).

You can debug its code with the Bochs debugger using `make emuldbg` or with the GDB debugger using `make gdb` instead of running `make emul`.

## Adding your own user code

To add your own user code you just need to modify the file *user.c*. There starts the code runned in user mode, so modify this file and add your own code there, then you can add more files which are called from this one.
