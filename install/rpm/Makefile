# Makefile to create source and binary rpm files for wxWidgets version of pwsafe

MAJOR := $(shell grep MAJOR.= ../../version.wx | awk '{print $$3}')
MINOR := $(shell grep MINOR.= ../../version.wx | awk '{print $$3}')
REV   := $(shell grep   REV.= ../../version.wx | awk '{print $$3}')
VERSION := $(MAJOR).$(MINOR).$(REV)
RPMBUILDDIR := $(HOME)/rpmbuild

.PHONY: all build tarball

all: build

build: tarball
	sed 's/@VERSION@/$(VERSION)/g' <pwsafe.spec.tmpl >passwordsafe.spec
	rpmbuild -ba -v passwordsafe.spec
	rm passwordsafe.spec
	find $(RPMBUILDDIR) -name passwordsafe*rpm | grep $(VERSION)

tarball:
	mkdir -p $(RPMBUILDDIR)/SOURCES
	cd ../.. && \
	git archive --format=tar.gz \
				--prefix=pwsafe-$(VERSION)/ \
				--output=$(RPMBUILDDIR)/SOURCES/$(VERSION).tar.gz \
				HEAD
