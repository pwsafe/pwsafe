# Make tools used to maintain .po files
# and derive dll's from them
# Really just a wrapper around MSBUILD
#

# MSVSVER is the version of Visual Studio
# Currently supported values are
# * 11 for Visual Studio 2012
# * 12 for Visual Studio 2013
# * 14 for Visual Studio 2015 (current default)
# This can be set by a higher-level Makefile

MSVSVER ?= 14

ifeq ($(MSVSVER),11)
VCBUILD = /cygdrive/c/WINDOWS/Microsoft.NET/Framework/v4.0.30319/MSBuild.exe
else ifeq ($(MSVSVER),12)
VCBUILD = "/cygdrive/c/Program Files (x86)/MSBuild/12.0/Bin/MSBuild.exe"
else ifeq ($(MSVSVER),14)
VCBUILD = "/cygdrive/c/Program Files (x86)/MSBuild/14.0/Bin/MSBuild.exe"
else
$(error "Unsupported MSVSVER")
endif

TOOLS_DIR = ../../../../build/bin
RESTEXT = $(TOOLS_DIR)/restext/release/ResText.exe
RESPWSL = $(TOOLS_DIR)/respwsl/release/ResPWSL.exe
RESTEXT64 = $(TOOLS_DIR)/restext/release64/ResText.exe
RESPWSL64 = $(TOOLS_DIR)/respwsl/release64/ResPWSL.exe

.PHONY: all clean

all : $(RESTEXT) $(RESPWSL) $(RESTEXT64) $(RESPWSL64)

$(RESTEXT) :
	(cd ./ResText; $(VCBUILD) /t:build /p:Configuration=Release /p:Platform=Win32 ResText-$(MSVSVER).vcxproj)

$(RESPWSL) :
	(cd ./ResPWSL; $(VCBUILD) /t:build /p:Configuration=Release /p:Platform=Win32 ResPWSL-$(MSVSVER).vcxproj)

$(RESTEXT64) :
	(cd ./ResText; $(VCBUILD) /t:build /p:Configuration=Release /p:Platform=x64 ResText-$(MSVSVER).vcxproj)

$(RESPWSL64) :
	(cd ./ResPWSL; $(VCBUILD) /t:build /p:Configuration=Release /p:Platform=x64 ResPWSL-$(MSVSVER).vcxproj)

clean :
	(cd ./ResText; $(VCBUILD) /t:clean /p:Configuration=Release /p:Platform=Win32 ResText-$(MSVSVER).vcxproj)
	(cd ./ResText; $(VCBUILD) /t:clean /p:Configuration=Release /p:Platform=x64 ResText-$(MSVSVER).vcxproj)
	(cd ./ResPWSL; $(VCBUILD) /t:clean /p:Configuration=Release /p:Platform=Win32 ResPWSL-$(MSVSVER).vcxproj)
	(cd ./ResPWSL; $(VCBUILD) /t:clean /p:Configuration=Release /p:Platform=x64 ResPWSL-$(MSVSVER).vcxproj)
