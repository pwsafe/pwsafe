# Makefile to build a Mac OS X distribution package (.dmg)
#
#
#
# Uncomment the correct TARGET and RELDIR below:
TARGET=pwsafe
MACHINEARCH=universal
WXDIR := $(shell grep OTHER_LDFLAGS ../../Xcode/pwsafe-release.xcconfig | awk 'BEGIN { RS = " " } /-L/{print $$1}' | grep static-release | sed s/-L// | sed s,static-release/lib,locale,)

# XCODE_CONFIG is defined in the macos-latest.yaml Github build workflow
ifndef XCODE_CONFIG
XCODE_CONFIG=Release
endif
RELDIR := $(shell xcodebuild -project ../../Xcode/pwsafe-xcode6.xcodeproj -showBuildSettings -scheme pwsafe -configuration $(XCODE_CONFIG) | awk '/TARGET_BUILD_DIR =/{print $$3}')

#TARGET=pwsafe
#TARGET=pwsafe-i386
###TARGET=pwsafe64
#TARGET=pwsafe-llvm
#RELDIR=../../Xcode/build/Release/
#RELDIR=../../Xcode/build-i386/Release/
###RELDIR=../../Xcode/build64/Release/
#RELDIR=../../Xcode/build-llvm/Release/
#RELDIR=$(HOME)/Library/Developer/Xcode/DerivedData/pwsafe-xcode6-*/Build/Products/Release/
# RELDIR=./../../Xcode/build/Products/Release/
#RELDIR=$(shell xcodebuild -project ../../Xcode/pwsafe-xcode6.xcodeproj -showBuildSettings | grep TARGET_BUILD_DIR | awk 'BEGIN { } /TARGET_BUILD_DIR =/{print $$3}' | sed "s/$$/\//")
# Path to wxWidgets library path has to be adapted for each environment
# WXDIR=../../../wxWidgets-3.1.5/locale
# Machine architecture - these days we are building Universal Binaries
#MACHINEARCH=$(shell uname -m)

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
DMGNAME := "../../$(TARGET)-$(MACHINEARCH)-$(RELEASENAME).dmg"
PWSAFE-APP := $(TARGET).app
RESOURCES := $(RELDIR)/pwsafe.app/Contents/Resources
DOCS := ../../README.md ../../docs/LICENSE.rtf ../../docs/ReleaseNotesWX.md ../../docs/Yubico ../../docs/wxWidgets
HELP := ../../help/helpEN.zip ../../help/helpRU.zip ../../help/helpDE.zip ../../help/helpES.zip ../../help/helpFR.zip ../../help/helpZH.zip ../../help/helpPL.zip
I18N := ../../src/ui/wxWidgets/I18N
MOS := $(I18N)/mos

all: loc help lang dist
	@echo Version $(RELEASENAME) generated in $(DMGNAME)

loc:
	@echo "Build location: $(RELDIR)"
	@echo "WX location: $(WXDIR)"

help:
	cd ../../help; make

I18N:
	cd $(I18N); make mos

lang: I18N
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
	$(CPBIN) $(WXDIR)/zh_CN.mo $(RESOURCES)/zh.lproj/wxstd.mo
#	$(CPBIN) $(WXDIR)/zh_TW.mo $(RESOURCES)/zh.lproj/wxstd.mo

dist: clean
	$(MDBIN) $(SRCDIR)
	$(CPBIN) -R $(DOCS) $(SRCDIR)
	$(CPBIN) -R $(RELDIR)/$(PWSAFE-APP) $(SRCDIR)
	create-dmg \
			--window-size 420 560 \
			--background ./background.png \
			--icon-size 40 \
			--volname "Password Safe" \
			--volicon ../../src/ui/wxWidgets/graphics/pwsafe.icns \
                        --eula ../../LICENSE \
			--icon "$(PWSAFE-APP)" 00 110 \
			--app-drop-link 180 110 \
			--icon "README.md" 00  300 \
			--icon "ReleaseNotesWX.md" 140 300 \
			--icon "LICENSE.rtf" 00 380 \
			--icon "wxWidgets" 95 380 \
			--icon "Yubico" 175 380 \
		$(DMGNAME) \
		$(SRCDIR)

clean-dist:
	@$(RMBIN) -f $(DMGNAME)

clean: clean-dist
	@$(RMBIN) -rf $(SRCDIR)

