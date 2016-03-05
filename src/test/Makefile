# Baby Makefile for Linux
#

CONFIG ?=unicodedebug

BUILD			:= $(CONFIG)

TESTSRC         := coretest.cpp $(wildcard *Test.cpp)

OBJPATH         = ../../obj/$(BUILD)
LIBPATH         = ../../lib/$(BUILD)
BINPATH         = ./bin
INCPATH         = ..

# libgtest: Packages do not include a compiled library.
# Debian's package installs source in /usr/src/gtest
GTEST_DIR     = /usr/src/gtest
GTEST_HEADERS = /usr/include/gtest/*.h \
				/usr/include/gtest/internal/*.h
GTEST_SRCS_   = $(GTEST_DIR)/src/*.cc \
				$(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

#destination related macros
TESTOBJ	 = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(TESTSRC)))
TEST	 = $(BINPATH)/coretest
OBJ	     = $(TESTOBJ) $(LIBOBJ)

CXXFLAGS += -DUNICODE -Wall -I$(INCPATH) -std=c++11
LDFLAGS   = -L$(LIBPATH) -lcore -luuid -los -lcore -lxerces-c -pthread

# rules
.PHONY: all clean test run setup

$(OBJPATH)/%.o : %.c
	$(CC) -g  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

all : setup test gtest.a

gtest-all.o: $(GTEST_SRCS_) $(GTEST_HEADERS)
	$(CXX) -g $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
		$(GTEST_DIR)/src/gtest-all.cc

gtest.a: gtest-all.o
	$(AR) $(ARFLAGS) $@ $^


run test : $(TEST)
	$(TEST)

$(TEST): $(LIB) $(TESTOBJ) gtest.a
	$(CXX) -g $(CXXFLAGS) $(filter %.o,$^) $(LDFLAGS) gtest.a -o $@

clean:
	rm -f *~ $(OBJ) $(TEST) $(DEPENDFILE) gtest-all.o gtest.a

setup:
	@mkdir -p $(OBJPATH) $(LIBPATH) $(BINPATH)
