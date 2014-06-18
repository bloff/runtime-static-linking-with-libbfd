runtime-static-linking-with-libbfd
==================================

An example of runtime static linking with libbfd.

The program in main.c loads the object file test_unit.o into memory, and runs a function defined in test_unit.c (currently "my_test_function_01");
that function, in turn, calls a function named "callback",
which is defined as "extern" in test_unit.c; but main.c relocates the call ("patches the assembly code" aka statically links) to the address of a function defined in main.c 
(currently "my_callback_01").

So we have runtime static linking.

The code should be portable to any platform where the following conditions are met:
* The platform is posix compatible (we use posix to allocate memory with read, write AND exec permissions; this can be done on other platforms also, but the code will need changing)
* GCC must produce a binary format with a section called ".text" containing the binary code for the functions in test_unit.c and another called ".data" containing the initialization of variables in test_unit.c (though currently .data is unused and WON'T WORK -- lazy me -- and of course this can be easily changed by changing the source code); in principle, any .o file gcc compiles into will do.
* libbfd must know how to relocate the calls in .text; in principle, anything gcc compiles into will do.

TODO:
* Patch all references to variables in .data.
* Allow for debugging with gdb. This might possibly be done with gdb proper, but it would need a couple of more days research, assuming it can be done at all with existing tools; but would be awesome.
