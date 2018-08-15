# Baby Makefile for Linux
#

CONFIG ?=unicodedebug

# Following sets WX_CONFIG to 3.0 version if found, else 2.x:
ifeq ($(PLATFORM),FreeBSD)
WX2=/usr/local/bin/wxgtk2y-2.8-config
WX3=/usr/local/bin/wxgtk2u-3.0-config
else
WX2=/usr/bin/wx-config
WX3=/usr/bin/wx-config-3.0
endif

WX_CONFIG?=$(shell if [ -e $(WX3) ]; then echo $(WX3); else echo $(WX2); fi)



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
GTEST_OBJ = $(OBJPATH)/gtest-all.o

#destination related macros
TESTOBJ	 = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(TESTSRC)))
TEST	   = $(BINPATH)/coretest
OBJS     = $(TESTOBJ) $(GTEST_OBJ)

CXXFLAGS += -DUNICODE -Wall -I$(INCPATH) -std=c++11
LDFLAGS  += -L$(LIBPATH) -lcore -los -luuid -lxerces-c -pthread -lX11 -lXtst

ifeq ($(CONFIG),debug)
LDFLAGS += `$(WX_CONFIG) --debug=yes --unicode=no --libs`
else ifeq ($(CONFIG),release)
LDFLAGS += `$(WX_CONFIG) --debug=no --unicode=no --libs`
else ifeq ($(CONFIG),unicodedebug)
LDFLAGS += `$(WX_CONFIG) --debug=yes --unicode=yes --libs`
else ifeq ($(CONFIG),unicoderelease)
LDFLAGS += `$(WX_CONFIG) --debug=no --unicode=yes --libs`
endif

# rules
.PHONY: all clean test run setup

$(OBJPATH)/%.o : %.c
	$(CC) -g  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

all : setup test

$(GTEST_OBJ): $(GTEST_SRCS_) $(GTEST_HEADERS)
	$(CXX) -g $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c -o $@ \
	$(GTEST_DIR)/src/gtest-all.cc


run test : $(TEST)
	$(TEST)

$(TEST): $(LIB) $(OBJS)
	$(CXX) -g $(CXXFLAGS) $(filter %.o,$^) $(LDFLAGS) -o $@

clean:
	rm -f *~ $(OBJ) $(TEST) $(DEPENDFILE)

setup:
	@mkdir -p $(OBJPATH) $(LIBPATH) $(BINPATH)
