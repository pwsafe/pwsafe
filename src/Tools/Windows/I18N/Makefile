# Make tools used to maintain .po files
# and derive dll's from them
# Really just a wrapper around VCBUILD
#

# MSVSVER is the version of Visual Studio
# Currently supported values are
# * 11 for Visual Studio 2012
# * 12 for Visual Studio 2013
# This can be set by a higher-level Makefile

MSVSVER ?= 12

ifeq ($(MSVSVER),11)
VCBUILD = /cygdrive/c/WINDOWS/Microsoft.NET/Framework/v4.0.30319/MSBuild.exe
else ifeq ($(MSVSVER),12)
VCBUILD = "/cygdrive/c/Program Files (x86)/MSBuild/12.0/Bin/MSBuild.exe"
else
$(error "Unsupported MSVSVER")
endif

TOOLS_DIR = ../../../../build/bin
RESTEXT = $(TOOLS_DIR)/restext/release/ResText.exe
RESPWSL = $(TOOLS_DIR)/respwsl/release/ResPWSL.exe

.PHONY: all clean  $(RESTEXT) $(RESPWSL)

all : $(RESTEXT) $(RESPWSL)

$(RESTEXT) :
	(cd ./ResText; $(VCBUILD) /t:rebuild /p:Configuration=Release ResText-$(MSVSVER).vcxproj)

$(RESPWSL) :
	(cd ./ResPWSL; $(VCBUILD) /t:rebuild /p:Configuration=Release ResPWSL-$(MSVSVER).vcxproj)

clean :
	(cd ./ResText; $(VCBUILD) /t:clean ResText-$(MSVSVER).vcxproj)
	(cd ./ResPWSL; $(VCBUILD) /t:clean ResPWSL-$(MSVSVER).vcxproj)
