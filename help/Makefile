BASEDIRNAME=pwsafe
LANGS=RU DE ES FR ZH SK
OS := $(shell uname -s)
WX  = $(shell [[ -d $@_wx ]] && echo _wx)
BWX = $(shell [[ -d $(BASEDIRNAME)$@_wx ]] && echo _wx)

ifeq ($(findstring Linux, $(OS)), Linux)
include Makefile.linux
else ifeq ($(findstring CYGWIN, $(OS)), CYGWIN)
include Makefile.windows
else ifeq ($(findstring Darwin, $(OS)), Darwin)
include Makefile.macos
else ifeq ($(findstring FreeBSD, $(OS)), FreeBSD)
include Makefile.freebsd
else
$(error "Unsupported OS or unable to determine OS: $(OS)")
endif

