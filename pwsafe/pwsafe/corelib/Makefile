# Baby Makefile for Linux
#

ALLSRC          = PWScore.cpp PWSXMLFilters.cpp \
									Report.cpp UnknownField.cpp \
									PWSfile.cpp SAXFilters.cpp UTF8Conv.cpp \
									ItemData.cpp PWSfileV1V2.cpp SAXHandlers.cpp \
									PWSfileV3.cpp \
									Match.cpp PWSFilters.cpp VerifyFormat.cpp \
									PWSprefs.cpp SMemFile.cpp XMLprefs.cpp \
									PWHistory.cpp \
	 								PWSclipboard.cpp PWSXML.cpp


LIBSRC          = sha1.cpp sha256.cpp BlowFish.cpp hmac.cpp \
		              TwoFish.cpp PWCharPool.cpp PWSdirs.cpp \
	                PWSrand.cpp SysInfo.cpp Util.cpp StringX.cpp \
									ItemField.cpp UUIDGen.cpp Debug.cpp ItemData.cpp
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
	@mkdir -p $(OBJPATH) $(LIBPATH) $(BINPATH)
