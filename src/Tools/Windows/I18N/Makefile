# Make tools used to maintain .po files
# and derive dll's from them
# Really just a wrapper around VCBUILD
#

VCBUILD = /cygdrive/c/WINDOWS/Microsoft.NET/Framework/v4.0.30319/MSBuild.exe

TOOLS_DIR = ../../../../build/bin
RESTEXT = $(TOOLS_DIR)/restext/release/ResText.exe
RESPWSL = $(TOOLS_DIR)/respwsl/release/ResPWSL.exe

.PHONY: all clean  $(RESTEXT) $(RESPWSL)

all : $(RESTEXT) $(RESPWSL)

$(RESTEXT) :
	(cd ./ResText; $(VCBUILD) /t:rebuild /p:Configuration=Release ResText.vcxproj)

$(RESPWSL) :
	(cd ./ResPWSL; $(VCBUILD) /t:rebuild /p:Configuration=Release ResPWSL.vcxproj)

clean :
	(cd ./ResText; $(VCBUILD) /t:clean ResText.vcxproj)
	(cd ./ResPWSL; $(VCBUILD) /t:clean ResPWSL.vcxproj)

