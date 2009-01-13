# Baby Makefile for Linux
#

NAME=corelib

# Following not used in Linux build
NOTSRC          = 					SAXHandlers.cpp \
									SAXFilters.cpp \
	 								PWSclipboard.cpp PWSXML.cpp

LIBSRC          = sha1.cpp sha256.cpp BlowFish.cpp hmac.cpp \
		              TwoFish.cpp PWCharPool.cpp PWSdirs.cpp \
	                PWSrand.cpp SysInfo.cpp Util.cpp StringX.cpp \
									ItemField.cpp UUIDGen.cpp Debug.cpp PWSprefs.cpp \
									ItemData.cpp PWSfile.cpp PWHistory.cpp \
									PWSfileV1V2.cpp PWSfileV3.cpp \
									PWSFilters.cpp \
									UnknownField.cpp  VerifyFormat.cpp Match.cpp \
									UTF8Conv.cpp Report.cpp PWScore.cpp \
									XMLprefs.cpp tinyxml/tinyxml.cpp \
									tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp

SRC             = $(LIBSRC)

OBJPATH         = ../obj/$(NAME)
LIBPATH         = ../lib

#destination related macros
LIBOBJ	  = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(LIBSRC)))
LIB	     = $(LIBPATH)/libcore.a
OBJ	     = $(LIBOBJ)

CXXFLAGS +=  -Wall -I..
# rules
.PHONY: all clean setup

$(OBJPATH)/%.o : %.c
	$(CC) -g  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

all : setup $(LIB)

$(LIB): $(LIBOBJ)
	ar rvus $@ $(filter %.o,$^)

clean:
	rm -f *~ $(OBJ) $(TEST) $(LIB) $(DEPENDFILE)

setup:
	@mkdir -p $(OBJPATH) $(OBJPATH)/tinyxml $(LIBPATH)
