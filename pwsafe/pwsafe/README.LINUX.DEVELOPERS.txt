PasswordSafe is being ported to Linux using the wxWidgets user
interface library. Following are notes for programmers wishing to
build the Linux version. Currently, PasswordSafe is being built on
Debian-based platforms (Debian and Ubuntu), so requirements are
described in terms of .deb packages. If someone builds pwsafe on
another distro, please update this document with the corresponding
package names (e.g., rpm).

Here are the packages/tools required for building the Linux version
under Debian:
g++
gmake (version 3.81 or newer.  Makefiles are not compatible with lower versions)
libuuid1
libwxgtk2.8-dev
libwxgtk2.8-dbg
libxt-dev
libxtst-dev
subversion
fakeroot

For Ubuntu:
- 'make' replaces 'gmake'
- uuid-dev should be added

For Fedora:
gcc-c++
subversion
libXt-devel
libXtst-devel
libuuid-devel
xerces-c-devel
wxGTK0devel
make

With these installed, running 'make' at the top of the source tree
will result in the debug version of pwsafe being built under
src/ui/wxWidgets/GCCUnicodeDebug

TBD:
1. Improve .deb building script
2. Add support for .rpm building
