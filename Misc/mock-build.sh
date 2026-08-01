#! /bin/sh
# Following from https://github.com/pwsafe/pwsafe/issues/543 by ykne.
# Mock can be installed via dnf or yum.
# With no argument compiles for current system.
# If added -r mock_arch, where mock_arch is a file name in /etc/mock without .cfg,
# then rpm will be compiled on specified architecture.
# Use -v WXVERSION to override wx* packages with a local build from %_rpmdir
# instead of whatever the mock chroot's repos provide.
# Use -a APPINDICATORVERSION likewise for libayatana-appindicator* packages.
# Neither is pinned by default - with no -v/-a, mock installs stock repo
# packages for both and nothing under %_rpmdir is even looked at.
# Tested on Fedora

usage() {
    echo "Usage: $0 [-v WXVERSION] [-a APPINDICATORVERSION] [mock options]" >&2
    echo "  -v WXVERSION            Override wx* packages with this local %_rpmdir build" >&2
    echo "  -a APPINDICATORVERSION  Override libayatana-appindicator* packages with this local %_rpmdir build" >&2
    exit 1
}

WANT_WX_VERSION=""
WANT_APPINDICATOR_VERSION=""

while getopts ":v:a:" opt; do
    case $opt in
        v) WANT_WX_VERSION="$OPTARG" ;;
        a) WANT_APPINDICATOR_VERSION="$OPTARG" ;;
        :) echo "ERROR: -$OPTARG requires an argument" >&2; usage ;;
        \?) break ;;
    esac
done
shift $((OPTIND - 1))

# Both values feed into shell glob patterns (and RPM filenames) below;
# restrict them to characters RPM versions/releases actually use.
for _v in "$WANT_WX_VERSION" "$WANT_APPINDICATOR_VERSION"; do
    case "$_v" in
        "") ;;
        *[!A-Za-z0-9._+~-]*)
            echo "ERROR: version '$_v' contains invalid characters (allowed: A-Z a-z 0-9 . _ + ~ -)" >&2
            exit 1
            ;;
    esac
done
unset _v

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

# version -> glob against local RPM filenames, e.g. "3.2.12" -> "3.2.12-*",
# "3.2.12-2.sni" (already version-release) -> unchanged.
ver_glob_for() {
    case "$1" in
        *-*) echo "$1" ;;
        *) echo "$1-*" ;;
    esac
}

local_rpm_candidates() {
    pkg="$1" verglob="$2"
    # $verglob may contain shell glob characters (e.g. "3.2.12-*") and must
    # stay unquoted here so the shell expands it.
    { ls -1v "$RPMDIR/$RPMARCH"/${pkg}-${verglob}."$DIST.$RPMARCH.rpm" 2>/dev/null
      ls -1v "$RPMDIR/noarch"/${pkg}-${verglob}."$DIST.noarch.rpm" 2>/dev/null; }
}

# Resolves pkg to a single matching RPM path. More than one match is always
# treated as an error (ambiguous pin) - this is only ever called when a
# version was explicitly requested, so there's no "just pick the latest"
# fallback to silently paper over an ambiguous glob.
local_rpm_path() {
    pkg="$1" wantver="$2" verglob="$3"
    candidates=$(local_rpm_candidates "$pkg" "$verglob")
    count=$(printf '%s\n' "$candidates" | grep -c .)
    if [ "$count" -gt 1 ]; then
        echo "ERROR: -v/-a $wantver is ambiguous for $pkg, matches:" >&2
        printf '%s\n' "$candidates" >&2
        exit 1
    fi
    printf '%s\n' "$candidates" | tail -n1
}

# Collects RPM paths for packages matching $2 (an `rpm -qa` glob, e.g. 'wx*')
# that are actually installed in the chroot, at local %_rpmdir build $1, into
# the shared $OVERRIDE_RPMS/$OVERRIDE_TMP_RPMS accumulators below. Does
# nothing at all if $1 is empty - no query, no accumulation.
#
# wxGTK's package Requires libayatana-appindicator-gtk3 >= 0.6.0, so if both
# -v and -a are given, both families' RPMs must land in a single `rpm -Uvh`
# transaction (see the combined install below) - installing wx by itself
# first would fail that dependency against whatever stock appindicator is
# still installed at that point.
collect_local_build() {
    wantver="$1" name_glob="$2" grep_anchor="$3"
    shift 3
    # "$@" is now just the trailing mock options, for the mock call below.
    [ -n "$wantver" ] || return 0

    verglob=$(ver_glob_for "$wantver")
    packages=$(mock "$@" --quiet --chroot "rpm -qa --qf '%{NAME}\n' '$name_glob'" 2>/dev/null | grep -E "$grep_anchor" | sort)

    for pkg in $packages; do
        rpm_path=$(local_rpm_path "$pkg" "$wantver" "$verglob") || exit 1
        if [ -z "$rpm_path" ]; then
            echo "no $verglob rpm found for $pkg, leaving as installed" >&2
            continue
        fi
        OVERRIDE_RPMS="$OVERRIDE_RPMS $rpm_path"
        OVERRIDE_TMP_RPMS="$OVERRIDE_TMP_RPMS /tmp/$(basename "$rpm_path")"
    done
}

OVERRIDE_RPMS=""
OVERRIDE_TMP_RPMS=""
collect_local_build "$WANT_WX_VERSION" 'wx*' '^wx' "$@"
collect_local_build "$WANT_APPINDICATOR_VERSION" 'libayatana-appindicator*' '^libayatana-appindicator' "$@"

if [ -n "$OVERRIDE_RPMS" ]; then
    mock "$@" --copyin $OVERRIDE_RPMS /tmp/
    mock "$@" --chroot "rpm -Uvh --force$OVERRIDE_TMP_RPMS"
fi

mock "$@" --enable-network --unpriv --chroot "cd /builddir && git clone $CLONE_OPTS $CLONE_URL && mkdir -p pwsafe/build && cd pwsafe/build && cmake .. -DNO_GTEST=ON && cmake --build . -j\$(nproc) && cpack -G RPM"
mock "$@" --copyout '/builddir/pwsafe/build/passwordsafe*.rpm' .
