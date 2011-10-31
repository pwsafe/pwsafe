# Makefile for supporting internationalization under wxWidgets

DOMAIN := pwsafe
SRC_BASE := ../../..
SRCS := $(wildcard $(SRC_BASE)/ui/wxWidgets/*.cpp) \
	$(wildcard $(SRC_BASE)/core/*.cpp) \
	$(wildcard $(SRC_BASE)/os/linux/*.cpp)

.PHONY: all help pot

help:
	@echo "Currently you can only specify pot as a target"

all: help

pot: $(DOMAIN).pot

$(DOMAIN).pot : $(SRCS)
	@xgettext --default-domain=$(DOMAIN) --language=C++ --keyword=_ --output=$@ $^
