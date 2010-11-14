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
ZIP = /usr/bin/zip

RELDIR = ../../src/ui/wxWidgets/GCCUnicodeRelease
DOCSDIR = ../../docs
XMLDIR = ../../xml
HELPDIR = ../../help

PWSAFE-BIN = $(RELDIR)/pwsafe
DOCS = ../../README.txt $(DOCSDIR)/ReleaseNotes.txt \
	$(DOCSDIR)/ChangeLog.txt
MANPAGE = $(DOCSDIR)/pwsafe.1
XMLFILES = $(XMLDIR)/pwsafe.xsd $(XMLDIR)/pwsafe_filter.xsd $(XMLDIR)/pwsafe.xsl
HELPZIP = /tmp/help.zip
HELPFILES = $(HELPZIP)

INFILES = $(PWSAFE-BIN) $(DOCS) $(MANPAGE) $(XMLFILES) $(HELPFILES)

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
	@$(MKDIR) debian/usr/share/doc/passwordsafe/help
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
	@$(CP) $(HELPFILES) debian/usr/share/doc/passwordsafe/help
	@$(MKDIR) debian/DEBIAN
	@$(SED) s/VERSION/$(RELNUM)/ < control > debian/DEBIAN/control
	@$(CP) ../desktop/pwsafe.desktop debian/tmp
	@$(CP) ../graphics/pwsafe.png debian/tmp
	@$(CP) postinst debian/DEBIAN
	@$(CP) prerm debian/DEBIAN
	@$(RM) $(HELPZIP)

$(HELPZIP):
	(cd $(HELPDIR)/default; $(ZIP) -qR $@ pwsafe.hh? *.html *.jpg)

clean:
	@$(RM) debian $(DEBNAME)
