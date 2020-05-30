#!/usr/bin/env bash
# shellcheck disable=SC1117
# Copyright 2019 Jacob Hrbek <kreyren@rixotstudio.cz>
# Distributed under the terms of the GNU General Public License v3 (https://www.gnu.org/licenses/gpl-3.0.en.html) or later
# Based in part upon 'install.sh' from rsplib (https://raw.githubusercontent.com/dreibh/rsplib/master/ci/install), which is:
# 		Copyright (C) 2018-2019 by Thomas Dreibholz <dreibh@iem.uni-due.de> as GPLv3 or any other GPL at your option

# shellcheck source=QA/travis-ci/travis-common.sh
. "QA/travis-ci/travis-common.sh"

# shellcheck source=QA/travis-ci/get-container.sh
. "QA/travis-ci/get-container.sh"

fixme "Travis - install.sh is disabling SC1117 as hotfix"

# Linux as-is
if [ "$TRAVIS_OS_NAME" = linux ] && [ -z "$DOCKER" ] && [ -z "$QEMU" ]; then

	# Update repositories based on available package manager
	if command -v apt >/dev/null; then
		sudo apt update || die 1 "Unable to update repositories on VARIANT '$VARIANT'"
	elif command -v cave >/dev/null; then
		cave sync || die 1 "Unable to update repositories on paludis"
	else
		die 256 "Unsupported package manager has been used"
	fi

	# Install dependencies based on variant used
	case "$VARIANT" in
		ubuntu-*|debian-*)
			# Install dependencies
			if [ -n "$PACKAGES" ]; then
				sudo apt install -y "$PACKAGES" || die 1 "Unable to install following packages: '$PACKAGES'"
			elif [ -z "$PACKAGES" ]; then
				info "Dependencies are not specified in PACKAGES variable"
			else
				die 256 "Unexpected happend while processing PACKAGES variable"
			fi
		;;
		*) die 2 "Unsupported variant '$VARIANT' has been parsed"
	esac

	# Do not clean repositories since docker images are not saved

# Linux via Docker
elif [ "$TRAVIS_OS_NAME" = linux ] && [ -n "$DOCKER" ] && [ -z "$QEMU" ]; then

	fixme "Fix duplicates of repository update"

	case "$VARIANT" in
		ubuntu-*|debian-*)
			# Update repositories
			docker exec "$CONTAINER" apt update || die 1 "Unable to update repositories on '$VARIANT' using DOCKER '$DOCKER'"

			# Install dependencies
			if [ -n "$PACKAGES" ]; then
				# NOTICE: DO not double quote PACKAGES
				docker exec "$CONTAINER" apt install -y $PACKAGES || die 1 "Unable to install following packages on '$CONTAINER': '$PACKAGES'"
			elif [ -z "$PACKAGES" ]; then
				true
			else
				die 256 "Resolving packages for '$CONTAINER'"
			fi
		;;
		*) die 256 "Unsupported variant '$VARIANT' has been parsed in DOCKER '$DOCKER'"
	esac

	info "Fetching repository for $VARIANT"

	sudo docker exec "$CONTAINER" git clone "$REPOSITORY" || die 1 "Unable to fetch '$REPOSITORY'"

# MacOS X
elif [ "$TRAVIS_OS_NAME" = osx ]; then
	# Homebrew takes lots of time on runtime due to the cleanup used, this is a hotfix (https://travis-ci.community/t/macosx-brew-update-takes-too-much-time/6295)
	HOMEBREW_NO_INSTALL_CLEANUP=1 brew update || die "Unable to update brew"

	info "Installing dependencies"

	if [ -n "$PACKAGES" ]; then
		brew install $PACKAGES || die "Unable to install dependencies on $TRAVIS_OS_NAME"
	elif [ -z "$PACKAGES" ]; then
		true
	else
		die 256 "Unexpected happend while installing packages on '$TRAVIS_OS_NAME'"
	fi

# FreeBSD via QEMU
elif [ "$TRAVIS_OS_NAME" = "linux" ] && [ "$QEMU" = "FreeBSD" ]; then
	if [ -n "$VARIANT" ]; then
		# Install packages
		# Ensure the file system is true (fuse-ufs2 in write mode is unreliable!)
		ssh -p 8829 -oStrictHostKeyChecking=no -i "$HOME/.ssh/id_rsa" root@localhost \
			"mount -fr / ; fsck -y /dev/gpt/rootfs ; mount -fw / ; df -h"

		# Basic dependencies:
		ssh -p 8829 -oStrictHostKeyChecking=no -i "$HOME/.ssh/id_rsa" root@localhost \
			env ASSUME_ALWAYS_YES=yes pkg update
		ssh -p 8829 -oStrictHostKeyChecking=no -i "$HOME/.ssh/id_rsa" root@localhost \
			env ASSUME_ALWAYS_YES=yes pkg install -y bash autoconf meson cppcheck fusefs-libs3 gcc git ninja bison libtool autoconf pkg-config indent fakeroot gzip rsync autopoint shellcheck

		# Bash shell:
		# Use bash, and make sure it is available under /bin/bash.
		ssh -p 8829 -oStrictHostKeyChecking=no -i "$HOME/.ssh/id_rsa" root@localhost \
			chsh -s /usr/local/bin/bash
		ssh -p 8829 -oStrictHostKeyChecking=no -i "$HOME/.ssh/id_rsa" root@localhost \
			ln -s /usr/local/bin/bash /bin/bash || true

		# Ports collection:
		# This is the slow method via portsnap:
		# --- ssh -p 8829 -oStrictHostKeyChecking=no -i "$HOME/.ssh/id_rsa" root@localhost \
		# --- "portsnap --interactive fetch extract | grep -v ^/usr/ports"
		# Using Git is much faster:
		ssh -p 8829 -oStrictHostKeyChecking=no -i "$HOME/.ssh/id_rsa" root@localhost \
			"rm -rf /usr/ports ; git clone --depth=1 --filter=tree:0 https://github.com/freebsd/freebsd-ports /usr/ports"

		# Package's dependencies:
		ssh -p 8829 -oStrictHostKeyChecking=no -i "$HOME/.ssh/id_rsa" root@localhost \
			"cd /travis/freebsd/*/ && ( make build-depends-list && make run-depends-list ) | sed -e 's/^.*\///g' -e 's/glib20/glib/g' | sort -u | xargs -r env ASSUME_ALWAYS_YES=yes pkg install -y"

		echo "===== The FreeBSD VM is ready! ====="

	elif [ -z "$VARIANT" ]; then
		die "Variable VARIANT is not set for FreeBSD via QEMU which is fatal, This should be set in travis.yml"

	else
		die "Unexpected in FreeBSD using QEMU"

	fi
else
	die "Invalid setting of TRAVIS_OS_NAME=$TRAVIS_OS_NAME, DOCKER=$DOCKER, QEMU=$QEMU!"

fi
