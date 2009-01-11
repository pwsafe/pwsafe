# This makefile automates the build of releases for sourceforge
# The RELEASENAME should be changed per release
# The RELEASEDIR should be set to whatever works for you.
#
# 'make -f Makefile.release' or 'make -f Makefile.release release'
# will build both binary and source zipfiles.
# bin-release or src-release will build only what their names imply.
#
# Oh, this works with GNU make under Cygwin. YMMV on other makes...

RELEASENAME = 3.15.3

RELEASEDIR = "/cygdrive/c/local/src/PasswordSafe/Releases"

# Shouldn't need to change anything below this line

BINRELNAME = pwsafe-$(RELEASENAME)-bin
SRCRELNAME = pwsafe-$(RELEASENAME)-src

RM = /usr/bin/rm
CP = /usr/bin/cp
MV = /usr/bin/mv
SED = /usr/bin/sed
ZIP = /usr/bin/zip
SVN = /cygdrive/c/local/Subversion/bin/svn
GPG = /usr/local/bin/gpg
GPG_KEY = ronys@users.sourceforge.net
GPG_SIGN = $(GPG) --detach-sign --default-key $(GPG_KEY)
SHA1SUM = /usr/bin/sha1sum
UPLOAD_CMD = /usr/local/bin/rsync -avP -e ssh
UPLOAD_DST =  ronys@frs.sourceforge.net:uploads/
PYTHON = /usr/bin/python
SVN2LOG = ../../python/svn2log.py
MAKENSIS = /cygdrive/c/local/NSIS/makensis.exe
WIXDIR = /cygdrive/c/local/WIX/bin
CANDLE = $(WIXDIR)/candle.exe
LIGHT = $(WIXDIR)/light.exe

BIN_MANIFEST = README.txt docs/ReleaseNotes.txt LICENSE \
    xml/pwsafe.xsd xml/pwsafe_filter.xsd xml/pwsafe.xsl \
	docs/ChangeLog.txt src/bin/releasem/pwsafe.exe help/default/pwsafe.chm

.PHONY: all release bin-release src-release installables signatures \
	upload sha1sums msi nsis upload-latest updat-pos i18n

all: release installables signatures sha1sums

upload: upload-latest
	(cd $(RELEASEDIR); \
	 $(UPLOAD_CMD) pwsafe-$(RELEASENAME).exe \
	 $(BINRELNAME).zip $(SRCRELNAME).zip \
	 pwsafe-$(RELEASENAME).msi \
	 pwsafe-$(RELEASENAME).msi.sig \
	 pwsafe-$(RELEASENAME).exe.sig \
	 $(BINRELNAME).zip.sig $(SRCRELNAME).zip.sig $(UPLOAD_DST))

upload-latest: latest.xml
	echo "cd htdocs" > putlatest.tmp
	echo "put $<" >> putlatest.tmp
	sftp -b putlatest.tmp ronys,passwordsafe@web.sourceforge.net
	$(RM) putlatest.tmp

latest.xml: src/ui/Windows/version.h
	Misc/make-latest-xml.pl $< > $@
	chmod 644 $@

sha1sums:
	(cd $(RELEASEDIR); \
	 $(SHA1SUM) pwsafe-$(RELEASENAME).exe \
	 pwsafe-$(RELEASENAME).msi \
	 $(BINRELNAME).zip $(SRCRELNAME).zip)

signatures:
	$(GPG_SIGN) $(RELEASEDIR)/pwsafe-$(RELEASENAME).exe
	$(GPG_SIGN) $(RELEASEDIR)/$(BINRELNAME).zip
	$(GPG_SIGN) $(RELEASEDIR)/$(SRCRELNAME).zip
	$(GPG_SIGN) $(RELEASEDIR)/pwsafe-$(RELEASENAME).msi

installables: nsis msi

nsis:
	$(MAKENSIS) /DVERSION=$(RELEASENAME) install/pwsafe.nsi
	$(MV) install/pwsafe-$(RELEASENAME).exe $(RELEASEDIR)

msi:
	$(SED) 's/PWSAFE_VERSION/$(RELEASENAME)/' \
		< install/pwsafe-template.wxs > install/pwsafe.wxs
	$(CANDLE) install/pwsafe.wxs
	$(LIGHT) -ext WixUIExtension -cultures:en-us pwsafe.wixobj \
					-out pwsafe.msi
	$(RM) pwsafe.wixobj
	$(MV) pwsafe.msi $(RELEASEDIR)/pwsafe-$(RELEASENAME).msi

release: bin-release src-release i18n

bin-release:
	@-mkdir $(RELEASEDIR)/$(BINRELNAME)

	$(CP) $(BIN_MANIFEST) $(RELEASEDIR)/$(BINRELNAME)
	(cd $(RELEASEDIR); $(ZIP) -9 -r  foo ./$(BINRELNAME); \
	$(MV) foo.zip $(BINRELNAME).zip)
	@$(RM) -rf $(RELEASEDIR)/$(BINRELNAME)

src-release: ChangeLog I18N/pwsafe.pot
	$(SVN) export --non-interactive . C:\\TMP\\$(SRCRELNAME)
	$(MV) C:\\TMP\\$(SRCRELNAME) $(RELEASEDIR)
	$(MV) ChangeLog $(RELEASEDIR)/$(SRCRELNAME)
	$(CP) version.h $(RELEASEDIR)/$(SRCRELNAME)
	$(CP) I18N/pwsafe.pot $(RELEASEDIR)/$(SRCRELNAME)
	(cd $(RELEASEDIR); $(ZIP) -9 -r  bar ./$(SRCRELNAME); \
	$(MV) bar.zip $(SRCRELNAME).zip)
	@$(RM) -rf $(RELEASEDIR)/$(SRCRELNAME)

ChangeLog:
	$(SVN) log -v --xml | $(PYTHON) $(SVN2LOG) -L -H -s -O

I18N/pwsafe.pot: bin/language/pwsafe_base.dll
	$(MAKE) -C I18N pwsafe.pot

update-pos:
	$(MAKE) -C I18N $@

i18n:
	$(MAKE) -C I18N dlls


# Local variables:
# mode: Makefile
# End:
