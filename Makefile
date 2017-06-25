OS:= $(findstring Linux, $(shell uname -s))
GPG_KEY := 7F2F1BB9

SF_UPLOAD_ROOT := ronys@frs.sourceforge.net:/home/frs/project/p/pa/passwordsafe

ifeq ($(findstring Linux, $(shell uname -s)), Linux)
include Makefile.linux
else ifeq ($(findstring CYGWIN, $(shell uname -s)), CYGWIN)
include Makefile.windows
else ifeq ($(findstring Darwin, $(shell uname -s)), Darwin)
include Makefile.macos
else ifeq ($(findstring FreeBSD, $(shell uname -s)), FreeBSD)
include Makefile.freebsd
else
$(error "Unsupported OS or unable to determine OS")
endif

