# Makefile to build a Mac OS X distribution package (.dmg)
#
#
#
# Uncomment the correct TARGET and RELDIR below:
#TARGET=pwsafe
#TARGET=pwsafe-i386
###TARGET=pwsafe64
TARGET=pwsafe
#TARGET=pwsafe-llvm
#RELDIR=../../Xcode/build/Release/
#RELDIR=../../Xcode/build-i386/Release/
###RELDIR=../../Xcode/build64/Release/
#RELDIR=../../Xcode/build-llvm/Release/
#RELDIR=$(HOME)/Library/Developer/Xcode/DerivedData/pwsafe-xcode6-*/Build/Products/Release/
RELDIR=./../../Xcode/build/Products/Release/
# RELDIR=$(shell xcodebuild -project ../../Xcode/pwsafe-xcode6.xcodeproj -showBuildSettings | grep TARGET_BUILD_DIR | awk 'BEGIN { } /TARGET_BUILD_DIR =/{print $$3}' | sed "s/$$/\//")
# Path to wxWidgets library path has to be adapted for each environment
WXDIR=../../../wxWidgets-3.1.5/locale
# WXDIR=$(shell grep OTHER_LDFLAGS ../../Xcode/pwsafe-release.xcconfig | awk 'BEGIN { RS = " " } /-L/{print $$1}' | sed s/-L// | sed s/static-release\\/lib/locale/)
# Machine architecture
MACHINEARCH=$(shell uname -m)

CPBIN=/bin/cp
RMBIN=/bin/rm
MDBIN=/bin/mkdir -p

include ../../version.wx

ifndef VER_REV
VER_REV = 0
endif

ifdef VER_SPECIAL
RELEASENAME := $(VER_MAJOR).$(VER_MINOR).$(VER_REV).$(VER_SPECIAL)
else
RELEASENAME := $(VER_MAJOR).$(VER_MINOR).$(VER_REV)
endif

SRCDIR=./dmg
RESOURCES=$(SRCDIR)/pwsafe.app/Contents/Resources
DMGNAME = "../../$(TARGET)-$(MACHINEARCH)-$(RELEASENAME).dmg"
PWSAFE-APP=$(TARGET).app
DOCS := ../../README.md ../../docs/LICENSE.rtf ../../docs/ReleaseNotes.md 
HELP := ../../help/helpEN.zip ../../help/helpRU.zip ../../help/helpDE.zip ../../help/helpES.zip ../../help/helpFR.zip ../../help/helpZH.zip ../../help/helpPL.zip
MOS := ../../src/ui/wxWidgets/I18N/mos

all: dist
	echo Version $(RELEASENAME) generated in $(DMGNAME)

dist:
	$(MDBIN) $(SRCDIR)
	$(RMBIN) -rf $(SRCDIR)/*
	$(CPBIN) $(DOCS) $(SRCDIR)
	$(CPBIN) -R $(RELDIR)$(PWSAFE-APP) $(SRCDIR)
	$(CPBIN) $(HELP) $(RESOURCES)
	$(MDBIN) $(RESOURCES)/da.lproj
	$(CPBIN) $(MOS)/da/LC_MESSAGES/pwsafe.mo $(RESOURCES)/da.lproj
	$(CPBIN) $(WXDIR)/da.mo $(RESOURCES)/da.lproj/wxstd.mo
	$(MDBIN) $(RESOURCES)/de.lproj
	$(CPBIN) $(MOS)/de/LC_MESSAGES/pwsafe.mo $(RESOURCES)/de.lproj
	$(CPBIN) $(WXDIR)/de.mo $(RESOURCES)/de.lproj/wxstd.mo
	$(MDBIN) $(RESOURCES)/es.lproj
	$(CPBIN) $(MOS)/es/LC_MESSAGES/pwsafe.mo $(RESOURCES)/es.lproj
	$(CPBIN) $(WXDIR)/es.mo $(RESOURCES)/es.lproj/wxstd.mo
	$(MDBIN) $(RESOURCES)/fr.lproj
	$(CPBIN) $(MOS)/fr/LC_MESSAGES/pwsafe.mo $(RESOURCES)/fr.lproj
	$(CPBIN) $(WXDIR)/fr.mo $(RESOURCES)/fr.lproj/wxstd.mo
	$(MDBIN) $(RESOURCES)/hu.lproj
	$(CPBIN) $(MOS)/hu/LC_MESSAGES/pwsafe.mo $(RESOURCES)/hu.lproj
	$(CPBIN) $(WXDIR)/hu.mo $(RESOURCES)/hu.lproj/wxstd.mo
	$(MDBIN) $(RESOURCES)/it.lproj
	$(CPBIN) $(MOS)/it/LC_MESSAGES/pwsafe.mo $(RESOURCES)/it.lproj
	$(CPBIN) $(WXDIR)/it.mo $(RESOURCES)/it.lproj/wxstd.mo
	$(MDBIN) $(RESOURCES)/ko.lproj
	$(CPBIN) $(MOS)/ko/LC_MESSAGES/pwsafe.mo $(RESOURCES)/ko.lproj
	$(CPBIN) $(WXDIR)/ko_KR.mo $(RESOURCES)/ko.lproj/wxstd.mo
	$(MDBIN) $(RESOURCES)/nl.lproj
	$(CPBIN) $(MOS)/nl/LC_MESSAGES/pwsafe.mo $(RESOURCES)/nl.lproj
	$(CPBIN) $(WXDIR)/nl.mo $(RESOURCES)/nl.lproj/wxstd.mo
	$(MDBIN) $(RESOURCES)/pl.lproj
	$(CPBIN) $(MOS)/pl/LC_MESSAGES/pwsafe.mo $(RESOURCES)/pl.lproj
	$(CPBIN) $(WXDIR)/pl.mo $(RESOURCES)/pl.lproj/wxstd.mo
	$(MDBIN) $(RESOURCES)/ru.lproj
	$(CPBIN) $(MOS)/ru/LC_MESSAGES/pwsafe.mo $(RESOURCES)/ru.lproj
	$(CPBIN) $(WXDIR)/ru.mo $(RESOURCES)/ru.lproj/wxstd.mo
	$(MDBIN) $(RESOURCES)/sv.lproj
	$(CPBIN) $(MOS)/sv/LC_MESSAGES/pwsafe.mo $(RESOURCES)/sv.lproj
	$(CPBIN) $(WXDIR)/sv.mo $(RESOURCES)/sv.lproj/wxstd.mo
	$(MDBIN) $(RESOURCES)/zh.lproj
	$(CPBIN) $(MOS)/zh/LC_MESSAGES/pwsafe.mo $(RESOURCES)/zh.lproj
#	$(CPBIN) $(WXDIR)/zh_TW.mo $(RESOURCES)/zh.lproj/wxstd.mo
	$(CPBIN) $(WXDIR)/zh_CN.mo $(RESOURCES)/zh.lproj/wxstd.mo
	./create-dmg \
			--window-size 410 370 \
			--background ./background.png \
			--icon-size 40 \
			--volname "Password Safe" \
			--volicon ../../src/ui/wxWidgets/graphics/pwsafe.icns \
			--icon "$(PWSAFE-APP)" 00 110 \
			--app-drop-link 240 110 \
			--icon "README.md" 60  290 \
			--icon "ReleaseNotes.md" 150 290 \
			--icon "LICENSE.rtf" 250 290 \
		$(DMGNAME) \
		$(SRCDIR)

clean-dist:
	$(RMBIN) -f $(DMGNAME)

clean: clean-dist
	$(RMBIN) -rf $(SRCDIR)




