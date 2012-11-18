# Makefile to create an rpm file for wxWidgets version of pwsafe

# RELEASENAME should be provided by invoker, or grokked
# from src/ui/wxWidgets/version.in
RELEASENAME := 0.8

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
DOCS := ../../README.txt $(DOCSDIR)/ReleaseNotes.txt \
	$(DOCSDIR)/ChangeLog.txt
MANPAGE := $(DOCSDIR)/pwsafe.1
XMLFILES := $(foreach i,\
						  pwsafe.xsd pwsafe_filter.xsd pwsafe.xsl KPV1_to_PWS.xslt,\
				  		$(XMLDIR)/$i)
HELPFILES = $(HELPDIR)/help.zip

INFILES = $(PWSAFE-BIN) $(DOCS) $(MANPAGE) $(XMLFILES) $(HELPFILES)

.PHONY: all rpm 

all : rpm

rpm: $(INFILES)
	@$(MKDIR) redhat/usr/bin
	@$(MKDIR) redhat/usr/share/pwsafe/xml
	@$(MKDIR) redhat/usr/share/doc/passwordsafe/help
	@$(MKDIR) redhat/usr/share/man/man1
	@$(MKDIR) redhat/usr/share/locale
	@$(MKDIR) redhat/tmp
	@$(CP) $(PWSAFE-BIN) redhat/usr/bin
	@strip redhat/usr/bin/pwsafe
	@$(CP) $(DOCS) redhat/usr/share/doc/passwordsafe
	@$(CAT) ../copyright ../../LICENSE > \
		redhat/usr/share/doc/passwordsafe/copyright
	@(cd redhat/usr/share/doc/passwordsafe; \
		$(MV) ChangeLog.txt changelog; gzip -9 changelog)
	@$(CP) $(MANPAGE) redhat/usr/share/man/man1
	@gzip -9 redhat/usr/share/man/man1/pwsafe.1
	@$(CP) $(XMLFILES) redhat/usr/share/pwsafe/xml
	@$(CP) $(HELPFILES) redhat/usr/share/doc/passwordsafe/help
	@$(FIND) redhat -type d -exec $(CHMOD) 755 {} \;
	@$(CP) ../desktop/fedora-pwsafe.desktop redhat/tmp
	@$(CP) ../graphics/pwsafe.png redhat/tmp
	@$(CP) -r $(MODIR)/* redhat/usr/share/locale

$(HELPFILES):
	$(MAKE) -C $(HELPDIR)

clean:
	@$(RM) install/rpm/redhat
	@$(RM) src/ui/wxWidgets/I18N/mos
