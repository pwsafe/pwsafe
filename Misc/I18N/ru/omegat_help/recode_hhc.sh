#!/bin/base -e
GIT_ROOT=`git rev-parse --show-toplevel`
HELP_PATH=${GIT_ROOT}/help/pwsafeRU
# hhc/hhk must be in cp1251
for ext in hhc hhk; do
    iconv -f utf-8 -t cp1251 ${HELP_PATH}/pwsafe.${ext} -o ${HELP_PATH}/pwsafeRU.${ext}
    sed -i -e 's/charset=UTF-8/charset=Windows-1251/' ${HELP_PATH}/pwsafeRU.${ext}
    rm ${HELP_PATH}/pwsafe.${ext}
done
