# Makefile to build a Mac OS X distribution package (.dmg)
#
#
#
# Uncomment the correct TARGET and RELDIR below:
#TARGET=pwsafe
#TARGET=pwsafe-i386
TARGET=pwsafe64
#TARGET=pwsafe-llvm
#RELDIR=../../Xcode/build/Release/
#RELDIR=../../Xcode/build-i386/Release/
RELDIR=../../Xcode/build64/Release/
#RELDIR=../../Xcode/build-llvm/Release/

CPBIN=/bin/cp
RMBIN=/bin/rm
MDBIN=/bin/mkdir -p

RELEASENAME := 0.9.0

SRCDIR=./dmg/
DMGNAME = ../../$(TARGET)-$(RELEASENAME).dmg
PWSAFE-APP=$(TARGET).app
DOCS := ../../README.md ../../docs/LICENSE.rtf ../../docs/ReleaseNotes.txt 

all: dist

dist:
	$(MDBIN) $(SRCDIR)
	$(RMBIN) -f $(SRCDIR).DS_Store
	$(CPBIN) $(DOCS) $(SRCDIR)
	$(CPBIN) -R $(RELDIR)$(PWSAFE-APP) $(SRCDIR)
	./create-dmg \
			--window-size 310 310 \
			--background ./background.png \
			--icon-size 40 \
			--volname "Password Safe" \
			--volicon ../../src/ui/wxWidgets/graphics/pwsafe.icns \
			--icon "$(PWSAFE-APP)" 100 95 \
			--app-drop-link 220 95 \
			--icon "README.md" 60  250 \
			--icon "ReleaseNotes.txt" 150 250 \
			--icon "LICENSE.rtf" 250 250 \
		$(DMGNAME) \
		$(SRCDIR)

clean-dist:
	$(RMBIN) -f $(DMGNAME)




