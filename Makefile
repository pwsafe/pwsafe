OS:= $(findstring Linux, $(shell uname -s))
GPG_KEY := 7F2F1BB9

SF_UPLOAD_ROOT := ronys@frs.sourceforge.net:/home/frs/project/p/pa/passwordsafe

test-gitpod:
	@ printf '\033[31m\033[1m!!! WARNING !!!:\033[0m %s\n' "This is going to create a new commit with a random message in your name, only test this in a new commit and ideally in a profile fork (y/n)"
	@ read -r something && [ "$something" != y ] || exit 1
	@ [ ! -f temporary.Dockerfile ] && printf '\033[31m\033[1mFATAL:\033[0m %s\n' "Expected file 'temporary.Dockerfile' for test-gitpod target is not present, create it?"
	@ # Configure .gitpod.yml to accept temporary image
	@ [ ! -f ".gitpod.yml.original" ] && mv .gitpod.yml .gitpod.yml.original
	@ printf '%s\n' "image:" "  file: temporary.Dockerfile" > .gitpod.yml
	@ gp preview "$$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c10)"
	@ # Disable caching
	@ cat temporary.Dockerfile | sed -E "s#(RUN\s{1}true\s{1}\")(replace)(\"\s{1})#\1$$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c10)\3#gm" > temporary.Dockerfile
	@ printf '%s\n' "" "RUN printf '%s\n' \"SUCCESS!\" && exit 99" >> temporary.Dockerfile
	@ git add * || true
	@ git commit -m "$$(head /dev/urandom | tr -dc A-Za-z0-9 | head -c10)" || true
	@ git push || true
	@ gp preview $$(git remote -v | grep -m1 origin | sed 's/origin\s//g' | sed 's/ (fetch)//g')

build-release:
	@ [ ! -d build ] && mkdir build 
	@ [ ! -d build-dbg ] && mkdir build-dbg 
	@ cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
	@ cmake -S . -B build-dbg -D CMAKE_BUILD_TYPE=Debug

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

