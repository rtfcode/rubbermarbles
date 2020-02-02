/*
 * Rubber Marbles - K Sheldrake
 * rb-draw.h
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 * 
 */


#ifndef _RB_DRAW_H
#define _RB_DRAW_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "rb-data.h"
#include "rb-hilbert.h"
#include "rb-shm.h"
#include "rb-mmap.h"
#include "macro.h"


#define COLSCALEREDF 0.7
#define COLSCALEGREENF 0.5
#define COLSCALEBLUEF 0.66
#define CORTESIASCIIR 0x10
#define CORTESIASCIIG 0x72
#define CORTESIASCIIB 0xb8
#define CORTESILOWR 0x4d
#define CORTESILOWG 0xaf
#define CORTESILOWB 0x4a
#define CORTESIHIGHR 0xe4
#define CORTESIHIGHG 0x1a
#define CORTESIHIGHB 0x1c

#define CORTESIHL_BLACKR 0x40
#define CORTESIHL_BLACKG 0x40
#define CORTESIHL_BLACKB 0x00
#define CORTESIHL_WHITER 0xff
#define CORTESIHL_WHITEG 0xff
#define CORTESIHL_WHITEB 0x00
#define CORTESIHL_ASCIIR 0x00
#define CORTESIHL_ASCIIG 0x00
#define CORTESIHL_ASCIIB 0xff
#define CORTESIHL_LOWR 0x00
#define CORTESIHL_LOWG 0xff
#define CORTESIHL_LOWB 0x00
#define CORTESIHL_HIGHR 0xff
#define CORTESIHL_HIGHG 0x00
#define CORTESIHL_HIGHB 0xff

#define COLSCALETHRESHOLD 50
#define COLSCALEHL 100



int colscalecolbyte(int b, guchar * red, guchar * green, guchar * blue);
int greyscalecolbyte(int b, guchar * red, guchar * green, guchar * blue);
int cortesicolbyte(int b, guchar * red, guchar * green, guchar * blue);
int plot_point(struct rgb *pic, int width, int height, int x, int y,
               int colraw);
int calcwindows(unsigned long *wholestart, unsigned long *wholeend,
                unsigned long *zoomstart, unsigned long *zoomend);
void freepic(guchar * pixels, gpointer data);
int draw_img(int pixbufnum);

#endif
