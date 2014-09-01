# Baby Makefile for Linux
#

BUILD			:= unicodedebug

TESTSRC         := coretest.cpp $(wildcard *Test.cpp)

OBJPATH         = ../../obj/$(BUILD)
LIBPATH         = ../../lib/$(BUILD)
BINPATH         = ./bin
INCPATH         = ..

# libgtest: Packages do not include a compiled library.
# Debian's package installs source in /usr/src/gtest
# I built the package there using cmake in /usr/src/gtest/build
LIBGTESTPATH = /usr/src/gtest/build

#destination related macros
TESTOBJ	 = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(TESTSRC)))
TEST	 = $(BINPATH)/coretest
OBJ	     = $(TESTOBJ) $(LIBOBJ)

CXXFLAGS += -DUNICODE -Wall -I$(INCPATH) -std=c++11
LDFLAGS   = -L$(LIBPATH) -L$(LIBGTESTPATH) -lcore -luuid -los -lcore -lgtest -pthread

# rules
.PHONY: all clean test run setup

$(OBJPATH)/%.o : %.c
	$(CC) -g  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

all : setup test

run test : $(TEST)
	$(TEST)

$(TEST): $(LIB) $(TESTOBJ)
	$(CXX) -g $(CXXFLAGS) $(filter %.o,$^) $(LDFLAGS) -o $@

clean:
	rm -f *~ $(OBJ) $(TEST) $(DEPENDFILE)

setup:
	@mkdir -p $(OBJPATH) $(LIBPATH) $(BINPATH)
