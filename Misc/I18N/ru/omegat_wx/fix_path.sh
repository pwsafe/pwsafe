#!/bin/sh
#mv ../pwsafe.1.utf8 ../pwsafe.1
echo -n "Fixing line breaks "
python adjust_newlines.py ../pwsafe_ru_wx.po --debug >nl.log 2>&1 && echo "OK" || { echo "FAILED"; exit 1; }
echo "Moving file to destination"
mv ../pwsafe_ru_wx.po ../../../../src/ui/wxWidgets/I18N/pos/pwsafe_ru.po
