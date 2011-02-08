# Makefile to create a Deb file for wxWidgets version of pwsafe

# RELEASENAME should be provided by invoker, or grokked
# from src/ui/wxWidgets/version.in
RELEASENAME := 0.4

DISTRO := $(shell echo $(word 2,$(shell lsb_release -d))|tr [:upper:] [:lower:])

DEBNAME32 = passwordsafe-$(DISTRO)-$(RELEASENAME).i686.deb
DEBNAME64 = passwordsafe-$(DISTRO)-$(RELEASENAME).amd64.deb
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
HELPFILES = $(HELPDIR)/help.zip

INFILES = $(PWSAFE-BIN) $(DOCS) $(MANPAGE) $(XMLFILES) $(HELPFILES)

ARCH := $(shell uname -m)
ifeq ($(ARCH),x86_64)
 DEFAULT := $(DEBNAME64)
else
 DEFAULT := $(DEBNAME32)
endif

.PHONY: all deb debtree clean

all : $(DEFAULT)

$(DEBNAME32): $(INFILES)
	@ln -sf control-$(DISTRO).i686 control
	$(MAKE) deb DEBNAME=$@
	@$(RM) control

$(DEBNAME64): $(INFILES)
	@ln -sf control-$(DISTRO).amd64 control
	$(MAKE) deb DEBNAME=$@
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
	@$(SED) s/VERSION/$(RELEASENAME)/ < control > debian/DEBIAN/control
	@$(CP) ../desktop/pwsafe.desktop debian/tmp
	@$(CP) ../graphics/pwsafe.png debian/tmp
	@$(CP) postinst debian/DEBIAN
	@$(CP) prerm debian/DEBIAN

$(HELPFILES):
	$(MAKE) -C $(HELPDIR)

clean:
	@$(RM) debian $(DEBNAME)
