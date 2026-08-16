#! /bin/sh
# Builds a pwsafe RPM, via mock, for an arbitrary Fedora or RHEL-clone
# target - from master, or from whatever branch is checked out locally -
# without needing a full pwsafe dev environment installed on this system,
# only mock itself. Useful as a clean-room sanity build of a local dev
# branch before pushing it.
#
# Can also optionally overlay newer wx*/libayatana-appindicator* packages
# from a local rpmbuild tree into the mock chroot only (see -v/-a below);
# this system's own installed packages are never touched either way.
#
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
    exit "${1:-1}"
}

WANT_WX_VERSION=""
WANT_APPINDICATOR_VERSION=""

while getopts ":hv:a:" opt; do
    case $opt in
        h) usage 0 ;;
        v) WANT_WX_VERSION="$OPTARG" ;;
        a) WANT_APPINDICATOR_VERSION="$OPTARG" ;;
        :) echo "ERROR: -$OPTARG requires an argument" >&2; usage ;;
        # An unrecognized flag (e.g. a mock option like -r) means option
        # parsing is done; getopts has already advanced OPTIND past it, so
        # back it up one or the shift below would swallow the flag itself
        # and leave its value as a stray positional argument.
        \?) OPTIND=$((OPTIND - 1)); break ;;
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

if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
    UPSTREAM=$(git rev-parse --abbrev-ref --symbolic-full-name @{u} 2>/dev/null)
    if [ -n "$UPSTREAM" ]; then
        # $BRANCH is interpolated unquoted into the chroot command string
        # below. Git branch names may legally contain shell metacharacters
        # (";", "$(...)", etc.), so restrict it to characters that can't
        # break out of that string before using it.
        case "$BRANCH" in
            *[!A-Za-z0-9._/-]*)
                echo "ERROR: current branch name '$BRANCH' contains characters unsafe to pass into the mock chroot (allowed: A-Z a-z 0-9 . _ / -)" >&2
                exit 1
                ;;
        esac
        REMOTE=$(echo "$UPSTREAM" | cut -d/ -f1)
        CLONE_URL=$(git remote get-url "$REMOTE")
        CLONE_OPTS="--branch $BRANCH --single-branch"
    fi
fi

mock --init "$@" || { echo "ERROR: mock --init failed" >&2; exit 1; }

# Query the dist tag and arch from the mock chroot itself, not the host -
# with -r targeting a different release or architecture (or a RHEL clone)
# than this host runs, the host's own %{dist}/%_arch (e.g. "fc42"/"x86_64")
# wouldn't match the local %_rpmdir build filenames for that target (e.g.
# "...fc44.aarch64.rpm").
DIST_ARCH=$(mock "$@" --quiet --chroot "rpm --eval '%{dist} %{_arch}'" 2>/dev/null)
DIST=$(echo "$DIST_ARCH" | cut -d' ' -f1 | sed 's/^\.//')
RPMARCH=$(echo "$DIST_ARCH" | cut -d' ' -f2)
if [ -z "$DIST" ] || [ -z "$RPMARCH" ]; then
    echo "ERROR: could not determine the dist tag/arch from the mock chroot" >&2
    exit 1
fi

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
    openssl-devel \
    wxGTK-devel \
    xerces-c-devel \
    ykpers-devel \
    qrencode-devel \
    file-devel \
    libayatana-appindicator-gtk3-devel \
    || { echo "ERROR: mock install failed" >&2; exit 1; }

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

# Collects local-build RPM paths for installed packages whose name starts
# with $2 into the shared OVERRIDE_RPMS/OVERRIDE_TMP_RPMS accumulators, later
# installed together in one rpm -Uvh transaction (wxGTK depends on
# libayatana-appindicator-gtk3, so mixing -v and -a needs both present at
# once). No-op if $1 (the requested version) is empty.
collect_local_build() {
    wantver="$1" name_base="$2"
    shift 2
    # "$@" is now just the trailing mock options, for the mock call below.
    [ -n "$wantver" ] || return 0

    verglob=$(ver_glob_for "$wantver")
    packages=$(mock "$@" --quiet --chroot "rpm -qa --qf '%{NAME}\n' '${name_base}*'" 2>/dev/null | grep -E "^${name_base}" | sort)

    for pkg in $packages; do
        rpm_path=$(local_rpm_path "$pkg" "$wantver" "$verglob") || exit 1
        if [ -z "$rpm_path" ]; then
            echo "ERROR: no $verglob rpm found for $pkg" >&2
            exit 1
        fi
        OVERRIDE_RPMS="$OVERRIDE_RPMS $rpm_path"
        OVERRIDE_TMP_RPMS="$OVERRIDE_TMP_RPMS /tmp/$(basename "$rpm_path")"
    done
}

OVERRIDE_RPMS=""
OVERRIDE_TMP_RPMS=""
collect_local_build "$WANT_WX_VERSION" 'wx' "$@"
collect_local_build "$WANT_APPINDICATOR_VERSION" 'libayatana-appindicator' "$@"

if [ -n "$OVERRIDE_RPMS" ]; then
    mock "$@" --copyin $OVERRIDE_RPMS /tmp/ || { echo "ERROR: mock --copyin failed" >&2; exit 1; }
    mock "$@" --chroot "rpm -Uvh --force ${OVERRIDE_TMP_RPMS# }" || { echo "ERROR: local package override install failed" >&2; exit 1; }
fi

mock "$@" --enable-network --unpriv --chroot "cd /builddir && git clone $CLONE_OPTS $CLONE_URL && mkdir -p pwsafe/build && cd pwsafe/build && cmake .. -DNO_GTEST=ON && cmake --build . -j\$(nproc) && cpack -G RPM" \
    || { echo "ERROR: build failed" >&2; exit 1; }
mock "$@" --copyout '/builddir/pwsafe/build/passwordsafe*.rpm' . || { echo "ERROR: copyout failed - was the RPM actually built?" >&2; exit 1; }
