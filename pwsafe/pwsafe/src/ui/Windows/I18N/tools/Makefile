# Make tools used to maintain .po files
# and derive dll's from them
# Really just a wrapper around VCBUILD
#

VCBUILD = /cygdrive/c/local/MSVS8/vc/vcpackages/vcbuild.exe

.PHONY: all clean

all : $(RESTEXT) $(RESPWSL)

$(RESTEXT) :
	(cd ./ResText; $(VCBUILD) ResText.sln Release\|Win32)

$(RESPWSL) :
	(cd ./ResPWSL; $(VCBUILD) ResPWSL.sln Release\|Win32)

clean :
	(cd ./ResText; $(VCBUILD) /clean ResText.sln)
	(cd ./ResPWSL; $(VCBUILD) /clean ResPWSL.sln)

