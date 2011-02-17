# Makefile for Linux build of pwsafe
# core library

#CONFIG=debug
#CONFIG=release
CONFIG=unicodedebug
#CONFIG=unicoderelease

NAME=core

DEPDIR= ./deps

# Following not used in Linux build
NOTSRC          = PWSclipboard.cpp \
									tinyxml/tinystr.cpp

LIBSRC          = BlowFish.cpp CheckVersion.cpp hmac.cpp \
                  ItemData.cpp ItemField.cpp  PWPolicy.cpp \
                  Match.cpp PWCharPool.cpp CoreImpExp.cpp \
                  PWHistory.cpp PWSAuxParse.cpp \
                  PWScore.cpp PWSdirs.cpp PWSfile.cpp \
                  PWSfileV1V2.cpp PWSfileV3.cpp \
                  PWSFilters.cpp PWSprefs.cpp \
                  Command.cpp PWSrand.cpp Report.cpp \
                  sha1.cpp sha256.cpp core_st.cpp\
                  StringX.cpp SysInfo.cpp \
                  TwoFish.cpp UnknownField.cpp  \
                  UTF8Conv.cpp Util.cpp UUIDGen.cpp \
                  VerifyFormat.cpp XMLprefs.cpp \
                  ExpiredList.cpp tinyxml/tinyxml.cpp \
                  tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp \
                  XML/XMLFileHandlers.cpp XML/XMLFileValidation.cpp \
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

CXXFLAGS +=  -Wall -I.. -DLINUX

ifeq ($(CONFIG),debug)
CXXFLAGS += -O0 -ggdb -D_DEBUG -DDEBUG
else ifeq ($(CONFIG),release)
CXXFLAGS += -O
else ifeq ($(CONFIG),unicodedebug)
CXXFLAGS += -O0 -ggdb -DUNICODE -DUSE_XML_LIBRARY=XERCES -DWCHAR_INCOMPATIBLE_XMLCH \
			-D_DEBUG -DDEBUG
else ifeq ($(CONFIG),unicoderelease)
CXXFLAGS += -O -DUNICODE -DUSE_XML_LIBRARY=XERCES -DWCHAR_INCOMPATIBLE_XMLCH
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

core_st.cpp core_st.h : core.rc2
	../../Misc/rc2cpp.pl $^

clean:
	@rm -f *~ $(OBJ) $(TEST) $(LIB)
	@rm -rf $(DEPDIR)
	@rm core_st.cpp core_st.h

setup:
	@mkdir -p $(OBJPATH) $(OBJPATH)/tinyxml $(OBJPATH)/XML/Xerces $(LIBPATH) $(DEPDIR)

$(DEPDIR)/%.d: %.cpp
	@set -e; mkdir -p $(DEPDIR) $(DEPDIR)/tinyxml; \
	mkdir -p $(DEPDIR)/XML/Xerces; \
	$(RM) $@; \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJECTPATH)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	$(RM) $@.$$$$

-include $(SRC:%.cpp=$(DEPDIR)/%.d)
