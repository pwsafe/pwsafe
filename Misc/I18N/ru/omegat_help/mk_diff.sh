#!/bin/sh
GIT_ROOT=`git rev-parse --show-toplevel`
LC_ALL=C diff -bNau $GIT_ROOT/help/default/html ./source/html >omegat_helper.diff
LC_ALL=C diff -bNau $GIT_ROOT/help/default/pwsafe.hhc ./source/pwsafe.hhc >>omegat_helper.diff
