# Makefile for Linux os-dependent library of
# pwsafe
#

#CONFIG=debug
#CONFIG=release
CONFIG=unicodedebug
#CONFIG=unicoderelease

WX_CONFIG?=/usr/bin/wx-config
CPPINC=`$(WX_CONFIG) --debug=no --unicode=yes --inplace --cxxflags`

DEPDIR= ./deps

NAME=os

ifndef NO_YUBI
YUBI_SRC=PWYubi.cpp
# Use the following if you've installed ykpers-1 manually
# from https://github.com/Yubico/yubikey-personalization.git
# YBPERS_INC?=/usr/local/include/ykpers-1

# If you've installed it from your distro's package repo, you
# should use this instead:
YBPERS_INC?=/usr/include/ykpers-1
endif

LIBSRC          = debug.cpp dir.cpp env.cpp \
                  file.cpp logit.cpp mem.cpp pws_str.cpp \
                  pws_time.cpp rand.cpp run.cpp\
                  utf8conv.cpp KeySend.cpp\
                  sleep.cpp xsendstring.cpp\
                  registry.cpp UUID.cpp\
                  unicode2keysym.cpp $(YUBI_SRC)

SRC             = $(LIBSRC)

OBJPATH         = ../../../obj/$(CONFIG)/$(NAME)
LIBPATH         = ../../../lib/$(CONFIG)

#destination related macros
LIBOBJ    = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(LIBSRC)))
LIB       = $(LIBPATH)/libos.a
OBJ       = $(LIBOBJ)

CPPFLAGS = $(CXXFLAGS) -Wall -I../../core -I../.. $(CPPINC) -I$(YBPERS_INC)

ifeq ($(CONFIG),debug)
CPPFLAGS += -O0 -g -ggdb -DDEBUG
CFLAGS += -g
else ifeq ($(CONFIG),release)
CPPFLAGS += -O -DNDEBUG
else ifeq ($(CONFIG),unicodedebug)
CPPFLAGS += -O0 -g -ggdb -DUNICODE -DDEBUG
CFLAGS += -g
else ifeq ($(CONFIG),unicoderelease)
CPPFLAGS += -O -DUNICODE -DNDEBUG
endif

# rules
.PHONY: all debug release unicodedebug unicoderelease clean setup

$(OBJPATH)/%.o : %.c
	$(CC)  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

all : setup $(LIB)

debug release unicodedebug unicoderelease:
	$(MAKE) all CONFIG=$@

$(LIB): $(LIBOBJ)
	ar rvus $@ $(filter %.o,$^)

clean:
	@rm -f *~ $(OBJ) $(TEST) $(LIB)
	@rm -rf $(DEPDIR)

setup:
	@mkdir -p $(OBJPATH) $(LIBPATH)

$(DEPDIR)/%.d: %.cpp
	@set -e; mkdir -p $(DEPDIR); $(RM) $@; \
	$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJECTPATH)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	$(RM) $@.$$$$

-include $(SRC:%.cpp=$(DEPDIR)/%.d)
