#! /bin/sh
# Following from https://github.com/pwsafe/pwsafe/issues/543 by ykne.
# Mock can be installed via dnf or yum.
# With no argument compiles for current system.
# If added -r mock_arch, where mock_arch is a file name in /etc/mock without .cfg,
# then rpm will be compiled on specified architecture.
# Tested on Fedora-29 and CentOS-7
#
# To install the resulting RPM on CentOS7, the https://fedoraproject.org/wiki/EPEL repository
# should be installed.

mock --init "$@"
mock "$@" install cmake gcc-c++ git gtest-devel libXt-devel libXtst-devel libcurl-devel libuuid-devel libyubikey-devel make openssl-devel wxGTK3-devel xerces-c-devel ykpers-devel qrencode-devel
mock "$@" --chroot 'su - mockbuild -c "git clone https://github.com/pwsafe/pwsafe.git; mkdir -p pwsafe/build; cd pwsafe/build;  cmake -D wxWidgets_CONFIG_EXECUTABLE=/usr/libexec/wxGTK3/wx-config ..; cpack -G RPM"'
mock "$@" --copyout '/builddir/pwsafe/build/passwordsafe*.rpm' .

