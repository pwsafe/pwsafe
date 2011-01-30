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
	pos/pwsafe_pl.po pos/pwsafe_zh.po pos/pwsafe_ru.po \
	pos/pwsafe_it.po pos/pwsafe_fr.po pos/pwsafe_dk.po \
	pos/pwsafe_kr.po pos/pwsafe_nl.po

DLL_LIST = pwsafeSV.dll pwsafeES.dll pwsafeDE.dll \
	pwsafePL.dll pwsafeZH.dll pwsafeRU.dll pwsafeIT.dll pwsafeFR.dll \
	pwsafeDA.dll pwsafeKR.dll pwsafeNL.dll

DEST_DIR = ../../../../build/bin/pwsafe/I18N
DLLS = $(DLL_LIST:%=$(DEST_DIR)/%)

TOOLS_SRC_DIR = ../../../Tools/Windows/I18N
TOOLS_BIN_DIR = ../../../../build/bin

help:
	@echo "Make one of:"
	@echo "update-pos dlls tools clean help"


# Currently we need a rule per language, due to the LCID.
# I'm sure that we can improve on this...

$(DEST_DIR)/pwsafeSV.dll : pos/pwsafe_sv.po
	$(RESTEXT) apply $(BASE_DLL) foo.dll ./$^; \
	$(RESPWSL) apply foo.dll 0x041d; \
	$(RM) foo.dll; $(MV) pwsafeSV_SE.dll $(DEST_DIR)/pwsafeSV.dll
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeES.dll : pos/pwsafe_es.po
	$(RESTEXT) apply $(BASE_DLL) foo.dll ./$^; \
	$(RESPWSL) apply foo.dll 0x0c0a; \
	$(RM) foo.dll; $(MV) pwsafeES_ES.dll $(DEST_DIR)/pwsafeES.dll
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeDE.dll : pos/pwsafe_de.po
	$(RESTEXT) apply $(BASE_DLL) foo.dll ./$^; \
	$(RESPWSL) apply foo.dll 0x0407; \
	$(RM) foo.dll; $(MV) pwsafeDE_DE.dll $(DEST_DIR)/pwsafeDE.dll
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafePL.dll : pos/pwsafe_pl.po
	$(RESTEXT) apply $(BASE_DLL) foo.dll ./$^; \
	$(RESPWSL) apply foo.dll 0x0415; \
	$(RM) foo.dll; $(MV) pwsafePL_PL.dll $(DEST_DIR)/pwsafePL.dll
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeZH.dll : pos/pwsafe_zh.po
	$(RESTEXT) apply $(BASE_DLL) foo.dll ./$^; \
	$(RESPWSL) apply foo.dll 0x0804; \
	$(RM) foo.dll; $(MV) pwsafeZH_CN.dll $(DEST_DIR)/pwsafeZH.dll
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeRU.dll : pos/pwsafe_ru.po
	$(RESTEXT) apply $(BASE_DLL) foo.dll ./$^; \
	$(RESPWSL) apply foo.dll 0x0419; \
	$(RM) foo.dll; $(MV) pwsafeRU_RU.dll $(DEST_DIR)/pwsafeRU.dll
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeIT.dll : pos/pwsafe_it.po
	$(RESTEXT) apply $(BASE_DLL) foo.dll ./$^; \
	$(RESPWSL) apply foo.dll 0x0410; \
	$(RM) foo.dll; $(MV) pwsafeIT_IT.dll $(DEST_DIR)/pwsafeIT.dll
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeFR.dll : pos/pwsafe_fr.po
	$(RESTEXT) apply $(BASE_DLL) foo.dll ./$^; \
	$(RESPWSL) apply foo.dll 0x040c; \
	$(RM) foo.dll; $(MV) pwsafeFR_FR.dll $(DEST_DIR)/pwsafeFR.dll
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeDA.dll : pos/pwsafe_dk.po
	$(RESTEXT) apply $(BASE_DLL) foo.dll ./$^; \
	$(RESPWSL) apply foo.dll 0x0406; \
	$(RM) foo.dll; $(MV) pwsafeDA_DK.dll $(DEST_DIR)/pwsafeDA.dll
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeKR.dll : pos/pwsafe_kr.po
	$(RESTEXT) apply $(BASE_DLL) foo.dll ./$^; \
	$(RESPWSL) apply foo.dll 0x0412; \
	$(RM) foo.dll; $(MV) pwsafeKO_KR.dll $(DEST_DIR)/pwsafeKR.dll
	$(CHMOD) 755 $@

$(DEST_DIR)/pwsafeNL.dll : pos/pwsafe_kr.po
	$(RESTEXT) apply $(BASE_DLL) foo.dll ./$^; \
	$(RESPWSL) apply foo.dll 0x0413; \
	$(RM) foo.dll; $(MV) pwsafeNL_NL.dll $(DEST_DIR)/pwsafeNL.dll
	$(CHMOD) 755 $@

# No need to change anything below this line
RESTEXT = $(TOOLS_BIN_DIR)/restext/release/ResText.exe
RESPWSL = $(TOOLS_BIN_DIR)/respwsl/release/ResPWSL.exe
CHMOD = /usr/bin/chmod
RM = /usr/bin/rm
MV = /usr/bin/mv

BASE_DLL = ../../../../build/bin/pwsafe/release/pwsafe_base.dll

.PHONY: update-pos dlls tools clean help dest_dir

update-pos: tools $(POS)

$(POS): $(BASE_DLL)

%.po: $(BASE_DLL)
	$(RESTEXT) extract $^ $@

pwsafe.pot: tools $(BASE_DLL)
	$(RESTEXT) extract -noupdate $(BASE_DLL) $@

dest_dir :
	@mkdir -p $(DEST_DIR)

dlls : dest_dir $(DLLS)

tools : 
	$(MAKE) -C $(TOOLS_SRC_DIR)

clean :
	$(MAKE) -C $(TOOLS_SRC_DIR) clean
