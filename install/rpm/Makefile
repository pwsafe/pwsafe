# Makefile to create source and binary rpm files for wxWidgets version of pwsafe

MAJOR := $(shell grep MAJOR.= ../../version.wx | awk '{print $$3}')
MINOR := $(shell grep MINOR.= ../../version.wx | awk '{print $$3}')
PWD   := $(shell pwd)
VERSION := $(MAJOR).$(MINOR)
RPMBUILDDIR := $(HOME)/rpmbuild

.PHONY: all build tarball

all: build

build: tarball
	sed 's/@VERSION@/$(VERSION)/g' <pwsafe.spec.tmpl >passwordsafe.spec
	rpmbuild -ba -v passwordsafe.spec
	rm passwordsafe.spec
	find $(RPMBUILDDIR) -name passwordsafe*rpm | grep $(VERSION)

tarball:
	mkdir  $(RPMBUILDDIR)/SOURCES || /bin/true
	[ ! -f ../../version.wx ] && echo "current directory $(PWD) should end with install/rpm" || /bin/true
	[ ! -d ../../CMakeModules ] && echo "current directory $(PWD) should end with install/rpm" || /bin/true
	(cd ../..; rm -rf build; tar cfz $(RPMBUILDDIR)/SOURCES/passwordsafe-$(VERSION).tar.gz *)
