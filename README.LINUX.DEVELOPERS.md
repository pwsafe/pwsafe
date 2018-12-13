PasswordSafe is being ported to Linux using the wxWidgets user
interface library. Following are notes for programmers wishing to
build the Linux version. Currently, PasswordSafe is being built on
Debian-based platforms (Debian and Ubuntu), and Fedora, so
requirements are described in terms of .deb and .rpmpackages.

The source code can be downloaded from
https://github.com/pwsafe/pwsafe/archive/master.zip
or cloned via git:
git clone https://github.com/pwsafe/pwsafe.git

For all Linux distros, your version of gcc/g++ needs to be 4.7 or higher
because gcc 4.6 doesn't support the C++11 standard.  It does support the
draft C++0x standard, but our makefiles have -std=c++11, which needs 4.7+

Even 4.7 doesn't seem to support the C++11 standard fully, so if you get
compilation errors, try using a more recent version of gcc, if possible.
It's best to try to build pwsafe on a recent version of Linux than
retrofit a new gcc on an old distro.

Here are the packages/tools required for building the Linux version
under Debian/Ubuntu:
cmake (See note 1 below)
fakeroot
g++
gettext
git
libcurl4-openssl-dev
libqrencode-dev
libssl-devlibgtest-dev
libuuid1
libwxgtk3.0-dbg (See note 2 below)
libwxgtk3.0-dev (See note 2 below)
libxerces-c-dev
libxt-dev
libxtst-dev
libykpers-1-dev (see note 3 below)
libyubikey-dev
make
pkg-config
uuid-dev
zip

The script Misc/setup-deb-dev-env.sh can be run (sudo or as root) to
install these and setup the gtest library.

For Fedora:
cmake (See note 1 below)
gcc-c++
git
gtest-devel
libXt-devel
libXtst-devel
libcurl-devel
libqrencode-devel
libuuid-devel
libyubikey-devel
make
openssl-devel
wxGTK3-devel
xerces-c-devel
ykpers-devel (see note 3 below)
qrencode-devel

To compile without Yubikey support, set the NO_YUBI flag
for make, e.g.,
$ NO_YUBI=1 make
(In this case, you don't need the libyubikey or libykpers-1
development packages)

--------------------
Note #1 - cmake
As of Dec. 2015 PasswordSafe can be built on Linux platforms using
cmake, e.g., mkdir build; cd build; cmake ..; make
Currently, this is in addition to the 'standard' toplevel make. In the
long term, it will probably replace it.
The cmake build can be configured in the standard manner, e.g, via
cmake-gui.

--------------------
Note #2 - wxWidgets

Some distributions lag behind the required version of wxWidgets,
providing a version older than that required by PasswordSafe. If this
is the case, you can either:
(a) Get the wxWidgets package from the relevant link in
http://wxwidgets.org/downloads/ under "Binaries", or
(b) Download the sources from here
http://www.wxwidgets.org/downloads/
and build the libraries yourself. If you do so:
1. Configure the build using the following:
$ ./configure --disable-shared --enable-stl --enable-utf8only \
  --enable-intl --enable-xlocale --enable-debug_gdb 
2. Set the WX_CONFIG environment variable to point to the correct
location, e.g. add the following to you .bashrc file:
export WX_CONFIG=$HOME/src/wxWidgets-3.0.2/wx-config

Note that we use a static build of wxWidgets in order to simplify the
distribution, not requiring users to get the wx3 package, and avoiding
potential conflicts with 2.8.

--------------------
Note #3 - libykpers-1/ykpers-devel:
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

--------------------

Running 'make' at the top of the source tree will result in the debug
version of pwsafe being built under src/ui/wxWidgets/GCCUnicodeDebug,
and the release version under src/ui/wxWidgets/GCCUnicodeRelease.

Note that under Fedora and RHEL5, wxGTK-devel doesn't support
"wx-config --debug=yes --unicode=yes" so just "make" fails. The
workaround is to use "make release", which will only build the release
version.

Fedora 22 didn't rebuild wxGTK with gcc 5 yet it seems, to the
ABI-version was at version 2, which was the default up until gcc
4.9. A way to solve this would be adding '-fabi-version=2' to the
CXXFLAGS.

Create a Debian Package
=======================
Extract the source tree into some directory and change into this
directory, i.e. where CMakeLists.txt is present.

* mkdir build
* cd build
* cmake ..
* cpack -G DEB ..


Install the Debian Package
==========================
* sudo dpkg -i passwordsafe-\<debian|ubuntu\>-\<version\>.\<arch\>.deb

Create a RPM Package
====================
Based on the procedure for building a Debian package a RPM based package can be created as well.

* mkdir build
* cd build
* cmake ..
* cpack -G RPM ..

Install the RPM Package
=======================
To install the RPM package use the following command as root.
* rpm -ivh passwordsafe-fedora-\<version\>.\<arch\>.rpm

Upgrading an already installed RPM package can be done with the following command as root. This is the same as install, except all other version(s) of the package are removed after the new package is installed.
* rpm -Uvh passwordsafe-fedora-\<version\>.\<arch\>.rpm
