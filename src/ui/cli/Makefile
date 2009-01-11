# Baby Makefile for Linux
#

SRC         = main.cpp

COREPATH        = ../../corelib
OSPATH          = ../../os
LIBPATH         = ../../lib
OBJPATH         = ../../obj
BINPATH         = ../../bin

#destination related macros
OBJ	 = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(SRC)))
EXE	    = $(BINPATH)/pwsafe

LIBCORE	= $(LIBPATH)/libcore.a
LIBOS  	= $(LIBPATH)/libos.a
LIBS    = $(LIBCORE) $(LIBOS)

CXXFLAGS +=  -Wall -I$(COREPATH) -I../..

# rules
.PHONY: all clean deepclean setup

$(OBJPATH)/%.o : %.c
	$(CC) -g  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

all : setup $(EXE)


$(LIBCORE): 
	$(MAKE) -C $(COREPATH)

$(LIBOS): 
	$(MAKE) -C $(OSPATH)/linux

$(EXE): $(LIBS) $(OBJ)
	$(CXX) -g $(CXXFLAGS) $(filter %.o,$^)  -o $@ $(LD_FLAGS) \
	-L$(LIBPATH) -lcore -los -luuid

clean:
	rm -f *~ $(OBJ) $(EXE) $(LIB) $(DEPENDFILE)

deepclean: clean
	$(MAKE) -C $(COREPATH) clean

setup:
	@mkdir -p $(OBJPATH) $(BINPATH)
