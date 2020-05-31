OS:= $(findstring Linux, $(shell uname -s))
GPG_KEY := 7F2F1BB9

SF_UPLOAD_ROOT := ronys@frs.sourceforge.net:/home/frs/project/p/pa/passwordsafe

.PHONY: all build test clean

all:
	@ $(error Invalid argument)
	@ exit 2

install-deps:
	@ Misc/name-me.sh

test-nameme-all: test-nameme-*

test-nameme-debian:
	@ sudo docker run debian sh -c 'apt-get update -q && apt-get install -qy busybox git lsb-release && git clone https://github.com/Kreyren/forkless-pwsafe.git && busybox sh forkless-pwsafe/Misc/name-me.sh && make -C forkless-pwsafe build'

test-nameme-ubuntu:
	@ sudo docker run ubuntu sh -c 'apt-get update -q && apt-get install -qy busybox git lsb-release && git clone https://github.com/Kreyren/forkless-pwsafe.git && busybox sh forkless-pwsafe/Misc/name-me.sh && make -C forkless-pwsafe build'

test-nameme-fedora:
	@ sudo docker run fedora sh -c 'dnf install -y busybox git redhat-lsb-core && git clone https://github.com/Kreyren/forkless-pwsafe.git && busybox sh forkless-pwsafe/Misc/name-me.sh && make -C forkless-pwsafe build'

test-nameme-archlinux:
	@ sudo docker run archlinux sh -c 'pacman -S busybox git lsb-release && git clone https://github.com/Kreyren/forkless-pwsafe.git && busybox sh forkless-pwsafe/Misc/name-me.sh && make -C forkless-pwsafe build'

# FIXME(Krey): lsb-release was not verified
test-nameme-nixos:
	@ sudo docker run nixos sh -c 'nix-env -iA busybox git lsb-release && git clone https://github.com/Kreyren/forkless-pwsafe.git && busybox sh forkless-pwsafe/Misc/name-me.sh && make -C forkless-pwsafe build'

# FIXME(Krey): Wasn't verified
test-nameme-alpine:
	@ sudo docker run alpine sh -c 'apk add busybox git lsb-release && git clone https://github.com/Kreyren/forkless-pwsafe.git && busybox sh forkless-pwsafe/Misc/name-me.sh && make -C forkless-pwsafe build'

test-nameme-freebsd:
	@ true

# Target to test gitpod on gitpod
test-gitpod:
	@ [ -n "$$GITPOD_GIT_USER_EMAIL" ] || { printf '\033[31m\033[1mFATAL:\033[0m %s\n' "This is not a gitpod envrionment capable of testing gitpod"; exit 1 ;}
	@ printf '\033[31m\033[1m!!! WARNING !!!:\033[0m %s\n' "This is going to create a new commit with a random message in your name, only test this in a new commit and ideally in a profile fork (y/n)"
	@ read -r something && [ "$$something" != y ] || exit 1
	@ [ -f DO_NOT_MERGE ] || printf '!!!WARNING!!! %s\n' "Target 'test-gitpod' has been invoked in this environment, verify prior merging!!!" > DO_NOT_MERGE
	@ [ -f temporary.Dockerfile ] || { printf '\033[31m\033[1mFATAL:\033[0m %s\n' "Expected file 'temporary.Dockerfile' for test-gitpod target is not present, create it?"; exit 1 ;}
	@ # Configure .gitpod.yml to accept temporary image
	@ [ -f ".gitpod.yml.original" ] || mv .gitpod.yml .gitpod.yml.original
	@ printf '%s\n' "image:" "  file: test.Dockerfile" > .gitpod.yml
	@ gp preview "$$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c10)"
	@ # Disable caching
	@ cat temporary.Dockerfile | sed -E "s#(RUN\s{1}true\s{1}\")(replace)(\"\s{1})#\1$$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c10)\3#gm" > test.Dockerfile
	@ #printf '%s\n' "" "RUN printf '%s\n' \"SUCCESS!\" && exit 99" >> test.Dockerfile
	@ git add * || true
	@ git commit -m "$$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c10)" || true
	@ git push || true
	@ gp preview https://gitpod.io/#$$(git remote -v | grep -m1 origin | sed 's/origin\s//g' | sed 's/ (fetch)//g' | sed 's/\.git//g')

build-release:
	@ [ ! -d build ] && mkdir build 
	@ [ ! -d build-dbg ] && mkdir build-dbg 
	@ cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
	@ cmake -S . -B build-dbg -D CMAKE_BUILD_TYPE=Debug

build:
	@ [ "$$(uname -s)" != Linux ] || $(MAKE) -f Makefile.linux
	@ [ "$$(uname -s)" != FreeBSD ] || $(MAKE) -f Makefile.freebsd
	@ [ "$$(uname -s)" != Darwin ] || $(MAKE) -f Makefile.macos
	@ # FIXME(Krey): Needs to be verified
	@ [ "$$(uname -s)" != Windows/Cygwin ] || $(MAKE) -f Makefile.windows