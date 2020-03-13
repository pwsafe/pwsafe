#! /bin/sh
# Following from https://github.com/pwsafe/pwsafe/issues/543 by ykne.
# Mock can be installed via dnf or yum.
# With no argument compiles for current system.
# If added -r mock_arch, where mock_arch is a file name in /etc/mock without .cfg,
# then rpm will be compiled on specified architecture.
# Tested on Fedora-30 and CentOS-7
# Build on or for CentOS-7 requires current mock* for devtoolset-7-gcc-c++ access.
#
# To install the resulting RPM on CentOS7, the https://fedoraproject.org/wiki/EPEL repository
# should be installed.

mock --init "$@"
if [ "$2" = epel-7-x86_64 -o \( -z "$2" -a $(readlink /etc/mock/default.cfg) = epel-7-x86_64.cfg \) ];then
    mock "$@" install cmake gcc-c++ git gtest-devel libXt-devel libXtst-devel libcurl-devel libuuid-devel libyubikey-devel make openssl-devel wxGTK3-devel xerces-c-devel ykpers-devel qrencode-devel file-devel file-devel redhat-lsb-core devtoolset-7-gcc-c++
    mock "$@" --enable-network --chroot 'su - mockbuild -c "git clone https://github.com/pwsafe/pwsafe.git; mkdir -p pwsafe/build; cd pwsafe/build; scl enable devtoolset-7 -- cmake -D wxWidgets_CONFIG_EXECUTABLE=/usr/libexec/wxGTK3/wx-config ..; scl enable devtoolset-7 -- cpack -G RPM"'
else
    mock "$@" install cmake gcc-c++ git gtest-devel libXt-devel libXtst-devel libcurl-devel libuuid-devel libyubikey-devel make openssl-devel wxGTK3-devel xerces-c-devel ykpers-devel qrencode-devel file-devel file-devel redhat-lsb-core
    mock "$@" --enable-network --chroot 'su - mockbuild -c "git clone https://github.com/pwsafe/pwsafe.git; mkdir -p pwsafe/build; cd pwsafe/build; cmake ..; cpack -G RPM"'
fi
mock "$@" --copyout '/builddir/pwsafe/build/passwordsafe*.rpm' .
