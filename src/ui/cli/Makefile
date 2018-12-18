# Baby Makefile for Linux
#

SRC         = main.cpp search.cpp argutils.cpp searchaction.cpp strutils.cpp \
			  safeutils.cpp diff.cpp impexp.cpp

TESTSRC         = add-entry-test.cpp arg-fields-test.cpp split-test.cpp \
				  safeutils.cpp argutils.cpp searchaction.cpp strutils.cpp \
				  search-test.cpp search.cpp

COREPATH        = ../../core
OSPATH          = ../../os
LIBPATH         = ../../../lib/unicodedebug
OBJPATH         = ../../../obj/unicodedebug/cli
BINPATH         = .

#destination related macros
OBJ	 = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(SRC)))
EXE	    = $(BINPATH)/pwsafe-cli

TESTOBJ	 = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(TESTSRC)))
TESTEXE	= $(BINPATH)/pwsafe-cli-test

LIBCORE	= $(LIBPATH)/libcore.a
LIBOS  	= $(LIBPATH)/libos.a
LIBS    = $(LIBCORE) $(LIBOS)

export CPPFLAGS += -std=c++11
ifeq ($(findstring Darwin, $(shell uname -s)), Darwin)
	XERCESCPPFLAGS=
	PLATFORM=mac
	EXTLIBS=-framework CoreFoundation -framework CoreGraphics
	TEST_EXTLIBS=-framework gtest $(EXTLIBS)
	# typically installed by homebrew etc.
	WX_CONFIG?=/usr/local/bin/wx-config
else
	XERCESCPPFLAGS=-DUSE_XML_LIBRARY=XERCES -DWCHAR_INCOMPATIBLE_XMLCH
	PLATFORM=unix
	GTEST_DIR=/usr/src/gtest
	GTEST_HEADERS=/usr/include/gtest/*.h /usr/include/gtest/internal/*.h
	GTEST_SRCS=$(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
	GTEST_OBJ = $(OBJPATH)/gtest-all.o
	EXTLIBS=-luuid -lxerces-c -lpthread
	TEST_EXTLIBS=$(EXTLIBS)
	WX_CONFIG?=/usr/bin/wx-config
endif

CXXFLAGS += $(CPPFLAGS) -Wall -I$(COREPATH) -I../.. $(XERCESCPPFLAGS) -DUNICODE
WXLIBS:=`$(WX_CONFIG) --debug=no --unicode=yes --libs`

# rules
.PHONY: all clean deepclean setup

$(OBJPATH)/%.o : %.c
	$(CC) -g  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

all : setup $(EXE) $(TESTEXE)


$(LIBCORE): 
	$(MAKE) -C $(COREPATH) WX_CONFIG=$(WX_CONFIG)

$(LIBOS): 
	$(MAKE) -C $(OSPATH)/$(PLATFORM)

$(EXE): $(LIBS) $(OBJ)
	$(CXX) -g $(filter %.o,$^)  -o $@ $(LD_FLAGS) \
	-L$(LIBPATH) -lcore -los -lcore $(EXTLIBS) $(WXLIBS)

clean:
	rm -f *~ $(OBJ) $(EXE) $(LIBS) $(DEPENDFILE)

deepclean: clean
	$(MAKE) -C $(COREPATH) clean

setup:
	@mkdir -p $(OBJPATH) $(BINPATH)

test: $(GTEST_OBJ) $(TESTEXE)

$(GTEST_OBJ): $(GTEST_SRCS_) $(GTEST_HEADERS)
	$(CXX) -g $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c -o $@ \
	$(GTEST_DIR)/src/gtest-all.cc

$(TESTEXE): $(LIBS) $(TESTOBJ)
	$(CXX) -v -g $(CXXFLAGS) $(filter %.o,$^)  -o $@  $(LD_FLAGS) \
	-L$(LIBPATH) -lcore -los -lcore $(TEST_EXTLIBS) $(GTEST_OBJ) $(WXLIBS)
