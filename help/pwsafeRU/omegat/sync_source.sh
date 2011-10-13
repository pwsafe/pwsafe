#!/bin/sh
pwd
#clean old files
rm -rf ./source/*
mkdir -p ./source/html
#copy files
cp ../../default/pwsafe.hhc ./source/pwsafe.hhc
cp ../../default/html/* ./source/html
#apply helper patches
patch -p0 < omegat_helper.diff
#update ui tmx
po2tmx --source-language=EN-US --language=RU-RU ../../../src/ui/Windows/I18N/pos/pwsafe_ru.po tm/pwsafe_ru-ui.tmx
