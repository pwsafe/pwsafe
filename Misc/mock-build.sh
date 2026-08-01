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

# WANT_VERSION feeds into shell glob patterns (and RPM filenames) below;
# restrict it to characters RPM versions/releases actually use.
case "$WANT_VERSION" in
    "") ;;
    *[!A-Za-z0-9._+~-]*)
        echo "ERROR: -v value '$WANT_VERSION' contains invalid characters (allowed: A-Z a-z 0-9 . _ + ~ -)" >&2
        exit 1
        ;;
esac

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
mock "$@" install \
    cmake \
    dpkg \
    gcc-c++ \
    git \
    libXt-devel \
    libXtst-devel \
    libcurl-devel \
    libuuid-devel \
    libyubikey-devel \
    make \
    openssl-devel \
    wxGTK-devel \
    xerces-c-devel \
    ykpers-devel \
    qrencode-devel \
    file-devel \
    ImageMagick \
    libayatana-appindicator-gtk3-devel

latest_rpm() {
    ls -1v "$@" 2>/dev/null | tail -n1
}

case "$WANT_VERSION" in
    "") VER_GLOB="[0-9]*" ;;
    *-*) VER_GLOB="$WANT_VERSION" ;;      # already version-release, e.g. 3.2.12-2.sni
    *) VER_GLOB="${WANT_VERSION}-*" ;;    # bare version, e.g. 3.2.12: match any release
esac

wx_rpm_candidates() {
    pkg="$1"
    # $VER_GLOB may contain shell glob characters (e.g. "3.2.12-*" or "[0-9]*")
    # and must stay unquoted here so the shell expands it.
    { ls -1v "$RPMDIR/$RPMARCH"/${pkg}-${VER_GLOB}."$DIST.$RPMARCH.rpm" 2>/dev/null
      ls -1v "$RPMDIR/noarch"/${pkg}-${VER_GLOB}."$DIST.noarch.rpm" 2>/dev/null; }
}

# Resolves pkg to a single matching RPM path. With -v, more than one match is
# treated as an error (ambiguous pin) rather than silently picking the latest.
wx_rpm_path() {
    pkg="$1"
    candidates=$(wx_rpm_candidates "$pkg")
    count=$(printf '%s\n' "$candidates" | grep -c .)
    if [ -n "$WANT_VERSION" ] && [ "$count" -gt 1 ]; then
        echo "ERROR: -v $WANT_VERSION is ambiguous for $pkg, matches:" >&2
        printf '%s\n' "$candidates" >&2
        exit 1
    fi
    printf '%s\n' "$candidates" | tail -n1
}

# Only override wx* packages actually installed by the earlier `mock install`
# (e.g. wxGTK-devel and whatever it pulled in), not every known wx subpackage.
WX_PACKAGES=$(mock "$@" --quiet --chroot "rpm -qa --qf '%{NAME}\n' 'wx*'" 2>/dev/null | grep '^wx' | sort)

WX_RPMS=""
WX_TMP_RPMS=""
for pkg in $WX_PACKAGES; do
    rpm_path=$(wx_rpm_path "$pkg") || exit 1
    if [ -z "$rpm_path" ]; then
        echo "no ${VER_GLOB} rpm found for $pkg, leaving as installed" >&2
        continue
    fi
    WX_RPMS="$WX_RPMS $rpm_path"
    WX_TMP_RPMS="$WX_TMP_RPMS /tmp/$(basename "$rpm_path")"
done

if [ -n "$WX_RPMS" ]; then
    mock "$@" --copyin $WX_RPMS /tmp/
    mock "$@" --chroot "rpm -Uvh --force$WX_TMP_RPMS"
fi
mock "$@" --enable-network --unpriv --chroot "cd /builddir && git clone $CLONE_OPTS $CLONE_URL && mkdir -p pwsafe/build && cd pwsafe/build && cmake .. -DNO_GTEST=ON && cmake --build . -j\$(nproc) && cpack -G RPM"
mock "$@" --copyout '/builddir/pwsafe/build/passwordsafe*.rpm' .
