OS:= $(findstring Linux, $(shell uname -s))
GPG_KEY := 5CCF8BB3

UPLOAD_ROOT := ronys@frs.sourceforge.net:/home/frs/project/p/pa/passwordsafe

ifeq ($(findstring Linux, $(shell uname -s)), Linux)
include Makefile.linux
else ifeq ($(findstring CYGWIN, $(shell uname -s)), CYGWIN)
include Makefile.windows
else ifeq ($(findstring Darwin, $(shell uname -s)), Darwin)
include Makefile.macos
else
$(error "Unsupported OS or unable to determine OS")
endif

