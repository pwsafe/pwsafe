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

POS = pwsafe.pot pos/pwsafe_sv.po pos/pwsafe_es.po pos/pwsafe_de.po pos/pwsafe_pl.po 
DLLS = dlls/pwsafeSV_SE.dll dlls/pwsafeES_ES.dll dlls/pwsafeDE_DE.dll dlls/pwsafePL_PL.dll

# Currently we need a rule per language, due to the LCID.
# I'm sure that we can improve on this...

dlls/pwsafeSV_SE.dll : pos/pwsafe_sv.po
	(cd dlls; \
		../$(RESTEXT) apply ../$(BASE_DLL) foo.dll ../$^; \
		../$(RESPWSL) apply foo.dll 0x041d; \
		$(RM) foo.dll)
	$(CHMOD) 755 $@

dlls/pwsafeES_ES.dll : pos/pwsafe_es.po
	(cd dlls; \
		../$(RESTEXT) apply ../$(BASE_DLL) foo.dll ../$^; \
		../$(RESPWSL) apply foo.dll 0x0c0a; \
		$(RM) foo.dll)
	$(CHMOD) 755 $@

dlls/pwsafeDE_DE.dll : pos/pwsafe_de.po
	(cd dlls; \
		../$(RESTEXT) apply ../$(BASE_DLL) foo.dll ../$^; \
		../$(RESPWSL) apply foo.dll 0x0407; \
		$(RM) foo.dll)
	$(CHMOD) 755 $@

dlls/pwsafePL_PL.dll : pos/pwsafe_pl.po
	(cd dlls; \
		../$(RESTEXT) apply ../$(BASE_DLL) foo.dll ../$^; \
		../$(RESPWSL) apply foo.dll 0x0415; \
		$(RM) foo.dll)
	$(CHMOD) 755 $@


# No need to change anything below this line
RESTEXT = ./tools/bin/restext/release/ResText.exe
RESPWSL = ./tools/bin/respwsl/release/ResPWSL.exe
CHMOD = /usr/bin/chmod
RM = /usr/bin/rm

BASE_DLL = ../../../bin/release/pwsafe_base.dll


.PHONY: update-pos dlls

update-pos: $(POS)

$(POS): $(BASE_DLL)

%.po: $(BASE_DLL)
	$(RESTEXT) extract $^ $@

pwsafe.pot: $(BASE_DLL)
	$(RESTEXT) extract -noupdate $^ $@

dlls : $(DLLS)

