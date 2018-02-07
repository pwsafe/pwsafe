# Makefile to create an rpm file for wxWidgets version of pwsafe

# RELEASENAME should be provided by invoker, or grokked
# from src/ui/wxWidgets/version.in
RELEASENAME := 0.97BETA

MKDIR = /bin/mkdir -p
CAT = /bin/cat
MV = /bin/mv
CP = /bin/cp
RM = /bin/rm -rf
SED = /bin/sed
ZIP = /usr/bin/zip
FIND = /usr/bin/find
CHMOD = /bin/chmod

DOCSDIR := ../../docs
XMLDIR  := ../../xml
HELPDIR := ../../help
MODIR   := ../../src/ui/wxWidgets/I18N/mos
RELEASEDIR := ../../Releases

# I'd prefer the rpm build to be done under this directory,
# but rpmbuild's default is ~/rpmbuild, and life's too short to find out
# where to change this.
RPMBUILDDIR := $(HOME)/rpmbuild
REDHATDIR := $(RPMBUILDDIR)/redhat

DOCS := ../../README.md $(DOCSDIR)/ReleaseNotes.txt \
	$(DOCSDIR)/ChangeLog.txt
MANPAGE := $(DOCSDIR)/pwsafe.1
XMLFILES := $(wildcard $(XMLDIR)/*.*)
HELPFILES := $(HELPDIR)/helpEN.zip
PKG_BASE := passwordsafe
SRC_TGZ := $(PKG_BASE)-$(RELEASENAME)-src.tgz
TARBALL := $(RELEASEDIR)/$(SRC_TGZ)


.PHONY: all build clean tarball

all : build

build:	tarball
	@$(MKDIR) $(RPMBUILDDIR)/SPECS $(RPMBUILDDIR)/SOURCES
	@$(SED) s/@VERSION@/$(RELEASENAME)/g < pwsafe.spec.tmpl > $(RPMBUILDDIR)/SPECS/passwordsafe.spec
	@$(CP) $(TARBALL) $(RPMBUILDDIR)/SOURCES
	@(cd $(RPMBUILDDIR); rpmbuild -ba -v SPECS/passwordsafe.spec)
	@$(CP) $(RPMBUILDDIR)/RPMS/*/*.rpm $(RELEASEDIR)
	@$(CP) $(RPMBUILDDIR)/SRPMS/*.rpm $(RELEASEDIR)

tarball: $(TARBALL)

$(TARBALL):
	(cd ../..;\
	$(MAKE) tar PKG_BASE=$(PKG_BASE))

clean:
	@$(RM)  $(RPMBUILDDIR)
