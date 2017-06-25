OS:= $(findstring Linux, $(shell uname -s))

ifeq ($(findstring Linux, $(shell uname -s)), Linux)
include Makefile.linux
else ifeq ($(findstring CYGWIN, $(shell uname -s)), CYGWIN)
include Makefile.windows
else ifeq ($(findstring Darwin, $(shell uname -s)), Darwin)
#include Makefile.macos
$(error "MacOS unsupported (yet)")
else ifeq ($(findstring FreeBSD, $(shell uname -s)), FreeBSD)
include Makefile.freebsd
else
$(error "Unsupported OS or unable to determine OS")
endif

