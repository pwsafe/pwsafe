PasswordSafe is being ported to Linux using the wxWidgets user
interface library. Following are notes for programmers wishing to
build the Linux version. Currently, PasswordSafe is being built on
Debian-based platforms (Debian and Ubuntu), so requirements are
described in terms of .deb packages. If someone builds pwsafe on
another distro, please update this document with the corresponding
package names (e.g., rpm).

For all Linux distros, your version of gcc/g++ needs to be 4.7 or higher
because gcc 4.6 doesn't support the C++11 standard.  It does support the
draft C++0x standard, but our makefiles have -std=c++11, which needs 4.7+

Even 4.7 doesn't seem to support the C++11 standard fully, so if you get
compilation errors, try using a more recent version of gcc, if possible.
Its best to try to build pwsafe on a recent version of Linux than retrofit
a new gcc on an old distro.

Here are the packages/tools required for building the Linux version
under Debian/Ubuntu:
fakeroot
g++
gettext
git
libuuid1
libwxgtk3.0-dbg (See note 1 below)
libwxgtk3.0-dev (See note 1 below)
libxerces-c-dev
libxt-dev
libxtst-dev
libykpers-1-dev (see note 2 below)
libyubikey-dev
make
pkg-config
uuid-dev
zip

For Fedora:
gcc-c++
git
libXt-devel
libXtst-devel
libuuid-devel
libyubikey-devel
make
wxGTK3-devel
xerces-c-devel
ykpers-devel (see note 2 below)

To compile without Yubikey support, set the NO_YUBI flag
for make, e.g.,
$ NO_YUBI=1 make
(In this case, you don't need the libyubikey or libykpers-1
development packages)

--------------------
Note #1 - wxWidgets 3.0

Some distributions still don't provide wxWidgets 3.0. In this case,
you can either:
(a) Get the packages from another repository, as described here:
http://codelite.org/LiteEditor/WxWidgets30Binaries
or
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

Note #2 - libykpers-1/ykpers-devel:
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
directory, i.e. where Makefile.linux is present.

* make -f Makefile.linux
* make -f Makefile.linux deb


Install the Debian package
==========================
* sudo dpkg -i Releases/passwordsafe-debian-<version>.<arch>.deb

64 bit status:
64 bit build appear functional and stable, but have not bee extensibly tested.
