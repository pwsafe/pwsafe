#!/bin/bash
# Silly script to setup the development pre-requisites for
# building PasswordSafe on Debian-based distros.
# Meant to be run as root or sudo'd
# Much room for improvement, error checking, reporting stuff, etc.
# Running this after cloning the git repo (or unpacking the sources)
# and you should be all set.

# Set your distro name here.  E.g. distro=stretch
distro=

if ! apt-cache show libwxgtk3.0-dev
then
  # The app catalog is missing wxWidgets, so a repository must be added
  if [ -z "${distro}" ]
  then
    # Try to guess the distro since it's unset.
    distro=$(awk '/debian-security/{print $3}' /etc/apt/sources.list)
  fi
  if [ -z "${distro}" ]
  then
    printf '%s\n' "Error: distro name could not be parsed from /etc/apt/sources.list." "Defaulting to stable." "Edit $0 and set your distro name on the first line"
    exit 1
  fi
  apt-key adv --fetch-keys http://repos.codelite.org/CodeLite.asc
  apt-add-repository "deb http://repos.codelite.org/wx3.1.0/debian/ ${distro%%/*} libs"
  aptitude update
fi

apt-get install cmake fakeroot g++ gettext git libgtest-dev \
	libqrencode-dev libuuid1 libwxgtk3.0-dev libxerces-c-dev \
	libxt-dev libxtst-dev libykpers-1-dev libyubikey-dev make \
	pkg-config uuid-dev zip

cd /usr/src/gtest
mkdir build
cd build
cmake ..
make
