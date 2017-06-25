# Makefile for Mac os-dependent library of
# pwsafe
#

#CONFIG=debug
#CONFIG=release
CONFIG=unicodedebug
#CONFIG=unicoderelease

DEPDIR= ./deps

NAME=os

LIBSRC          = cleanup.cpp debug.cpp dir.cpp env.cpp \
                  file.cpp logit.cpp mem.cpp pws_str.cpp \
                  pws_time.cpp rand.cpp run.cpp\
                  utf8conv.cpp KeySend.cpp\
                  sleep.cpp macsendstring.cpp\
									registry.cpp UUID.cpp media.cpp

SRC             = $(LIBSRC)

OBJPATH         = ../../../obj/$(CONFIG)/$(NAME)
LIBPATH         = ../../../lib/$(CONFIG)

#destination related macros
LIBOBJ    = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(LIBSRC)))
LIB       = $(LIBPATH)/libos.a
OBJ       = $(LIBOBJ)

CXXFLAGS +=  -Wall -I../../core -I../..
ifeq ($(CONFIG),debug)
CXXFLAGS += -O0 -ggdb
else ifeq ($(CONFIG),release)
CXXFLAGS += -O -DNDEBUG
else ifeq ($(CONFIG),unicodedebug)
CXXFLAGS += -O0 -ggdb -DUNICODE
else ifeq ($(CONFIG),unicoderelease)
CXXFLAGS += -O -DUNICODE -DNDEBUG
endif

# rules
.PHONY: all debug release unicodedebug unicoderelease clean setup

$(OBJPATH)/%.o : %.c
	$(CC) -g  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

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
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJECTPATH)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	$(RM) $@.$$$$

-include $(SRC:%.cpp=$(DEPDIR)/%.d)
