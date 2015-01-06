#!/bin/sh
GIT_ROOT=`git rev-parse --show-toplevel`
HHC_PATH=$GIT_ROOT/help/pwsafeRU
iconv -f utf-8 -t cp1251 $HHC_PATH/pwsafe.hhc -o $HHC_PATH/pwsafeRU.hhc
sed -i -e "s/<HEAD>/<HEAD><meta http-equiv='content-type' content='text\/html; charset=Windows-1251'>/" $HHC_PATH/pwsafeRU.hhc
rm $HHC_PATH/pwsafe.hhc
