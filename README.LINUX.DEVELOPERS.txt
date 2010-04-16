PasswordSafe is being ported to Linux using the wxWidgets user
interface library. Following are notes for programmers wishing to
build the Linux version. Currently, PasswordSafe is being built on
Debian-based platforms (Debian and Ubuntu), so requirements are
described in terms of .deb packages. If someone builds pwsafe on
another distro, please update this document with the corresponding
package names (e.g., rpm).

Here are the packages/tools required for building the Linux version:
g++
make
libuuid1
libwxgtk2.8-dev
libxt-dev
libxtst-dev
subversion

With these installed, running 'make' at the top of the source tree
will result in the debug version of pwsafe being built under
src/ui/wxWidgets/GCCUnicodeDebug

TBD (development process, not functionality):
1. Support "make release" to create non-Debug build
2. Add packaging support (e.g., "make deb", "make rpm", etc.)
