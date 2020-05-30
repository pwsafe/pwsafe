#!/usr/bin/env bash
# Copyright 2019 Jacob Hrbek <kreyren@rixotstudio.cz>
# Distributed under the terms of the GNU General Public License v3 (https://www.gnu.org/licenses/gpl-3.0.en.html) or later

# shellcheck source=QA/travis-ci/travis-common.sh
. "QA/travis-ci/travis-common.sh"

# shellcheck source=QA/travis-ci/get-container.sh
. "QA/travis-ci/get-container.sh"

case "$TOOL" in
  *check*) true ;;
  *) die 0 "Skipping tests as instructed"
esac

# Linux as-is
if [ "$TRAVIS_OS_NAME" = linux ] && [ -z "$DOCKER" ] && [ -z "$QEMU" ]; then
	make test-rustlang

# Linux via Docker
elif [ "$TRAVIS_OS_NAME" = linux ] && [ -n "$DOCKER" ] && [ -z "$QEMU" ]; then
	sudo docker exec "$CONTAINER" make --directory="/travis/Zernit" test-rustlang

# MacOS X
elif [ "$TRAVIS_OS_NAME" = osx ]; then
	make --directory="/Users/travis/build/Kreyrock/Kreyrock" check

elif [ "$TRAVIS_OS_NAME" = "linux" ] && [ "$QEMU" = "FreeBSD" ]; then
	ssh -p 8829 -oStrictHostKeyChecking=no -i "$HOME/.ssh/id_rsa" root@localhost \
		make check

else
	die "Invalid setting of TRAVIS_OS_NAME=$TRAVIS_OS_NAME, DOCKER=$DOCKER, QEMU=$QEMU!"

fi
