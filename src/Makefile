# $Id: Makefile,v 1.5 2006/10/05 03:26:41 whot Exp $

APPNAME=malaminya

CC=g++
OBJ=Util.o Error.o XConn.o Pointer.o Toolbar.o ColorButton.o MalaMinya.o main.o 
LIBRARIES=x11 xext xi ImageMagick++
FLAGS=-Wall -ggdb
CFLAGS=$(FLAGS) `pkg-config --cflags $(LIBRARIES)` 
LIB=`pkg-config --libs $(LIBRARIES)` 
CTAGS=ctags

all: $(OBJ) ctags
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LIB) -o $(APPNAME) $(OBJ)

.cpp.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

clean:
	rm -f $(APPNAME) *.o

run: all
	./$(APPNAME)

ctags:
	$(CTAGS) *.cpp *.h

dist: CFLAGS:=$(CFLAGS) -DMAKEDIST
dist: clean all