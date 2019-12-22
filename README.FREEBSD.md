= Introduction =
The FreeBSD port of PasswordSafe is currently in BETA.
This means that (1) you should take care to keep copies of the
database to protect against possible loss of data due to bugs, and (2)
there are several unimplemented features. Nonetheless, we feel that
this is good enough to release as an early beta to gather feedback
from a wider audience.

= Supported =
This has only been tested:
 * FreeBSD 10.2 amd64 with wx 3.0
 * FreeBSD 10.3 amd64 with wx 3.0
 * FreeBSD 11.0 i386 with wx 3.0
 * FreeBSD 11.0 amd64 with wx 3.0
== Known not working ==
 * The help system
 * Debug builds
 * -d / -e command line switches

= Installation =
 1. Install:
    - archivers/zip
    - devel/gmake
    - devel/cmake
    - devel/googletest
    - misc/e2fsprogs-libuuid
    - lang/clang38
    - textproc/xerces-c3
    - x11-toolkits/wxgtk30
 2. mkdir build; cd build;
 3. cmake -D wxWidgets_CONFIG_EXECUTABLE=/usr/local/bin/wxgtk2u-3.0-config -D CMAKE_C_COMPILER=clang38 -DCMAKE_CXX_COMPILER=clang++38 ..
 4. gmake
 5. Your `pwsafe` binary is in `build` (your current directory)
 6. At start you get a warning about the help system

= Reporting bugs =
Please submit bugs via https://github.com/pwsafe/pwsafe
Make sure you include output of `uname -a`
