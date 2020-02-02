# Rubber Marbles - K Sheldrake
# Makefile
#
# This file is part of rubbermarbles.
# 
# Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
# This work is free. You can redistribute it and/or modify it under the
# terms of the Do What The Fuck You Want To Public License, Version 2,
# as published by Sam Hocevar. See the COPYING file or
# http://www.wtfpl.net/for more details.
# 
#

VERSION=0.1
DATE=17/5/2016
WARN=-Wall
CFLAGS=$(WARN)
CFLAGSGTK=`pkg-config --cflags gtk+-2.0`
FT_INC=-I/usr/include/freetype2
GTKLIBS=`pkg-config --libs gtk+-2.0`
MACLIBS=-lm -pthread
MACLIBSGL=-L/System/Library/Frameworks -framework Cocoa -framework OpenGL -lglfw3 -lGLEW -lfreetype
LINUXLIBS=-lm -lrt -pthread
LINUXLIBSGL=-lglfw -lGLEW -lGL -lfreetype
RBVER=-DRBVER=\"$(VERSION)\"
RBDATE=-DRBDATE=\"$(DATE)\"

all:
	# Choose target: linux or mac

mac:
	OSLIBS="$(MACLIBS)" OSLIBSGL="$(MACLIBSGL)" make itall

linux:
	OSLIBS="$(LINUXLIBS)" OSLIBSGL="$(LINUXLIBSGL)" make itall

itall: rubbermarbles.c rubbermarbles.h rb-draw.o rb-gtk.o rb-hilbert.o rb-shm.o shader_utils.o matrixm.o rb-vis.o vis-shm.o trigraph rb-hexdump rb-render
	cc $(CFLAGS) $(CFLAGSGTK) $(RBVER) $(RBDATE) -o rubbermarbles rubbermarbles.c rb-draw.o rb-gtk.o rb-hilbert.o rb-shm.o rb-vis.o $(GTKLIBS) $(OSLIBS)

trigraph: trigraph.c trigraph.h vis-shm.o shader_utils.o matrixm.o tg-text.o rb-conf.o
	cc $(CFLAGS) $(FT_INC) -o trigraph trigraph.c vis-shm.o rb-shm.o shader_utils.o matrixm.o tg-text.o rb-conf.o $(OSLIBS) $(OSLIBSGL)
	rm -f delayedtrigraph
	rm -f bigraph
	rm -f delayedbigraph
	ln -s trigraph delayedtrigraph
	ln -s trigraph bigraph
	ln -s trigraph delayedbigraph

rb-hexdump: rb-hexdump.c rb-hexdump.h vis-shm.o rb-shm.o rb-conf.o
	cc $(CFLAGS) $(CFLAGSGTK) -o rb-hexdump rb-hexdump.c vis-shm.o rb-shm.o rb-conf.o $(GTKLIBS) $(OSLIBS)

rb-render: rb-render.c rb-render.h vis-shm.o rb-shm.o rb-ren-draw.o rb-hilbert.o rb-mmap.o
	cc $(CFLAGS) $(CFLAGSGTK) -o rb-render rb-render.c vis-shm.o rb-shm.o rb-ren-draw.o rb-hilbert.o rb-mmap.o $(GTKLIBS) $(OSLIBS)
	rm -f rb-shannon
	ln -s rb-render rb-shannon

rb-ren-draw.o: rb-ren-draw.c rb-ren-draw.h
	cc -c $(CFLAGS) $(CFLAGSGTK) rb-ren-draw.c

rb-draw.o: rb-draw.c rb-draw.h
	cc -c $(CFLAGS) $(CFLAGSGTK) rb-draw.c

rb-gtk.o: rb-gtk.c rb-gtk.h
	cc -c $(CFLAGS) $(CFLAGSGTK) rb-gtk.c

rb-hilbert.o: rb-hilbert.c rb-hilbert.h
	cc -c $(CFLAGS) rb-hilbert.c

rb-mmap.o: rb-mmap.c rb-mmap.h
	cc -c $(CFLAGS) rb-mmap.c

rb-shm.o: rb-shm.c rb-shm.h
	cc -c $(CFLAGS) rb-shm.c

rb-vis.o: rb-vis.c rb-vis.h
	cc -c $(CFLAGS) $(CFLAGSGTK) rb-vis.c

vis-shm.o: vis-shm.c vis-shm.h
	cc -c $(CFLAGS) vis-shm.c

rb-conf.o: rb-conf.c rb-conf.h
	cc -c $(CFLAGS) rb-conf.c

shader_utils.o: shader_utils.c shader_utils.h
	cc -c $(CFLAGS) shader_utils.c

matrixm.o: matrixm.c matrixm.h
	cc -c $(CFLAGS) matrixm.c

tg-text.o: tg-text.c tg-text.h
	cc -c $(CFLAGS) $(FT_INC) tg-text.c

install: rubbermarbles trigraph delayedtrigraph bigraph delayedbigraph rb-hexdump
	cp -a rubbermarbles trigraph delayedtrigraph bigraph delayedbigraph rb-hexdump rb-render rb-shannon /usr/local/bin
	cp -a etc/rb-vis.conf /etc

clean:
	rm -rf *.o rubbermarbles trigraph delayedtrigraph bigraph delayedbigraph rb-hexdump rb-render rb-shannon


