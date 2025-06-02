# Melon OS

Unix like operating system on i386.


## How to build

Simply run:
```
make
```
You will then see `melonos.qcow2`, `melonfs.qcow2` being built.


##### To start in qemu

```
make qemu
```

##### To enable gdb debug mode

```
GDB=1 make qemu
```


##### To interact with the OS through serial IO

Run the following on one terminal
```
make serial
```
This will wait for qemu to start.

On another one run qemu.

```
# you can turn off qemu graphics like this
GRAPHICS=0 GDB=1 make qemu
```

Now you can handle IO through the serial.


##### To print realtime debug log

Make sure build with debug flag on
```
DEBUG=1 make
```

Then, on one terminal run
```
make debug
```
It will listen to qemu's debuggin output through COM2.

Then run
```
make qemu
```
as usual.

Now all the real time debugging info will be printed on the debug terminal.


##### How to debug

You can debug both the kernel and userpace program with gdb. The debugging output
can be accessed via `make debug` as mentioned above.

To debug ther kernel, simply run `DEBUG=1 make qemu` and run gdb on another session.
Once the gdb starts, run `target remote localhost:1234`. This connects the gdb with
qemu. Then you can set a breakpoint like `(gdb) b scheduler`.

To debug a user space program, you need to load user program symbols to gdb as well.
E.g, to debug the shell, once the gdb connects to the kernel, run `(gdb) add-symbol-file melon/sh_`.
Then `(gdb) b main`. Note symbols in kernel and user program will be at the same namespace, if
there is a conflict you need to delete the one you don't need.

You can use .gdbinit to automate the debugging process. E.g My .gdbinit always have the following
line:
```
target remote localhost:1234
```
This auto-connects the gdb with qemu.


## Design

This is a rough diagram for what's going on here.

![melonos](doc/melonos.png)
