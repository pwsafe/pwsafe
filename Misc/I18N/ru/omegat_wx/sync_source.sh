#!/bin/sh

GIT_ROOT=`git rev-parse --show-toplevel`
POFILE=${GIT_ROOT}/src/ui/Windows/I18N/pos/pwsafe_ru.po
pwd
#clean old files
rm -rf ./source/*
mkdir -p ./source
#copy files
#cp ../../ReleaseNotes* ../../ChangeLog* ./source
#cp ../../pwsafe.1 ./source/pwsafe.1.utf8
#chmod -X ./source/*

# Sync core string
${GIT_ROOT}/Misc/rc2cpp.pl ${GIT_ROOT}/src/core/core.rc2
#apply helper patches
[ -e omegat_helper.diff ] && patch -p0 < omegat_helper.diff
#update ui tmx
po2tmx --source-language=EN-US --language=RU-RU $POFILE ../tm/pwsafe_ru-ui.tmx || exit 1
#sync wxwidgets ui po
SRC_BASE=${GIT_ROOT}/src
SRCS="${SRC_BASE}/ui/wxWidgets/*.h ${SRC_BASE}/ui/wxWidgets/*.cpp ${SRC_BASE}/core/*.h ${SRC_BASE}/core/*.cpp ${SRC_BASE}/os/unix/*.h ${SRC_BASE}/os/unix/*.cpp"
xgettext --no-wrap --from-code=UTF-8 --default-domain=pwsafe --language=C++ --keyword=_ --keyword=wxTRANSLATE --output=./source/pwsafe.pot $SRCS || exit 1
msgmerge --no-wrap --no-fuzzy-match --output=./source/pwsafe_ru_wx.po $POFILE ./source/pwsafe.pot || exit 1
rm  ./source/pwsafe.pot
