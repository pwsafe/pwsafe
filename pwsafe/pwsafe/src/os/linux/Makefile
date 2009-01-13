# Baby Makefile for Linux
#

NAME=os

LIBSRC          = file.cpp mem.cpp \
									sys.cpp env.cpp \
									dir.cpp utf8conv.cpp \
									rand.cpp pws_time.cpp

SRC             = $(LIBSRC)

OBJPATH         = ../../obj/$(NAME)
LIBPATH         = ../../lib

#destination related macros
LIBOBJ	  = $(addprefix $(OBJPATH)/,$(subst .cpp,.o,$(LIBSRC)))
LIB	     = $(LIBPATH)/libos.a
OBJ	     = $(LIBOBJ)

CXXFLAGS +=  -Wall -I../../corelib -I../..

# rules
.PHONY: all clean setup

$(OBJPATH)/%.o : %.c
	$(CC) -g  $(CFLAGS)   -c $< -o $@

$(OBJPATH)/%.o : %.cpp
	$(CXX) -g $(CXXFLAGS) -c $< -o $@

all : setup $(LIB)

$(LIB): $(LIBOBJ)
	ar rvus $@ $(filter %.o,$^)

clean:
	rm -f *~ $(OBJ) $(TEST) $(LIB) $(DEPENDFILE)

setup:
	@mkdir -p $(OBJPATH) $(LIBPATH)
