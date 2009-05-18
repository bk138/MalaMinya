# $Id: Makefile,v 1.5 2006/10/05 03:26:41 whot Exp $

APPNAME=malaminya

CC=g++
OBJ=Util.o Error.o XConn.o Pointer.o Toolbar.o ColorButton.o MalaMinya.o main.o 
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

ColorButton.o: ColorButton.h XConn.h Error.h MalaMinya.h Pointer.h Toolbar.h
Error.o: Error.h
MalaMinya.o: MalaMinya.h Pointer.h Toolbar.h XConn.h ColorButton.h logger.h
MalaMinya.o: Error.h Util.h
Pointer.o: Pointer.h
Toolbar.o: logger.h Toolbar.h Pointer.h XConn.h Error.h MalaMinya.h
Toolbar.o: ColorButton.h Util.h
Util.o: Util.h logger.h
XConn.o: XConn.h Error.h
main.o: Error.h MalaMinya.h Pointer.h Toolbar.h XConn.h ColorButton.h
