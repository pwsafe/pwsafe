#!/bin/sh
POFILE=../../../../src/ui/Windows/I18N/pos/pwsafe_ru.po
pwd
#clean old files
rm -rf ./source/*
mkdir -p ./source ./glossary ./tm
#copy files
cp ../../../../help/pwsafeRU/omegat/glossary/* ./glossary
#cp ../../ReleaseNotes* ../../ChangeLog* ./source
#cp ../../pwsafe.1 ./source/pwsafe.1.utf8
chmod -X ./source/*
#apply helper patches
[ -e omegat_helper.diff ] && patch -p0 < omegat_helper.diff
#update ui tmx
po2tmx --source-language=EN-US --language=RU-RU $POFILE tm/pwsafe_ru-ui.tmx
#sync wxwidgets ui po
SRC_BASE=../../../../src
SRCS="${SRC_BASE}/ui/wxWidgets/*.cpp ${SRC_BASE}/core/*.cpp ${SRC_BASE}/os/linux/*.cpp"
xgettext --no-wrap --from-code=UTF-8 --default-domain=pwsafe --language=C++ --keyword=_ --output=./source/pwsafe.pot $SRCS
msgmerge --no-wrap --no-fuzzy-match --output=./source/pwsafe_ru_wx.po $POFILE ./source/pwsafe.pot
#rm  ./source/pwsafe.pot
