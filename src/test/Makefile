# Baby Makefile for Linux
#

BUILD			:= unicodedebug

TESTSRC         := coretest.cpp test.cpp

OBJPATH         = ../../obj/$(BUILD)
LIBPATH         = ../../lib/$(BUILD)
BINPATH         = ./bin
INCPATH         = ..

#destination related macros
TESTOBJ	 = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(TESTSRC)))
TEST	 = $(BINPATH)/coretest
OBJ	     = $(TESTOBJ) $(LIBOBJ)

CXXFLAGS += -DUNICODE -Wall -I$(INCPATH)
LDFLAGS   = -L$(LIBPATH) -lcore -los

# rules
.PHONY: all clean test setup

$(OBJPATH)/%.o : %.c
	$(CC) -g  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

all : setup test

test : $(TEST)

$(TEST): $(LIB) $(TESTOBJ)
	$(CXX) -g $(CXXFLAGS) $(filter %.o,$^) $(LDFLAGS) -o $@

clean:
	rm -f *~ $(OBJ) $(TEST) $(DEPENDFILE)

setup:
	@mkdir -p $(OBJPATH) $(LIBPATH) $(BINPATH)
