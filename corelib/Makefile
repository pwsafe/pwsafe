# Baby Makefile for Linux
#

LIBSRC          = sha1.cpp sha256.cpp
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

CXXFLAGS += -DUNICODE

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
