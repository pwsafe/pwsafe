#!/bin/sh
GIT_ROOT=`git rev-parse --show-toplevel`
HELP_PATH=${GIT_ROOT}/help/pwsafeRU
for f in ${HELP_PATH}/html/*.html ${HELP_PATH}/*.{hhc,hhk}; do
    sed -i -e "s/␣/\&nbsp;/g" "${f}"
done
