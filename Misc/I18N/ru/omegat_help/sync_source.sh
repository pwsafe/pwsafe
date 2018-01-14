#!/bin/sh
GIT_ROOT=`git rev-parse --show-toplevel`
#clean old files
rm -rf ./source/*
mkdir -p ./source/html
#copy files
cp $GIT_ROOT/help/default/pwsafe.{hhc,hhk} ./source/
cp $GIT_ROOT/help/default/html/* ./source/html
chmod -X ./source/html/*
#apply helper patches
[ -e omegat_helper.diff ] && patch -p0 < omegat_helper.diff
#update ui tmx
po2tmx --source-language=EN-US --language=RU-RU $GIT_ROOT/src/ui/Windows/I18N/pos/pwsafe_ru.po ../tm/pwsafe_ru-ui.tmx
# update hhk
python ./create_index.py source/html source/pwsafe.hhk source/pwsafe.hhk --baseuri html
