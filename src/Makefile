# $Id: Makefile,v 1.5 2006/10/05 03:26:41 whot Exp $

APPNAME=malaminya

CC=g++
OBJ=Util.o Error.o XConn.o Pointer.o Toolbar.o Backbuffer.o ColorButton.o MalaMinya.o main.o 
LIBRARIES=x11 xext xi ImageMagick++
FLAGS=-Wall -ggdb
CFLAGS=$(FLAGS) `pkg-config --cflags $(LIBRARIES)` 
LIB=`pkg-config --libs $(LIBRARIES)` 
CTAGS=ctags

all: $(APPNAME) ctags

$(APPNAME):	 $(OBJ) 
	$(CC) $(CFLAGS) $(LIB) -o $(APPNAME) $(OBJ)

.cpp.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(APPNAME) *.o

run: all
	./$(APPNAME)

ctags:
	$(CTAGS) *.cpp *.h

dist: CFLAGS:=$(CFLAGS) -DMAKEDIST
dist: clean all

# you can depend on me
depend:
	makedepend -Y -- *.cpp 

# DO NOT DELETE

Backbuffer.o: Backbuffer.h XConn.h
ColorButton.o: ColorButton.h XConn.h Error.h MalaMinya.h Pointer.h Toolbar.h
ColorButton.o: Backbuffer.h
Error.o: Error.h
MalaMinya.o: MalaMinya.h Pointer.h XConn.h Toolbar.h ColorButton.h
MalaMinya.o: Backbuffer.h logger.h Error.h Util.h
Pointer.o: Util.h logger.h MalaMinya.h Pointer.h XConn.h Toolbar.h
Pointer.o: ColorButton.h Backbuffer.h
Toolbar.o: logger.h Toolbar.h Pointer.h XConn.h Error.h MalaMinya.h
Toolbar.o: ColorButton.h Backbuffer.h Util.h
Util.o: Util.h logger.h
XConn.o: XConn.h Error.h
main.o: Error.h MalaMinya.h Pointer.h XConn.h Toolbar.h ColorButton.h
main.o: Backbuffer.h
