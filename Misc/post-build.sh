#!/bin/bash
#
# post-build script
# Meant to be called by cpack after package creation to finish up the build process, but can also work standalone.
#
# Currently just signs the created Linux package (deb or rpm, type and name passed as argument) using gpg key,
# or the created Mac OS X package using the Apple Developer ID Installer certificate.
# Signing key defaults to current project admin, but this can be overrode via PWS_SIGNER and PWS_KEYID
# environment variables.
# For Mac OS X, set MACOS_SIGN_IDENTITY to the name of the certificate to use.
#

SIGNER=${PWS_SIGNER:="ronys@pwsafe.org"}
KEYID=${PWS_KEYID:="7F2F1BB9"}

MACOS_SIGN_IDENTITY=${MACOS_SIGN_IDENTITY:=""}


Usage () {
    echo "Usage: $0 DEB|RPM|PKG|DMG package-to-sign"
    exit 1
}


[ $# -eq 2 ] || Usage
[ "$1" = "DEB" ] || [ "$1" = "RPM" ] || [ "$1" = "PKG" ] || [ "$1" = "DMG" ] || Usage
[ -e $2 ] || Usage

TYPE=$1
PACKAGE=$2

if [ "$TYPE" = "PKG" ] || [ "$TYPE" = "DMG" ]; then
    if [ -z "$MACOS_SIGN_IDENTITY" ]; then
        echo "MACOS_SIGN_IDENTITY is not defined. Please set the signing identity."
        exit 2
    fi
fi

case "$TYPE" in
RPM)
    if command -v rpmsign >/dev/null 2>&1; then
        RPMSIGN=$(command -v rpmsign)
    else
        echo "Couldn't find rpmsign"
        exit 2
    fi
    $RPMSIGN --addsign --define "%_gpg_name $SIGNER" $PACKAGE
    ;;
DEB)
    if command -v dpkg-sig >/dev/null 2>&1; then
        DEBSIGN=$(command -v dpkg-sig)
    elif command -v debsigs >/dev/null 2>&1; then
        DEBSIGN=$(command -v debsigs)
    else
        echo "Couldn't find dpkg-sig or debsigs"
        exit 2
    fi
    $DEBSIGN --sign builder -k $KEYID $PACKAGE
    ;;
PKG)
    if command -v productsign >/dev/null 2>&1; then
        PRODUCTSIGN=$(command -v productsign)
    else
        echo "Couldn't find productsign"
        exit 2
    fi
    $PRODUCTSIGN --sign "$MACOS_SIGN_IDENTITY" $PACKAGE signed-$PACKAGE
    mv signed-$PACKAGE $PACKAGE
    ;;
DMG)
    if command -v codesign >/dev/null 2>&1; then
        CODESIGN=$(command -v codesign)
    else
        echo "Couldn't find codesign"
        exit 2
    fi
    $CODESIGN --sign "$MACOS_SIGN_IDENTITY" $PACKAGE
    ;;
*)
    Usage #Should have been caught earlier...
esac

exit 0
