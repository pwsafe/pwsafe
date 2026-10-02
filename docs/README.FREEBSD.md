## Introduction
The FreeBSD port of Password Safe is currently in BETA.
This means that you should take care to keep copies of the
database to protect against possible loss of data due to bugs. 
Nonetheless, we feel that this is good enough to release 
as an early beta to gather feedback from a wider audience.


### Supported
This has only been tested:
* FreeBSD 14.x amd64 with wx 3.2
* FreeBSD 15.x amd64 with wx 3.2


## Requirements
Here are the packages/tools required for building "pwsafe".
- archivers/zip
- devel/git
- devel/gmake
- devel/cmake
- devel/googletest
- devel/gettext-tools
- misc/libuuid
- lang/llvm19
- textproc/xerces-c3
- x11-toolkits/wx32-gtk3
- graphics/libqrencode
- security/ykpers


## Build
1. Create the build directory
    ```
    mkdir build; cd build
    ```
 
2. Create the makefiles
    ```
    cmake -D wxWidgets_CONFIG_EXECUTABLE=/usr/local/bin/wxgtk3u-3.2-config -D CMAKE_C_COMPILER=clang19 -DCMAKE_CXX_COMPILER=clang++19 ..
    ```
    
3. Start the build process
    ```
    gmake
    ```

4. Your `pwsafe` binary is in `build` (your current directory)

5. Create an installation package that includes the Help and translation files
    ```
    cpack -G FREEBSD ..
    ```

6. As root, install the `passwordsafe` package on your system
    ```
    pkg add passwordsafe-freebsd-<VERSION>-<ARCH>.pkg
    ```


## Reporting Bugs
Please submit bugs via https://github.com/pwsafe/pwsafe. 
Make sure you include output of `uname -a`.
