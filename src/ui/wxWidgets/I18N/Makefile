# Makefile for supporting internationalization under wxWidgets

DOMAIN := pwsafe
POT := $(DOMAIN).pot
SRC_BASE := ../../..
SRCS := $(wildcard $(SRC_BASE)/ui/wxWidgets/*.cpp) \
	$(wildcard $(SRC_BASE)/core/*.cpp) \
	$(wildcard $(SRC_BASE)/os/linux/*.cpp)

LANGS := de dk es fr it kr nl pl ru sv zh

POS := $(patsubst %,pos/pwsafe_%.po,$(LANGS))

MOS := $(patsubst %,mos/%/LC_MESSAGES/pwsafe.mo,$(LANGS))

.PHONY:  help pot pos mos

help:
	@echo "pot - update po template $(POT) from source code"
	@echo "pos - update translated po files from pot file (to send to xlators)"
	@echo "mos - regenerate compiled catralog files from translations"


pot: $(POT)

pos: $(POS)

mos: $(MOS)

$(POT) : $(SRCS)
	@xgettext --default-domain=$(DOMAIN) --language=C++ --keyword=_ --output=$@ $^

$(POS) : $(POT)
	@msgmerge --update --backup=off $@ $^


mos/%/LC_MESSAGES/pwsafe.mo : pos/pwsafe_%.po
	@mkdir -p $(dir $@)
	@msgfmt --check --output-file=$@ $^
