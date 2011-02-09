#!/bin/sh
#clean old files
rm -rf ./source/*
mkdir -p ./source/html
#copy files
cp ../../default/*.hh? ./source
cp ../../default/html/* ./source/html
#apply helper pathes
patch -p0 < omegat_helper.diff
#update ui tmx
po2tmx --source-language=EN-US --language=RU-RU ../../../src/ui/Windows/I18N/pos/pwsafe_ru.po omegat/pwsafe_ru-ui.tmx
