# Baby Makefile for Linux
#

TESTSRC         = corelibtest.cpp test.cpp
SRC             = $(TESTSRC)

OBJPATH         = ./obj
LIBPATH         = ../../corelib/lib
BINPATH         = ./bin
INCPATH         = ../../corelib

#destination related macros
TESTOBJ	 = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(TESTSRC)))
LIB	     = $(LIBPATH)/libcore.a
TEST	   = $(BINPATH)/corelibtest
OBJ	     = $(TESTOBJ) $(LIBOBJ)

CXXFLAGS += -DUNICODE -Wall -I$(INCPATH)
LDFLAGS = -L$(LIBPATH) -lcore
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
	rm -f *~ $(OBJ) $(TEST) $(LIB) $(DEPENDFILE)

setup:
	@mkdir -p $(OBJPATH) $(LIBPATH) $(BINPATH)
