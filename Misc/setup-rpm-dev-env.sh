#!/bin/bash
# Silly script to setup the development pre-requisites for
# building PasswordSafe on RPM-based distros.
# Meant to be run as root or sudo'd
# Much room for improvement, error checking, reporting stuff, etc.
# Running this after cloning the git repo (or unpacking the sources)
# and you should be all set.

dnf install cmake gcc-c++ git gtest-devel libXt-devel libXtst-devel \
    libcurl-devel libuuid-devel libyubikey-devel \
    make openssl-devel wxGTK3-devel xerces-c-devel ykpers-devel qrencode-devel

#cd /usr/src/gtest
#mkdir build
#cd build
#cmake ..
#make
