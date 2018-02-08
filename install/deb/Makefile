# Makefile to create a Deb file for wxWidgets version of pwsafe

# RELEASENAME should be provided by invoker, or grokked
# from src/ui/wxWidgets/version.in
RELEASENAME := 0.94

DISTRO := $(shell echo $(word 3,$(shell lsb_release -i))|tr [:upper:] [:lower:])

DEBNAME32 = $(RELEASEDIR)passwordsafe-$(DISTRO)-$(RELEASENAME).i686.deb
DEBNAME64 = $(RELEASEDIR)passwordsafe-$(DISTRO)-$(RELEASENAME).amd64.deb
MKDIR = /bin/mkdir -p
CAT = /bin/cat
MV = /bin/mv
CP = /bin/cp
RM = /bin/rm -rf
SED = /bin/sed
ZIP = /usr/bin/zip
FIND = /usr/bin/find
CHMOD = /bin/chmod

RELDIR  := ../../src/ui/wxWidgets/GCCUnicodeRelease
DOCSDIR := ../../docs
XMLDIR  := ../../xml
HELPDIR := ../../help
MODIR   := ../../src/ui/wxWidgets/I18N/mos

PWSAFE-BIN := $(RELDIR)/pwsafe
DOCS := ../../README.md $(DOCSDIR)/ReleaseNotes.txt \
	$(DOCSDIR)/ChangeLog.txt
MANPAGE := $(DOCSDIR)/pwsafe.1
XMLFILES := $(foreach i,\
						  pwsafe.xsd pwsafe_filter.xsd pwsafe.xsl KPV1_to_PWS.xslt,\
				  		$(XMLDIR)/$i)
HELPFILES = $(HELPDIR)/helpEN.zip

INFILES = $(PWSAFE-BIN) $(DOCS) $(MANPAGE) $(XMLFILES) $(HELPFILES)

ARCH := $(shell uname -m)
ifeq ($(ARCH),x86_64)
 DEFAULT := $(DEBNAME64)
else
 DEFAULT := $(DEBNAME32)
endif

.PHONY: all deb debtree clean

all : $(DEFAULT)

ifndef NO_YUBI
SPCL := 
else
SPCL := -noyubi
endif
CONTROL32 := control-$(DISTRO)$(SPCL).i686
CONTROL64 := control-$(DISTRO)$(SPCL).amd64

$(DEBNAME32): $(INFILES)
	@ln -sf $(CONTROL32) control
	$(MAKE) deb DEBNAME=$@
	@$(RM) control

$(DEBNAME64): $(INFILES)
	@ln -sf $(CONTROL64) control
	$(MAKE) deb DEBNAME=$@
	@$(RM) control

deb: debtree
	@fakeroot dpkg-deb --build debian $(DEBNAME)
	@$(RM) debian

debtree:
	@$(MKDIR) debian/usr/bin
	@$(MKDIR) debian/usr/share/passwordsafe/xml
	@$(MKDIR) debian/usr/share/passwordsafe/help
	@$(MKDIR) debian/usr/share/man/man1
	@$(MKDIR) debian/usr/share/locale
	@$(CP) $(PWSAFE-BIN) debian/usr/bin
	@$(CHMOD) 0755 debian/usr/bin/pwsafe
	@strip debian/usr/bin/pwsafe
	@$(CP) $(DOCS) debian/usr/share/doc/passwordsafe
	@$(CP) copyright.debian \
		debian/usr/share/doc/passwordsafe/copyright
	@$(CP) ./changelog.Debian debian/usr/share/doc/passwordsafe
	@(cd debian/usr/share/doc/passwordsafe; \
		$(MV) ChangeLog.txt changelog; gzip -9 changelog; gzip -9 changelog.Debian)
	@$(CP) $(MANPAGE) debian/usr/share/man/man1
	@gzip -9 debian/usr/share/man/man1/pwsafe.1
	@$(CP) $(XMLFILES) debian/usr/share/passwordsafe/xml
	@$(CP) $(HELPFILES) debian/usr/share/passwordsafe/help
	@$(CP) -r $(MODIR)/* debian/usr/share/locale
	@$(MKDIR) debian/DEBIAN
	@$(SED) s/VERSION/$(RELEASENAME)/ < control > debian/DEBIAN/control
	@$(CP) ../desktop/pwsafe.desktop debian/usr/share/applications
	@$(MKDIR) debian/usr/share/icons/hicolor/48x48/apps
	@$(CP) ../graphics/pwsafe.png debian/usr/share/icons/hicolor/48x48/apps
	@$(FIND) debian -type d -exec $(CHMOD) 755 {} \;
	@$(CHMOD) -R g-w debian/usr/share/passwordsafe
	@$(CHMOD) -R g-w debian/usr/share/doc/passwordsafe/*
	@$(CHMOD) g-w debian/usr/share/man/man1/pwsafe.1.gz
	@$(CP) postinst debian/DEBIAN
	@$(CP) prerm debian/DEBIAN
	@$(CHMOD) 0755 debian/DEBIAN/postinst debian/DEBIAN/prerm

$(HELPFILES):
	$(MAKE) -C $(HELPDIR)

clean:
	@$(RM) debian $(DEBNAME)
