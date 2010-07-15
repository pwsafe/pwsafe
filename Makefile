OS:= $(findstring Linux, $(shell uname -s))

ifeq ($(findstring Linux, $(shell uname -s)), Linux)
include Makefile.linux
else ifeq ($(findstring Cygwin, $(shell uname -s)), Cygwin)
include Makefile.windows
else ifeq ($(findstring Darwin, $(shell uname -s)), Darwin)
include Makefile.macos
else
$(error "Unsupported OS or unable to determine OS")
endif

