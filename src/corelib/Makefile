# Baby Makefile for Linux
#

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
									tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp \
									os/linux/file.cpp os/linux/mem.cpp \
									os/linux/sys.cpp os/linux/env.cpp \
									os/linux/dir.cpp os/linux/utf8conv.cpp \
									os/linux/rand.cpp os/linux/pws_time.cpp

SRC             = $(LIBSRC)

OBJPATH         = ./obj
LIBPATH         = ./lib

#destination related macros
LIBOBJ	  = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(LIBSRC)))
LIB	     = $(LIBPATH)/libcore.a
OBJ	     = $(LIBOBJ)

CXXFLAGS +=  -Wall
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
	@mkdir -p $(OBJPATH) $(OBJPATH)/tinyxml $(OBJPATH)/os \
            $(OBJPATH)/os/linux $(LIBPATH)
