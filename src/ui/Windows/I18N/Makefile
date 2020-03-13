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
# https://msdn.microsoft.com/en-us/library/cc233982.aspx
#

#Update the following for each language we support

POS := pwsafe.pot pos/pwsafe_cz.po pos/pwsafe_es.po pos/pwsafe_de.po \
	pos/pwsafe_pl.po pos/pwsafe_zh.po pos/pwsafe_ru.po \
	pos/pwsafe_it.po pos/pwsafe_fr.po pos/pwsafe_dk.po \
	pos/pwsafe_kr.po pos/pwsafe_sv.po pos/pwsafe_nl.po \
	pos/pwsafe_tr.po pos/pwsafe_hu.po pos/pwsafe_sl.po

DLL_LIST = pwsafeCZ.dll pwsafeES.dll pwsafeDE.dll \
	pwsafePL.dll pwsafeZH.dll pwsafeRU.dll pwsafeIT.dll pwsafeFR.dll \
	pwsafeDA.dll pwsafeKO.dll pwsafeNL.dll pwsafeSV.dll pwsafeTR.dll \
	pwsafeHU.dll pwsafeSL.dll

DEST_DIR := ../../../../build/bin/pwsafe/I18N
DEST_DIR_64 := ../../../../build/bin/pwsafe/I18N64
DLLS = $(DLL_LIST:%=$(DEST_DIR)/%)
DLLS_64 = $(DLL_LIST:%=$(DEST_DIR_64)/%)

TOOLS_SRC_DIR := ../../../Tools/Windows/I18N
TOOLS_BIN_DIR := ../../../../build/bin

CHECK_PO := ../../../../Misc/check_po.py

help:
	@echo "Make one of:"
	@echo "update-pos dlls tools clean check help"

# No need to change anything below this line
RESTEXT = $(TOOLS_BIN_DIR)/restext/release/ResText.exe
RESPWSL = $(TOOLS_BIN_DIR)/respwsl/release/ResPWSL.exe
CHMOD = /usr/bin/chmod
RM = /usr/bin/rm
MV = /usr/bin/mv

BASE_DLL = ../../../../build/bin/pwsafe/release/pwsafe_base.dll

.PHONY: update-pos dlls dlls_32 dlls_64 tools clean help check dest_dir

update-pos: tools $(POS)

# All of the .po files depend on the pwsafe_base.dll from the languageDLL project
$(POS): $(BASE_DLL)

# How to make a .po file from pwsafe_base.dll
%.po: $(BASE_DLL)
	$(RESTEXT) extract $^ $@

pwsafe.pot: tools $(BASE_DLL)
	$(RESTEXT) extract -noupdate $(BASE_DLL) $@

dest_dir :
	@mkdir -p $(DEST_DIR)

# Originally, this rule triggered the individual DLL rules above
# However, we now use the VB scripts to make the language DLLs
dlls : tools dest_dir dlls_32 dlls_64

# The issue with the VB scripts is that they have built-in pauses that stop the makefile
dlls_32:
	cscript Create_DLLs.vbs -quiet x86

dlls_64:
	cscript Create_DLLs.vbs -quiet x64

# TODO 64 bit
tools : 
	$(MAKE) -C $(TOOLS_SRC_DIR)

# TODO
check :
	python $(CHECK_PO) $(POS)

# TODO
clean :
	$(MAKE) -C $(TOOLS_SRC_DIR) clean
