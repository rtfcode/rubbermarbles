/*
 * Rubber Marbles - K Sheldrake
 * rb-data.h
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 * 
 * Provides data structures and defines.
 */


#ifndef _RB_DATA_H
#define _RB_DATA_H

#include <gtk/gtk.h>

/* help dialog text */
#define HELPTEXT "\nRubber Marbles is an implementation of the ideas presented by Greg\nConti, Aldo Cortesi and Christopher Domas. It visualises binary files\nwith an extendable set of visualisers.\n\nColours:\nCortesi uses 0x00 black, 0xff white, ascii blue, green low and other red.\nGreyscale and colour scale are obvious.\n\nHilbert:\nHilbertFlipped (default) is Hilbert curve that goes clockwise.\nHilbert is standard anti-clockwise Hilbert curve.\n\nZigzag:\nLinear is simple scan lines.\nZigzag go back and forth.\n\nLeft mouse button sets start of selection; right button sets end.\nUse left button to drag selection windows.\n\nGo:\nMove selection windows to start and end.\n\nVisualise:\nRun a visualiser on the zoomed selection window.\n\nWindow:\nEnable or disable the left hand views.\n\n\nHAVE FUN :)\n\n"

/* total number of concurrent visualisers */
#define MAX_VIS 16
/* total number of visualisers to load */
#define MAXVISLOAD 1000

#define COL_CORTESI 1
#define COL_GREYSCALE 2
#define COL_COLSCALE 3
#define COL_COLSCALE2 4

#define DISP_HILBERTFLIPPED 1
#define DISP_HILBERT 2
#define DISP_LINEAR 3
#define DISP_ZIGZAG 4

#define HEX_PER_ROW 16

#define PIXBUF_WHOLE_HILBERT 0
#define PIXBUF_WHOLE_ZIGZAG 1
#define PIXBUF_WIN 2
#define PIXBUF_ZOOM_ZIGZAG 3
#define PIXBUF_ZOOM_HILBERT 4

#define ZOOM_WHOLE 0
#define ZOOM_ZOOM 1

#define BUTTON_DOWN 0
#define BUTTON_UP 1
#define BUTTON_DRAG 2

//#define MMAP_CHUNK_SIZE 32 * 1024 * 1024

#define VIS_TRIGRAPH 0
#define VIS_DELAYED_TRIGRAPH 1

#define GO_WHOLE_TOP 0
#define GO_WHOLE_BOTTOM 1
#define GO_ZOOM_TOP 2
#define GO_ZOOM_BOTTOM 3

#define RB_LITTLE_ENDIAN 0
#define RB_BIG_ENDIAN 1

/* widgets */
struct widgets {
    GtkWidget *main_window;
    GtkWidget *hilbert_whole;
    GtkWidget *zigzag_whole;
    GtkWidget *zigzag_win;
    GtkWidget *zigzag_zoom;
    GtkWidget *hilbert_zoom;
    GtkWidget *help_dialog;
};

/* an rgb value. Used in an array to make an rgb bitmap */
struct rgb {
    guchar red;
    guchar green;
    guchar blue;
};

/* display settings */
struct displayset {
    int col_set;
    int disp_hilbert;
    int disp_zigzag;
    int drag;
    int dragy;
    int whole_hide;
};

/* mmap context */
/*
struct filemmap {
    unsigned long mmap_size;
    unsigned long mmap_offset;
    unsigned char *mmap_ptr;
	unsigned char *filedata;
	unsigned char *data;
	unsigned long size;
};
*/
/* a pixbuf including size */
struct pixbuf {
    int width;
    int height;
    GdkPixbuf *buf;
};

/* a selection window */
struct window {
    off_t start;
    off_t end;
    float step_hilbert;
    float step_zigzag;
};

/* a saved rbg bitmap */
struct savepic {
    struct rgb *pic;
    int col_set;
	int disp_hilbert;
	int disp_zigzag;
    long start;
    long end;
};

/* a running visualiser */
struct vis {
    int visid;
    pid_t pid;
};

/* a loaded visualiser */
struct visualiser {
    char name[256];
	char exe[PATH_MAX];
};



#endif
