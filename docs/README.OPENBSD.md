## Introduction
The OpenBSD port of Password Safe is currently in BETA.
This means that (1) you should take care to keep copies of the
database to protect against possible loss of data due to bugs, and (2)
there are several unimplemented features. Nonetheless, we feel that
this is good enough to release as an early beta to gather feedback
from a wider audience.


### Supported
This has only been tested:
* OpenBSD 7.2 amd64 with wx 3.0

If you build it on a different version/arch (like arm64), please send patches!

### Known not working
* QR codes - OpenBSD doesn't seem to have libqrencode. But has
 [qr-code-generator](https://www.nayuki.io/page/qr-code-generator-library)



## Requirements
Here are the packages/tools required for building "pwsafe".
- archivers/zip
- devel/gmake
- devel/cmake
- devel/googletest
- misc/e2fsprogs-libuuid
- lang/clang38
- textproc/xerces-c3
- x11-toolkits/wxgtk30
- wxWidgets-gtk3
- libmagic
- xerces-c
- gettext-tools (for xgettext, to build helpfiles).


On OpenBSD 7.2, I could not find the 'zip' utility. "pkg_info -Q zip" and all variants of it failed to get anything. Curiously, the 'unzip' binary comes with the os. But 'zip' is somehow missing.

Before you do what's below, check if you already have the "zip" command from somewhere. "which zip" will tell you. If you have "zip" already, skip this step.

There are two ways of dealing with this:

a. Comment out the line that builds helpfiles. This is the least painful way if you don't need PasswworSafe's bundled help content:

	add_subdirectory (help) # online help

			OR

b. Download the binary zip package "zip-3.0p1.tgz" from one of OpenBSD's mirrors, e.g.
	https://cdn.openbsd.org/pub/OpenBSD/7.2/packages/amd64/

   Install it like this

	 pkg_add -n zip-3.0p1.tgz        # check if it'd install cleanly
         pkg_add zip-3.0p1.tgz           # actually install it

## Build
1. Create the build directory
    ```
    mkdir build; cd build;
    ```
 
2. Create the makefiles
    ```
    cmake -D wxWidgets_CONFIG_EXECUTABLE=/usr/local/bin/wxgtk2u-3.0-config -D CMAKE_C_COMPILER=clang38 -DCMAKE_CXX_COMPILER=clang++38 ..
    ```
    
3. Start the build process
    ```
    gmake
    ```

4. Your `pwsafe` binary is in `build` (your current directory)

5. At start you get a warning about the help system. Copy the generated help file of your language (e.g. helpEN.zip) to /usr/share/passwordsafe/help/helpEN.zip.

6. If for some reason you cannot use CMake, try the classic Makefile's. Try "gmake unicoderelease" or "gmake unicodedebug" in the pwsafe root dir. Note that you must use gmake: these Makefiles don't work with BSD make. The generated binary will be in src/ui/wxWidgets/GCCUnicode{Release,Debug}/pwsafe


## Reporting Bugs
Please submit bugs via https://github.com/pwsafe/pwsafe. 
Make sure you include output of `uname -a`.
