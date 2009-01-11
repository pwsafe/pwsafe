# Baby Makefile for Linux
#

SRC         = main.cpp

COREPATH        = ../../corelib
LIBPATH         = $(COREPATH)/lib
OBJPATH         = ./obj
BINPATH         = ./bin

#destination related macros
OBJ	 = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(SRC)))
LIB	     = $(LIBPATH)/libcore.a
EXE	    = $(BINPATH)/pwsafe

CXXFLAGS +=  -Wall -I$(COREPATH)
# rules
.PHONY: all clean deepclean setup

$(OBJPATH)/%.o : %.c
	$(CC) -g  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

all : setup $(EXE)


$(LIB): 
	$(MAKE) -C $(COREPATH)

$(EXE): $(LIB) $(OBJ)
	$(CXX) -g $(CXXFLAGS) $(filter %.o,$^)  -o $@ $(LD_FLAGS) \
	-L$(LIBPATH) -lcore -luuid

clean:
	rm -f *~ $(OBJ) $(EXE) $(LIB) $(DEPENDFILE)

deepclean: clean
	$(MAKE) -C $(COREPATH) clean

setup:
	@mkdir -p $(OBJPATH) $(BINPATH)
