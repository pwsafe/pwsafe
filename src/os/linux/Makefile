# Makefile for Linux os-dependent library of
# pwsafe
#

CONFIG=debug
#CONFIG=release

DEPDIR= ./deps

NAME=os

LIBSRC          = file.cpp mem.cpp \
									sys.cpp env.cpp \
									dir.cpp utf8conv.cpp \
									rand.cpp pws_time.cpp

SRC             = $(LIBSRC)

OBJPATH         = ../../../obj/$(CONFIG)/$(NAME)
LIBPATH         = ../../../lib/$(CONFIG)

#destination related macros
LIBOBJ	  = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(LIBSRC)))
LIB	     = $(LIBPATH)/libos.a
OBJ	     = $(LIBOBJ)

CXXFLAGS +=  -Wall -I../../corelib -I../..
ifeq ($(CONFIG),debug)
CXXFLAGS += -O0 -ggdb
else ifeq ($(CONFIG),release)
CXXFLAGS += -O
endif

# rules
.PHONY: all debug release clean setup

$(OBJPATH)/%.o : %.c
	$(CC) -g  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

all : setup $(LIB)

debug release:
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
