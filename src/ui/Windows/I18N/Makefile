# This Makefile is used to automate the internationalization support
# of PasswordSafe.
#
# There are two main processes related to i18n:
# 1. Updating the input files (.po & .pot)
# 2. Generating dlls from translated po files
#
# - pwsafe.pot is the raw input file for translation, serves as a
#   basis for all new translations
# - pwsafe*.po are translations to a given language/locale.
# - The above are generates/updated from the $(BASE_DLL).
#
# The list of LCIDs is at
# http://www.microsoft.com/globaldev/reference/lcid-all.mspx 
#

#Update the following for each language we support

POS = pwsafe.pot pos/pwsafe_sv.po pos/pwsafe_es.po pos/pwsafe_de.po \
	pos/pwsafe_pl.po pos/pwsafe_zh.po pos/pwsafe_ru.po

DLL_LIST = pwsafeSV.dll pwsafeES.dll pwsafeDE.dll \
	pwsafePL.dll pwsafeZH.dll pwsafeRU.dll

DEST_DIR = ../../../bin/release
DLLS = $(DLL_LIST:%=$(DEST_DIR)/%)

help:
	@echo "Make one of:"
	@echo "update-pos dlls tools clean help"


# Currently we need a rule per language, due to the LCID.
# I'm sure that we can improve on this...

$(DEST_DIR)/pwsafeSV.dll : pos/pwsafe_sv.po
	(cd $(DEST_DIR); \
 	 $(RESTEXT) apply $(notdir $(BASE_DLL)) foo.dll ../../ui/Windows/I18N/$^; \
	 $(RESPWSL) apply foo.dll 0x041d; \
	 $(RM) foo.dll; $(MV) pwsafeSV_SE.dll pwsafeSV.dll)
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeES.dll : pos/pwsafe_es.po
	(cd $(DEST_DIR); \
		$(RESTEXT) apply $(notdir $(BASE_DLL)) foo.dll ../../ui/Windows/I18N/$^; \
		$(RESPWSL) apply foo.dll 0x0c0a; \
		$(RM) foo.dll; $(MV) pwsafeES_ES.dll pwsafeES.dll)
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeDE.dll : pos/pwsafe_de.po
	(cd $(DEST_DIR); \
		$(RESTEXT) apply $(notdir $(BASE_DLL)) foo.dll ../../ui/Windows/I18N/$^; \
		$(RESPWSL) apply foo.dll 0x0407; \
		$(RM) foo.dll; $(MV) pwsafeDE_DE.dll pwsafeDE.dll)
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafePL.dll : pos/pwsafe_pl.po
	(cd $(DEST_DIR); \
		$(RESTEXT) apply $(notdir $(BASE_DLL)) foo.dll ../../ui/Windows/I18N/$^; \
		$(RESPWSL) apply foo.dll 0x0415; \
		$(RM) foo.dll; $(MV) pwsafePL_PL.dll pwsafePL.dll)
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeZH.dll : pos/pwsafe_zh.po
	(cd $(DEST_DIR); \
		$(RESTEXT) apply $(notdir $(BASE_DLL)) foo.dll ../../ui/Windows/I18N/$^; \
		$(RESPWSL) apply foo.dll 0x0804; \
		$(RM) foo.dll; $(MV) pwsafeZH_CN.dll pwsafeZH.dll)
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeRU.dll : pos/pwsafe_ru.po
	(cd $(DEST_DIR); \
		$(RESTEXT) apply $(notdir $(BASE_DLL)) foo.dll ../../ui/Windows/I18N/$^; \
		$(RESPWSL) apply foo.dll 0x0419; \
		$(RM) foo.dll; $(MV) pwsafeRU_RU.dll pwsafeRU.dll)
	$(CHMOD) 755 $@

# No need to change anything below this line
export RESTEXT = $(CURDIR)/tools/bin/restext/release/ResText.exe
export RESPWSL = $(CURDIR)/tools/bin/respwsl/release/ResPWSL.exe
CHMOD = /usr/bin/chmod
RM = /usr/bin/rm
MV = /usr/bin/mv
VCBUILD = /cygdrive/c/local/MSVS8/vc/vcpackages/vcbuild.exe

BASE_DLL = $(DEST_DIR)/pwsafe_base.dll


.PHONY: update-pos dlls tools clean help

update-pos: tools $(POS)

$(POS): $(BASE_DLL)

%.po: $(BASE_DLL)
	$(RESTEXT) extract $^ $@

pwsafe.pot: tools $(BASE_DLL)
	$(RESTEXT) extract -noupdate $(BASE_DLL) $@

dlls : $(DLLS)

tools : 
	$(MAKE) -C tools

clean :
	$(MAKE) -C tools clean

$(RESTEXT) :
	(cd tools/ResText; $(VCBUILD) ResText.sln Release\|Win32)

$(RESPWSL) :
	(cd tools/ResPWSL; $(VCBUILD) ResPWSL.sln Release\|Win32)

