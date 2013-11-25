PasswordSafe is being ported to Linux using the wxWidgets user
interface library. Following are notes for programmers wishing to
build the Linux version. Currently, PasswordSafe is being built on
Debian-based platforms (Debian and Ubuntu), so requirements are
described in terms of .deb packages. If someone builds pwsafe on
another distro, please update this document with the corresponding
package names (e.g., rpm).

Here are the packages/tools required for building the Linux version
under Debian/Ubuntu:
fakeroot
g++
gettext
make (version 3.81 or newer.  Makefiles are not compatible with lower versions)
libuuid1
libwxgtk2.8-dev
libwxgtk2.8-dbg
libxerces-c-dev
libxt-dev
libxtst-dev
libykpers-1-dev (see note below)
libyubikey-dev
git
uuid-dev
zip

For Fedora:
gcc-c++
git
libXt-devel
libXtst-devel
libuuid-devel
libykpers-1-devel (see note below)
libyubikey-devel
xerces-c-devel
wxGTK-devel
make

To compile without Yubikey support, set the NO_YUBI flag
for make, e.g.,
$ NO_YUBI=1 make
(In this case, you don't need the libyubikey or libykpers-1
development packages)

--------------------

Notes re libykpers-1:
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
version of pwsafe being built under src/ui/wxWidgets/GCCUnicodeDebug

(Note that under Fedora and RHEL5, wxGTK-devel doesn't support
"wx-config --debug=yes --unicode=yes" so just "make" fails. The
workaround is to use "make release". The release binary will be found
under src/ui/wxWidgets/GCCUnicodeRelease.)

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
