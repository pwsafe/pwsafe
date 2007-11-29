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

#Update the following for each language we support
POS = pwsafe.pot pos/pwsafesv.po
DLLS = dlls/pwsafeSV_SE.dll

# Currently we need a rule per language, due to the LCID.
# I'm sure that we can improve on this...

dlls/pwsafeSV_SE.dll : pos/pwsafesv.po
	(cd dlls; \
		../$(RESTEXT) apply ../$(BASE_DLL) foo.dll ../$^; \
		../$(RESPWSL) apply foo.dll 0x041d; \
		$(RM) foo.dll)
	$(CHMOD) 755 $@

# No need to change anything below this line
RESTEXT = ./tools/ResText.exe
RESPWSL = ./tools/ResPWSL.exe
CHMOD = /usr/bin/chmod
RM = /usr/bin/rm

BASE_DLL = ../bin/language/pwsafe_base.dll

POS = pwsafe.pot pos/pwsafesv.po

.PHONY: update-pos dlls

update-pos: $(POS)

$(POS): $(BASE_DLL)

%.po: $(BASE_DLL)
	$(RESTEXT) extract $^ $@

pwsafe.pot: $(BASE_DLL)
	$(RESTEXT) extract -noupdate $^ $@

dlls : $(DLLS)

