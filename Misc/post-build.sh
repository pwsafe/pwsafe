#!/bin/bash
#
# post-build script
# Meant to be called by cpack after package creation to finish up the build process
# Currently signs the created package (deb or rpm, name passed as argument) using gpg key.
#
# Detecting runtime distro type (Debian/RedHat) here rather than having 2 scripts selected by
# cpack, as CMakeLists.txt is convoluted enough as-is.

SIGNER=${PWS_SIGNER:="ronys@pwsafe.org"}

Usage () {
    echo "Usage: $0 DEB|RPM package-to-sign"
    exit 1
}


[ $# -eq 2 ] || Usage
[ "$1" = "DEB" ] || [ "$1" = "RPM" ] || Usage
[ -e $2 ] || Usage

TYPE=$1
PACKAGE=$2
RPMSIGN=/usr/bin/rpmsign

case "$TYPE" in
RPM)
    if [ ! -x $RPMSIGN ]; then
        echo "rpmsign not found"
        exit 2
    fi
    $RPMSIGN --addsign --define "%_gpg_name $SIGNER" $PACKAGE
    ;;
DEB)
    echo "TBD: debsign $PACKAGE"
    ;;
*)
    Usage #Should have been caught earlier...
esac

exit 0
