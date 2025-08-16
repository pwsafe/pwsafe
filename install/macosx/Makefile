# Makefile to build a Mac OS X distribution package (.dmg)
#
#
#
# Define the correct TARGET and RELDIR below:
TARGET=pwsafe
PACKAGE=PasswordSafe-macOS
MACHINEARCH=universal
LANGS=da de es fr hu it ko nl pl ru sk sl sv zh

# XCODE_CONFIG and WX_SRC_DIR are defined in the macos-latest.yaml Github build workflow
XCODE_CONFIG ?= Release

ifndef RELDIR
RELDIR := $(shell xcodebuild -project ../../Xcode/pwsafe-xcode6.xcodeproj -showBuildSettings -scheme pwsafe -configuration $(XCODE_CONFIG) | awk '/TARGET_BUILD_DIR =/{print $$3}')
endif

ifdef WX_SRC_DIR
WXDIR := $(WX_SRC_DIR)/locale
else
WXDIR := $(shell grep OTHER_LDFLAGS ../../Xcode/pwsafe-release.xcconfig | awk 'BEGIN { RS = " " } /-L/{print $$1}' | sed s/-L// | sed s,/[^/]*/lib,/locale,)
endif

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
DMGNAME := "../../$(PACKAGE)-$(RELEASENAME)-$(MACHINEARCH).dmg"
PWSAFE-APP := $(TARGET).app
RESOURCES := $(RELDIR)/pwsafe.app/Contents/Resources
DOCS := ../../README.md ../../docs/LICENSE.rtf ../../docs/ReleaseNotesWX.md ../../docs/Yubico
HELP := ../../help/helpEN.zip ../../help/helpRU.zip ../../help/helpDE.zip ../../help/helpES.zip ../../help/helpFR.zip ../../help/helpZH.zip ../../help/helpSK.zip
I18N := ../../src/ui/wxWidgets/I18N
MOS := $(I18N)/mos

.PHONY: all loc help lang pwsI18N wxlocale dist clean clean-dist

all: loc help lang dist
	@echo Version $(RELEASENAME) generated in $(DMGNAME)

loc:
	@echo "Build location: $(RELDIR)"
	@echo "WX location: $(WXDIR)"

help:
	cd ../../help; make
	$(CPBIN) $(HELP) $(RESOURCES)

pwsI18N:
	cd $(I18N); make mos

wxlocale:
	cd $(WXDIR); make allmo

lang: pwsI18N wxlocale
	@for l in $(LANGS); do			\
		case $$l in			\
		ko)	LL=ko_KR;;		\
		zh)	LL=zh_CN;;		\
		 *)	LL=$$l;;		\
		esac;				\
		echo "Copying files for $$LL ...";	\
		$(MDBIN) $(RESOURCES)/$$l.lproj ;	\
		$(CPBIN) $(MOS)/$$l/LC_MESSAGES/pwsafe.mo $(RESOURCES)/$$l.lproj ;	\
		$(CPBIN) $(WXDIR)/$$LL.mo $(RESOURCES)/$$l.lproj/wxstd.mo ;		\
	done

dist: clean
	$(MDBIN) $(SRCDIR)
	$(CPBIN) -R $(DOCS) $(SRCDIR)
	$(CPBIN) -R $(RELDIR)/$(PWSAFE-APP) $(SRCDIR)
	/usr/bin/codesign -v --force --sign - -o runtime --entitlements \
		$(RELDIR)/../../Intermediates.noindex/pwsafe-xcode6.build/Release/pwsafe.build/pwsafe.app.xcent \
		--timestamp=none --generate-entitlement-der $(SRCDIR)/pwsafe.app
	create-dmg \
			--window-size 420 560 \
			--background ./background_flat_icon.png \
			--icon-size 40 \
			--volname "Password Safe" \
			--volicon $(SRCDIR)/$(PWSAFE-APP)/Contents/Resources/AppIcon.icns \
			--eula ../../LICENSE \
			--icon "$(PWSAFE-APP)" 00 110 \
			--app-drop-link 180 110 \
			--icon "README.md" 00  300 \
			--icon "ReleaseNotesWX.md" 140 300 \
			--icon "LICENSE.rtf" 00 380 \
			--icon "Yubico" 140 380 \
		$(DMGNAME) \
		$(SRCDIR)

clean-dist:
	@$(RMBIN) -f $(DMGNAME)

clean: clean-dist
	@$(RMBIN) -rf $(SRCDIR)

