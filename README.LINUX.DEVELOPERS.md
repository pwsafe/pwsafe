## Introduction
Password Safe has being ported to Linux using the wxWidgets user
interface library. Following are notes for programmers wishing to
build the Linux version. Currently, Password Safe is being built on
Debian-based platforms (Debian and Ubuntu), and Fedora, so
requirements are described in terms of .deb and .rpm packages.

## Short Guide
The following should work for Debian and Fedora-based distros:
1. Download the source code from https://github.com/pwsafe/pwsafe/archive/master.zip or clone the repository via git: 

    ` $ git clone https://github.com/pwsafe/pwsafe.git `

2. Setup dependencies

    `$ cd pwsafe`    
    `$ sudo sh Misc/setup-linux-dev-env.sh `

3. Setup the build environment:

     `$ mkdir build`

     `$ cd build`

     `$ cmake -G Ninja ..` (See below on how to customize your build)

4. Make the executables:

    `$ ninja`
This will create a 'pwsafe' executable in your build directory.
5. Run the core test suite:
  
    `$ ninja test`
This will run coretest in the src/test sub-directory.  
If it encounters errors, check your locale settings. It is known to work with:
    `LC_ALL=en_US.UTF-8`
6. Make the package:

- To build a deb package:
```
    $ cpack -G DEB ..
```
- To build an RPM package:
```
    $ cpack -G RPM ..
```

7. Install

- To install the deb package you've just built:

    `$ sudo dpkg -i passwordsafe-\<debian|ubuntu\>-\<version\>.\<arch\>.deb`

- To install the rpm package you've just built:

    `$ dnf install passwordsafe-fedora-\<version\>.\<arch\>.rpm`

8. Run a program

    `$ pwsafe`

## Requirements
The packages that Password Safe depends upon are listed in Misc/setup-linux-dev-env.sh. Note that some of the package names differ between DEB and RPM distros, and some, unfortunately, between versions within a distro. The script attemps to take all this into account. Running it as root or via sudo should install all the packages needed to build Password Safe.

## Customization
The easiest way to customize the build is by running cmake-gui instead of cmake from the build directory, e.g.,

    $ cmake-gui ..

Alternately, you can pass cmake flags, such as:

    `$ cmake -D CMAKE_BUILD_TYPE=Debug -D NO_QR=ON -D NO_GTEST=ON -D wxWidgets_CONFIG_EXECUTABLE=~/wxWidgets-3.1.3/wxbuild/wx-config ..`


If you have a custom build of the Wx library you would like to use 
you can point to its 'wx-config' command by use of cmake's command 
line option 'wxWidgets_CONFIG_EXECUTABLE'. You can also disable the 
gtest unit testing (option NO_GTEST), YubiKey support (option NO_YUBI) 
and QR support (option NO_QR), if they are not required.

## wxWidgets

Some distributions lag behind the required version of wxWidgets,
providing a version older than that required by Password Safe. If this
is the case, you can either:

(a) Get the wxWidgets package from the relevant link in
    http://wxwidgets.org/downloads/ under "Binaries", or
    
(b) Download the sources from here
    http://www.wxwidgets.org/downloads/
    
and build the libraries yourself. If you do so:
1. Configure the build using the following:

    ```
    $ ./configure --disable-shared --enable-stl --enable-utf8only \
    --enable-intl --enable-xlocale --enable-debug_gdb
    ```

2. Set the WX_CONFIG environment variable to point to the correct
   location, e.g. add the following to you .bashrc file:

    ```
    export WX_CONFIG=$HOME/src/wxWidgets-3.0.2/wx-config
    ```

Note that we use a static build of wxWidgets in order to simplify the
distribution, not requiring users to get the wx3 package, and avoiding
potential conflicts with 2.8.

Alternatively, the Wx library can be build using cmake.
By default shared libraries with unicode support are build. For a static 
build set option 'wxBUILD_SHARED' to 'OFF'.

Unpack the downloaded wxWidgets package and change into its directory. 
Create a new directory for the build artefacts and change to that one. 
Execute cmake with appropriate preprocessor symbols and run make (consider
using option '-j').

    cd <wxWidgets directory, e.g. wxWidgets-3.1.3>
    mkdir <build directory, e.g. wxbuild>
    cd <build directory, e.g. wxbuild>
    cmake -D wxUSE_STL=ON -D wxBUILD_SHARED=OFF ..

The internationalization system (option wxUSE_INTL) and x-locale support 
(option wxUSE_XLOCALE) is also enabled by default. For a list of options 
to tune the build see also the documentation (
[3.0](https://docs.wxwidgets.org/3.0/page_wxusedef.html), 
[3.1](https://docs.wxwidgets.org/3.1/page_wxusedef.html)
) about wxUSE preprocessor symbols.


##  libykpers-1 and ykpers-devel
If your distro doesn't have the development version of this you will
need to build and install it from the source:
https://github.com/Yubico/yubikey-personalization.git

In case you want to specify a non-standard location from which
yubikey-personalization headers/libs are to be used, invoke "make"
like this:

    $ YBPERS_LIBPATH=<dir with libykpers-1.a or .so> YBPERS_INC=<yubikey-pers dir/ykcore/> make unicode{release,debug}

If your build linked with libykpers-1.so in a non-standard location,
you might need to invoke pwsafe as

    $ LD_LIBRARY_PATH=<libykpers-1.a or libykpers-1.so dir> pwsafe 
