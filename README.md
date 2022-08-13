# AIX/ia64 EFI shenanigans

This repository is dedicated to Itanium EFI experiments related to an ongoing
effort to run AIX/ia64 (also known as Project Monterey) on modern Itanium 2 and
Itanium 9xxx hardware. However, it might be also useful as a base for other
cases where an IA-64 EFI toolchain based on modern GCC and binutils is needed.

Since currently the boot is stuck at EFI bootloader level, a toolchain capable
of analyzing and building IA-64 EFI binaries is handy.

The choice explored here consists of an up-to-date IA-64 GCC/Binutils ELF
toolchain, with EFI PE binary support via the `pei-ia64` BFD target, together
with the GNU-EFI library, which forms the base of this repository.

## Building GNU-EFI

Assuming `PATH` is set to contain an GCC/Binutils ia64 toolchain and
`TOOL_PREFIX` contains its prefix (e.g. `ia64-unknown-linux-gnu`), GNU-LIB
in this repository including example apps can be build like this:

```
make ARCH=ia64 CC=$TOOL_PREFIX-gcc AS=$TOOL_PREFIX-as LD=$TOOL_PREFIX-ld AR=$TOOL_PREFIX-ar RANLIB=$TOOL_PREFIX-ranlib OBJCOPY=$TOOL_PREFIX-objcopy all
```

Note: The `all` target is required, at least on my machine, to be specified
manually in order for make to build everything including subdirs correctly.
