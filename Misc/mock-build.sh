#! /bin/sh
# Following from https://github.com/pwsafe/pwsafe/issues/543 by ykne.
# Mock can be installed via dnf or yum.
# With no argument compiles for current system.
# If added -r mock_arch, where mock_arch is a file name in /etc/mock without .cfg,
# then rpm will be compiled on specified architecture.
# Use -v WXVERSION to pin a specific wxGTK version instead of the latest.
# Tested on Fedora

usage() {
    echo "Usage: $0 [-v WXVERSION] [mock options]" >&2
    echo "  -v WXVERSION  Install this exact wxGTK version instead of the latest" >&2
    exit 1
}

WANT_VERSION=""

while getopts ":v:" opt; do
    case $opt in
        v) WANT_VERSION="$OPTARG" ;;
        :) echo "ERROR: -$OPTARG requires an argument" >&2; usage ;;
        \?) break ;;
    esac
done
shift $((OPTIND - 1))

CLONE_URL="https://github.com/pwsafe/pwsafe.git"
CLONE_OPTS=""
RPMDIR=$(rpm --eval '%_rpmdir')
RPMARCH=$(rpm --eval '%_arch')
DIST=$(rpm --eval '%{dist}' | sed 's/^\.//')

if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
    UPSTREAM=$(git rev-parse --abbrev-ref --symbolic-full-name @{u} 2>/dev/null)
    if [ -n "$UPSTREAM" ]; then
        REMOTE=$(echo "$UPSTREAM" | cut -d/ -f1)
        CLONE_URL=$(git remote get-url "$REMOTE")
        CLONE_OPTS="--branch $BRANCH --single-branch"
    fi
fi

mock --init "$@"
mock "$@" install cmake dpkg gcc-c++ git libXt-devel libXtst-devel libcurl-devel libuuid-devel libyubikey-devel make openssl-devel wxGTK-devel xerces-c-devel ykpers-devel qrencode-devel file-devel ImageMagick libayatana-appindicator-gtk3-devel

latest_rpm() {
    ls -1v "$@" 2>/dev/null | tail -n1
}

if [ -n "$WANT_VERSION" ]; then
    VER_GLOB="${WANT_VERSION}-*"
else
    VER_GLOB="[0-9]*"
fi

if ls "$RPMDIR/$RPMARCH"/wxGTK-${VER_GLOB}."$DIST.$RPMARCH.rpm" >/dev/null 2>&1; then
    WXBASE=$(latest_rpm "$RPMDIR/$RPMARCH"/wxBase-${VER_GLOB}."$DIST.$RPMARCH.rpm")
    WXBASE_DEVEL=$(latest_rpm "$RPMDIR/$RPMARCH"/wxBase-devel-${VER_GLOB}."$DIST.$RPMARCH.rpm")
    WXGTK=$(latest_rpm "$RPMDIR/$RPMARCH"/wxGTK-${VER_GLOB}."$DIST.$RPMARCH.rpm")
    WXGTK_DEVEL=$(latest_rpm "$RPMDIR/$RPMARCH"/wxGTK-devel-${VER_GLOB}."$DIST.$RPMARCH.rpm")
    WXGTK_GL=$(latest_rpm "$RPMDIR/$RPMARCH"/wxGTK-gl-${VER_GLOB}."$DIST.$RPMARCH.rpm")
    WXGTK_I18N=$(latest_rpm "$RPMDIR/noarch"/wxGTK-i18n-${VER_GLOB}."$DIST.noarch.rpm")
    WXGTK_MEDIA=$(latest_rpm "$RPMDIR/$RPMARCH"/wxGTK-media-${VER_GLOB}."$DIST.$RPMARCH.rpm")
    WXGTK_WEBVIEW=$(latest_rpm "$RPMDIR/$RPMARCH"/wxGTK-webview-${VER_GLOB}."$DIST.$RPMARCH.rpm")

    mock "$@" --copyin \
        "$WXBASE" \
        "$WXBASE_DEVEL" \
        "$WXGTK" \
        "$WXGTK_DEVEL" \
        "$WXGTK_GL" \
        "$WXGTK_I18N" \
        "$WXGTK_MEDIA" \
        "$WXGTK_WEBVIEW" \
        /tmp/
    mock "$@" --chroot "rpm -Uvh --force /tmp/wxBase-3*.rpm /tmp/wxBase-devel*.rpm /tmp/wxGTK-3*.rpm /tmp/wxGTK-devel*.rpm /tmp/wxGTK-gl*.rpm /tmp/wxGTK-i18n*.rpm /tmp/wxGTK-media*.rpm /tmp/wxGTK-webview*.rpm"
fi
mock "$@" --enable-network --unpriv --chroot "cd /builddir && git clone $CLONE_OPTS $CLONE_URL && mkdir -p pwsafe/build && cd pwsafe/build && cmake .. -DNO_GTEST=ON && cmake --build . -j\$(nproc) && cpack -G RPM"
mock "$@" --copyout '/builddir/pwsafe/build/passwordsafe*.rpm' .
