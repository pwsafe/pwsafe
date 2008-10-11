# Baby Makefile for Linux
#

ALLSRC          = PWScore.cpp PWSXMLFilters.cpp \
									SAXHandlers.cpp \
									SAXFilters.cpp \
									PWSFilters.cpp \
	 								PWSclipboard.cpp PWSXML.cpp

LIBSRC          = sha1.cpp sha256.cpp BlowFish.cpp hmac.cpp \
		              TwoFish.cpp PWCharPool.cpp PWSdirs.cpp \
	                PWSrand.cpp SysInfo.cpp Util.cpp StringX.cpp \
									ItemField.cpp UUIDGen.cpp Debug.cpp PWSprefs.cpp \
									ItemData.cpp PWSfile.cpp PWHistory.cpp \
									PWSfileV1V2.cpp PWSfileV3.cpp \
									UnknownField.cpp  VerifyFormat.cpp Match.cpp \
									UTF8Conv.cpp Report.cpp \
									XMLprefs.cpp tinyxml/tinyxml.cpp \
									tinyxml/tinyxmlerror.cpp tinyxml/tinyxmlparser.cpp \
									os/linux/file.cpp os/linux/mem.cpp \
									os/linux/sys.cpp


TESTSRC         = 
SRC             = $(LIBSRC) $(TESTSRC)

OBJPATH         = ./obj
LIBPATH         = ./lib
BINPATH         = ./bin

#destination related macros
TESTOBJ	 = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(TESTSRC)))
LIBOBJ	  = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(LIBSRC)))
LIB	     = $(LIBPATH)/libcore.a
TEST	    = $(BINPATH)/test
OBJ	     = $(TESTOBJ) $(LIBOBJ)

CXXFLAGS += -DUNICODE -Wall

# rules
.PHONY: all clean test setup

$(OBJPATH)/%.o : %.c
	$(CC) -g  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

all : setup $(LIB)

test : $(TEST)

$(LIB): $(LIBOBJ)
	ar rvus $@ $(filter %.o,$^)

$(TEST): $(LIB) $(TESTOBJ)
	$(CXX) -g $(CXXFLAGS) $(filter %.o,$^)  -o $@ $(LD_FLAGS)

clean:
	rm -f *~ $(OBJ) $(TEST) $(LIB) $(DEPENDFILE)

setup:
	@mkdir -p $(OBJPATH) $(OBJPATH)/tinyxml $(OBJPATH)/os \
            $(OBJPATH)/os/linux $(LIBPATH) $(BINPATH)
