# Makefile for Linux build of pwsafe
# corelib library

#CONFIG=debug
#CONFIG=release
CONFIG=unicodedebug
#CONFIG=unicoderelease

NAME=corelib

DEPDIR= ./deps

# Following not used in Linux build
NOTSRC          = PWSclipboard.cpp

LIBSRC          = BlowFish.cpp hmac.cpp \
                  ItemData.cpp ItemField.cpp  \
                  Match.cpp PWCharPool.cpp \
                  PWHistory.cpp PWSAuxParse.cpp \
                  PWScore.cpp PWSdirs.cpp PWSfile.cpp \
                  PWSfileV1V2.cpp PWSfileV3.cpp \
                  PWSFilters.cpp PWSprefs.cpp \
                  PWSrand.cpp Report.cpp \
                  sha1.cpp sha256.cpp \
                  StringX.cpp SysInfo.cpp \
                  TwoFish.cpp UnknownField.cpp  \
                  UTF8Conv.cpp Util.cpp UUIDGen.cpp \
                  VerifyFormat.cpp XMLprefs.cpp \
                  tinyxml/tinystr.cpp tinyxml/tinyxml.cpp \
                  tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp \
                  XML/XMLFileHandlers.cpp XML/XMLFileValidation.cpp \
                  XML/Expat/EFileHandlers.cpp XML/Expat/EFileValidator.cpp \
                  XML/Expat/EFileXMLProcessor.cpp XML/Expat/EFilterHandlers.cpp \
                  XML/Expat/EFilterValidator.cpp XML/Expat/EFilterXMLProcessor.cpp \
                  XML/Expat/ESecMemMgr.cpp \
                  XML/Xerces/XFileSAX2Handlers.cpp XML/Xerces/XFileValidator.cpp \
                  XML/Xerces/XFileXMLProcessor.cpp XML/Xerces/XFilterSAX2Handlers.cpp \
                  XML/Xerces/XFilterXMLProcessor.cpp XML/Xerces/XSecMemMgr.cpp                  

SRC             = $(LIBSRC)

OBJPATH         = ../../obj/$(CONFIG)/$(NAME)
LIBPATH         = ../../lib/$(CONFIG)

#destination related macros
LIBOBJ    = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(LIBSRC)))
LIB       = $(LIBPATH)/libcore.a
OBJ       = $(LIBOBJ)

CXXFLAGS +=  -Wall -I..

ifeq ($(CONFIG),debug)
CXXFLAGS += -O0 -ggdb
else ifeq ($(CONFIG),release)
CXXFLAGS += -O
else ifeq ($(CONFIG),unicodedebug)
CXXFLAGS += -O0 -ggdb -DUNICODE
else ifeq ($(CONFIG),release)
CXXFLAGS += -O -DUNICODE
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
  @mkdir -p $(OBJPATH) $(OBJPATH)/tinyxml $(OBJPATH)/XML/Expat $(OBJPATH)/XML/Xerces $(LIBPATH) $(DEPDIR)

$(DEPDIR)/%.d: %.cpp
  @set -e; mkdir -p $(DEPDIR) $(DEPDIR)/tinyxml;$(RM) $@; \
  $(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
  sed 's,\($*\)\.o[ :]*,$(OBJECTPATH)/\1.o $@ : ,g' < $@.$$$$ > $@; \
  $(RM) $@.$$$$

-include $(SRC:%.cpp=$(DEPDIR)/%.d)
