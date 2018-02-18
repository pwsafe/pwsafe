#!/bin/sh
GIT_ROOT=`git rev-parse --show-toplevel`
HELP_PATH=${GIT_ROOT}/help/pwsafeRU

# make substitutions and change necoding
for f in ${HELP_PATH}/html/*.{html,js} ${HELP_PATH}/pwsafe.{hhc,hhk}; do
    sed -i -e "s/␣/\&nbsp;/g; s/→/\&rarr;/g; s/ü/\&uuml;/g; s/ß/\&szlig;/g;" "${f}"
    iconv -f utf-8 -t cp1251 "${f}" -o "${f}.cp1251" && \
    sed -i -e 's/charset=UTF-8/charset=Windows-1251/' "${f}.cp1251" && \
    mv "${f}.cp1251" "${f}" || { echo "Error in ${f}"; exit 1; }
done

# fix names
for ext in hhc hhk; do
    mv ${HELP_PATH}/pwsafe.${ext} ${HELP_PATH}/pwsafeRU.${ext}
done

