# Makefile to create a Deb file for wxWidgets version of pwsafe

RELNUM = 0.2-1
DISTRO = $(shell echo $(word 2,$(shell lsb_release -d))| tr [:upper:] [:lower:])

DEBNAME = passwordsafe-$(DISTRO)-$(RELNUM).deb

MKDIR = /bin/mkdir -p
CAT = /bin/cat
MV = /bin/mv
CP = /bin/cp
RM = /bin/rm -rf
SED = /bin/sed

RELDIR = ../../src/ui/wxWidgets/GCCUnicodeRelease
DOCSDIR = ../../docs
XMLDIR = ../../xml

PWSAFE-BIN = $(RELDIR)/pwsafe
DOCS = ../../README.txt $(DOCSDIR)/ReleaseNotes.txt \
	$(DOCSDIR)/ChangeLog.txt
MANPAGE = $(DOCSDIR)/pwsafe.1
XMLFILES = $(XMLDIR)/pwsafe.xsd $(XMLDIR)/pwsafe_filter.xsd $(XMLDIR)/pwsafe.xsl

INFILES = $(PWSAFE-BIN) $(DOCS) $(MANPAGE) $(XMLFILES)

.PHONY: all deb debtree clean

all : $(DEBNAME)

$(DEBNAME): $(INFILES)
	@ln -sf control-$(DISTRO) control
	$(MAKE) deb
	@$(RM) control

deb: debtree
	@fakeroot dpkg-deb --build debian $(DEBNAME)
	@$(RM) debian

debtree:
	@$(MKDIR) debian/usr/bin
	@$(MKDIR) debian/usr/share/pwsafe/xml
	@$(MKDIR) debian/usr/share/doc/passwordsafe
	@$(MKDIR) debian/usr/share/man/man1
	@$(MKDIR) debian/tmp
	@$(CP) $(PWSAFE-BIN) debian/usr/bin
	@strip debian/usr/bin/pwsafe
	@$(CP) $(DOCS) debian/usr/share/doc/passwordsafe
	@$(CAT) ./copyright ../../LICENSE > \
		debian/usr/share/doc/passwordsafe/copyright
	@$(CP) ./changelog.Debian debian/usr/share/doc/passwordsafe
	@(cd debian/usr/share/doc/passwordsafe; \
		$(MV) ChangeLog.txt changelog; gzip -9 changelog; gzip -9 changelog.Debian)
	@$(CP) $(MANPAGE) debian/usr/share/man/man1
	@gzip -9 debian/usr/share/man/man1/pwsafe.1
	@$(CP) $(XMLFILES) debian/usr/share/pwsafe/xml
	@$(MKDIR) debian/DEBIAN
	@$(SED) s/VERSION/$(RELNUM)/ < control > debian/DEBIAN/control
	@$(CP) ../desktop/pwsafe.desktop debian/tmp
	@$(CP) ../../src/ui/graphics/pwsafe.png debian/tmp
	@$(CP) postinst debian/DEBIAN
	@$(CP) prerm debian/DEBIAN

clean:
	@$(RM) debian $(DEBNAME)
