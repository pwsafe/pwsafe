# Makefile for Linux build of pwsafe
# core library

#CONFIG=debug
#CONFIG=release
CONFIG?=unicodedebug
#CONFIG=unicoderelease

NAME=core

DEPDIR= ./deps

# Following sets WX_CONFIG to 3.0 version if found, else 2.x:
ifeq ($(PLATFORM),FreeBSD)
WX2=/usr/local/bin/wxgtk2y-2.8-config
WX3=/usr/local/bin/wxgtk2u-3.0-config
else
WX2=/usr/bin/wx-config
WX3=/usr/bin/wx-config-3.0
endif

WX_CONFIG?=$(shell if [ -e $(WX3) ]; then echo $(WX3); else echo $(WX2); fi)

# Following not used in Linux build
NOTSRC          = PWSclipboard.cpp

LIBSRC          = AES.cpp BlowFish.cpp CheckVersion.cpp \
                  Item.cpp ItemData.cpp ItemAtt.cpp ItemField.cpp  \
                  Match.cpp PolicyManager.cpp PWCharPool.cpp CoreImpExp.cpp \
                  PWPolicy.cpp PWHistory.cpp PWSAuxParse.cpp \
                  PWScore.cpp PWSdirs.cpp PWSfile.cpp PWSfileHeader.cpp \
                  PWSfileV1V2.cpp PWSfileV3.cpp PWSfileV4.cpp \
                  PWSFilters.cpp PWSLog.cpp PWSprefs.cpp \
                  Command.cpp PWSrand.cpp Report.cpp \
                  sha1.cpp sha256.cpp core_st.cpp\
								  pbkdf2.cpp KeyWrap.cpp RUEList.cpp \
                  StringX.cpp SysInfo.cpp \
                  TwoFish.cpp UnknownField.cpp  \
                  UTF8Conv.cpp Util.cpp CoreOtherDB.cpp \
                  VerifyFormat.cpp XMLprefs.cpp \
                  ExpiredList.cpp PWStime.cpp\
                  pugixml/pugixml.cpp \
                  XML/XMLFileHandlers.cpp XML/XMLFileValidation.cpp \
                  XML/Xerces/XFileSAX2Handlers.cpp XML/Xerces/XFileValidator.cpp \
                  XML/Xerces/XFileXMLProcessor.cpp XML/Xerces/XFilterSAX2Handlers.cpp \
                  XML/Xerces/XFilterXMLProcessor.cpp XML/Xerces/XSecMemMgr.cpp PWSLog.cpp \
				  RUEList.cpp

SRC             = $(LIBSRC)

OBJPATH         = ../../obj/$(CONFIG)/$(NAME)
LIBPATH         = ../../lib/$(CONFIG)

#destination related macros
LIBOBJ    = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(LIBSRC)))
LIB       = $(LIBPATH)/libcore.a
OBJ       = $(LIBOBJ)

CPPFLAGS := $(CXXFLAGS) -fPIC -Wall -I.. $(CPPFLAGS)

ifeq ($(findstring Darwin, $(shell uname -s)), Darwin)
	    XERCESCPPFLAGS=
		CPPFLAGS += -std=c++11
else
		CPPFLAGS += -DLINUX
	    XERCESCPPFLAGS=-DUSE_XML_LIBRARY=XERCES -DWCHAR_INCOMPATIBLE_XMLCH
endif

ifeq ($(CONFIG),debug)
CPPFLAGS += -O0 -g -ggdb -D_DEBUG -DDEBUG `$(WX_CONFIG) --debug=yes --unicode=no --inplace --cxxflags`
else ifeq ($(CONFIG),release)
CPPFLAGS += -O -DNDEBUG `$(WX_CONFIG) --debug=no --unicode=no --inplace --libs`
else ifeq ($(CONFIG),unicodedebug)
CPPFLAGS += -O0 -g -ggdb -DUNICODE $(XERCESCPPFLAGS) -D_DEBUG -DDEBUG `$(WX_CONFIG) --debug=yes --unicode=yes --inplace --cxxflags`
CFLAGS += -g
else ifeq ($(CONFIG),unicoderelease)
CPPFLAGS += -O -DUNICODE $(XERCESCPPFLAGS) -DNDEBUG `$(WX_CONFIG) --debug=no --unicode=yes --inplace --cxxflags`
endif

# rules
.PHONY: all debug release unicodedebug unicoderelease clean setup

$(OBJPATH)/%.o : %.c
	$(CC) $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

all : setup $(LIB)

debug release unicodedebug unicoderelease:
	$(MAKE) all CONFIG=$@

$(LIB): $(LIBOBJ)
	ar rvus $@ $(filter %.o,$^)

core_st.cpp core_st.h : core.rc2
	../../Misc/rc2cpp.pl $^

StringX.cpp: core_st.h

clean:
	@rm -f *~ $(OBJ) $(TEST) $(LIB)
	@rm -rf $(DEPDIR)
	@rm core_st.cpp core_st.h

setup:
	@mkdir -p $(OBJPATH) $(OBJPATH)/pugixml $(OBJPATH)/XML/Xerces $(LIBPATH) $(DEPDIR)

$(DEPDIR)/%.d: %.cpp
	@set -e; mkdir -p $(DEPDIR) $(DEPDIR)/pugixml; \
	mkdir -p $(DEPDIR)/XML/Xerces; \
	$(RM) $@; \
	$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJECTPATH)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	$(RM) $@.$$$$

-include $(SRC:%.cpp=$(DEPDIR)/%.d)
