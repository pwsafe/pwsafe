#!/bin/sh
# Silly script to setup the development pre-requisites for
# building PasswordSafe on Debian and RPM-based distros.
# Meant to be run as root or sudo'd
# Running this after cloning the git repo (or unpacking the sources)
# and you should be all set.
#
# Upgrade to this inspired by a similar script by kreyren
#
# Command overrides
if [ -z "$UNAME" ]; then
    UNAME="/usr/bin/uname"
    [ ! -x "$UNAME" ] && UNAME="/bin/uname"
fi
[ -z "$LSB_RELEASE" ] && LSB_RELEASE="/usr/bin/lsb_release"

die() {
    echo "$2"
    exit "$1"
}

# Exit on anything unexpected
set -e

# Check if we're effectively root
if [ ! "$(id -u)" = 0 ]; then
    die 1 "Run as root or under sudo"
fi

# See if we're Linux by checking the kernel
if command -v $UNAME 1>/dev/null; then
	KERNEL="$($UNAME -s)"
else
    die 2 "Could not find uname command, can't identify kernel"
fi

if [ "$KERNEL" = "Linux" ]; then
    # OK, let's see what distro we have here
    if command -v $LSB_RELEASE 1>/dev/null; then
        DISTRO="$($LSB_RELEASE -si | tr '[:upper:]' '[:lower:]')"
    elif [ -f /etc/os-release ]; then
        DISTRO="$(grep -o "^ID=.*" /etc/os-release | sed -e 's#^ID=##gm' -e 's/"//g')"
    elif ! command -v $LSB_RELEASE 1>/dev/null && [ ! -f /etc/os-release ]; then
        die 4 "Unable to identify distribution since command '$LSB_RELEASE' and file /etc/os-release are not present"
    else
        die 5 "Failed to identify distro in $0 running logic for Linux"
    fi

    # ... and the release number
    if command -v $LSB_RELEASE 1>/dev/null; then
        RELEASE="$($LSB_RELEASE -rs | sed 's/\([0-9]\)\..*/\1/')" # integer part, e.g., 20.04 --> 20
        if [ "$DISTRO" = "ubuntu" ] ; then
            RELEASE_MONTH="$($LSB_RELEASE -rs | sed 's/.*\.\([0-9]\)/\1/')" # integer part, e.g., 20.04 --> 04
        fi
    elif [ -f /etc/os-release ]; then
        RELEASE=$(awk -F= '/VERSION_ID/ {print $2}' /etc/os-release | sed s/\"//g | sed 's/\([0-9]\)\..*/\1/') # integer part, e.g., 20.04 --> 20
        if [ "$DISTRO" = "ubuntu" ] ; then
            RELEASE_MONTH=$(awk -F= '/VERSION_ID/ {print $2}' /etc/os-release | sed s/\"//g | sed 's/.*\.\([0-9]\)/\1/') # integer part, e.g., 20.04 --> 04
        fi
    else
        die 6 "Unable to determine release"
    fi
else
    die 3 "Sorry, can't configure for $KERNEL systems (yet)."
fi

[ -z "$RELEASE" ] && RELEASE=0 # debian testing and Arch don't have a release number
[ -z "$RELEASE_MONTH" ] && RELEASE_MONTH=0 # debian testing and Arch don't have a release month

# We have distro and release, let's get to work

case "$DISTRO" in
    debian|ubuntu|linuxmint|pop|raspbian)
        if test "$DISTRO" = "ubuntu" -a \
         \( \( "$RELEASE" -eq 22 -a "$RELEASE_MONTH" -eq 10 \) -o "$RELEASE" -gt 22 \) ; then
            LIBWXDEV="libwxgtk3.2-dev"
        elif test \( \( "$DISTRO" = "ubuntu" -o "$DISTRO" = "pop" -o "$DISTRO" = "linuxmint" \) -a "$RELEASE" -ge 20 \) -o \
         \( "$DISTRO" = "debian" -a "$RELEASE" -eq 11 \) ; then
            LIBWXDEV="libwxgtk3.0-gtk3-dev"
        elif test \( "$DISTRO" = "debian"  -a "$RELEASE" -eq 12 \) ; then
            LIBWXDEV="libwxgtk3.2-dev"
        elif test \( "$DISTRO" = "debian"  -a "$RELEASE" -eq 0 \) ; then
            LIBWXDEV="libwxgtk3.2-dev"
        else
            LIBWXDEV="libwxgtk3.0-dev"
        fi
        apt-get install -qy cmake fakeroot g++ gettext git libgtest-dev \
            libcurl4-openssl-dev libqrencode-dev  libssl-dev libuuid1 \
            $LIBWXDEV libxerces-c-dev libxt-dev libxtst-dev \
            libykpers-1-dev libyubikey-dev make pkg-config uuid-dev zip \
            libmagic-dev ninja-build
        # dpkg-sig is nice-to-have, not available on debian testing?
        apt-get install -qy dpkg-sig || (echo "dpkg-sig isn't mandatory"; true)
    ;;
    fedora|fedora-asahi-remix)
        if test "$RELEASE" -lt 39 ; then
            LIBWXBASE="wxBase3-devel"
            LIBWXGTK="wxGTK3-devel"
        else
            LIBWXBASE="wxBase-devel"
            LIBWXGTK="wxGTK-devel"
        fi
        dnf -y install cmake file-devel gcc-c++ git gtest-devel libXt-devel libXtst-devel \
        libcurl-devel libuuid-devel libyubikey-devel ninja-build \
        make openssl-devel rpmdevtools rpm-sign $LIBWXBASE $LIBWXGTK xerces-c-devel \
        ykpers-devel qrencode-devel
    ;;
    arch|manjarolinux)
        pacman -S --noconfirm base-devel cmake file git libxt ninja qrencode wxwidgets-gtk3 \
        xerces-c yubikey-personalization zip
    ;;
    opensuse*)
        zypper --non-interactive install cmake file-devel fakeroot gcc-c++ \
        gettext-tools git gtest libcurl-devel libmagic1 libopenssl-devel \
        libuuid-devel libxerces-c-devel libXt-devel libXtst-devel libykpers-devel \
        libyubikey-devel make ninja qrencode-devel rpmdevtools wxWidgets-3_2-devel zip
    ;;
    *) die 10 "Don't know how to setup $DISTRO release $RELEASE (yet)."
esac
exit 0
