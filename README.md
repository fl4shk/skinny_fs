# skinny_fs
Tiny Entirely-In-RAM Filesystem (i.e. ramdisk) Implementation in C.

This has no dependencies besides your choice of libc, though `skinny_fs`
*does not* rely on `stdio.h`,
besides for use in `main.c` (which is just a test program making use of
`skinny_fs`).

You can probably ignore `build.sh` and `main.c` unless you want to try out
the test program

There is also support for situations like the following:
* For example, a microcontroller where you have existing files stored in
the flash memory, subject to the following limitations:
    * Said file must be possible to at least *read from* with a
    dereferenced C pointer.
    * You must know the size of the file ahead of time.
    * A write into the file can end up copying it to a new `malloc()`ed
    file *in RAM* if you end up writing *past* the initial file size.

Other overall limitations include the following:
* There is no relative paths support. Every path is absolute.
    * Directories are effectively non-existent,
    but `"/"` or `"\"` in a path can be used to fake support of
    directories.
* There is no built-in support for writing files back to an actually
non-volatile disk.
