#!/bin/sh
GIT_ROOT=`git rev-parse --show-toplevel`
HELP_PATH=${GIT_ROOT}/help/pwsafeRU/html
for f in ${HELP_PATH}/*.html; do
    sed -i -e "s/␣/\&nbsp;/g" "${f}"
done
