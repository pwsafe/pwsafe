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
libxerces-c-dev
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
wxGTK-devel
make

With these installed, running 'make' at the top of the source tree
will result in the debug version of pwsafe being built under
src/ui/wxWidgets/GCCUnicodeDebug (*)

(*) Note that under Fedora and RHEL5, wxGTK-devel doesn't support
"wx-config --debug=yes --unicode=yes" so just "make" fails. The
workaround is to use "make CONFIG=unicoderelease"

TBD:
1. Improve .deb building script
2. Add support for .rpm building

64 bit status:
Work has started on supporting 64-bit builds under Linux (thanks to
pm_kan). Currently (11/2010) the program compiles cleanly, but crashes
when trying to add an entry or otherwise access the cryptographic
functionality. Here's a partial list:
- check usage of constants instead sizeof
- check %s formats (wchar_t args in (v)(s)wprintf require %ls instead
of %s in Linux) 
- use stdint.h types
- check long type sizes (in Lin x86_64 sizeof(long)==8 in
Lin32/Win32/Win64 sizeof(long)==4) 
- check size_t conversions (in Lin x86_64 sizeof(size_t)!=sizeof(unsigned int))
- cppCheck warnings
- valgrind warnings

