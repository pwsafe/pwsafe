OS:= $(findstring Linux, $(shell uname -o))

ifeq ($(findstring Linux, $(shell uname -o)), Linux)
include Makefile.linux
else ifeq ($(findstring Cygwin, $(shell uname -o)), Cygwin)
include Makefile.windows
else
$(error "Unsupported OS or unable to determine OS")
endif

