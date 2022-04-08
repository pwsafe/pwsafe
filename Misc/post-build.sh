#!/bin/bash
#
# post-build script
# Meant to be called by cpack after package creation to finish up the build process, but can also work standalone.
#
# Currently just signs the created package (deb or rpm, type and name passed as argument) using gpg key.
# Signing key defaults to current project admin, but this can be overrode via PWS_SIGNER and PWS_KEYID
# environment variables.
#

SIGNER=${PWS_SIGNER:="ronys@pwsafe.org"}
KEYID=${PWS_KEYID:="7F2F1BB9"}

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
DEBSIGN=/usr/bin/dpkg-sig

case "$TYPE" in
RPM)
    if [ ! -x $RPMSIGN ]; then
        echo "$RPMSIGN not found"
        exit 2
    fi
    $RPMSIGN --addsign --define "%_gpg_name $SIGNER" $PACKAGE
    ;;
DEB)
     if [ ! -x $DEBSIGN ]; then
        echo "$DEBSIGN not found"
        exit 2
    fi
    $DEBSIGN --sign builder -k $KEYID $PACKAGE
    ;;
*)
    Usage #Should have been caught earlier...
esac

exit 0
