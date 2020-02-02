/*
 * Rubber Marbles - K Sheldrake
 * rb-render.h
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

#ifndef _RB_REN_DATA_H
#define _RB_REN_DATA_H

#include <stdlib.h>
#include <gtk/gtk.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define UPDATE_CONT 0
#define UPDATE_HOLD 1

#define REN_HILBERT 0
#define REN_SHANNON 1

struct ren_ctx {
    /* window dims */
    int xsize;
    int ysize;
    int xpos;
    int ypos;

    /* widgets */
    GtkWidget *window;
    GtkWidget *vbox;
	GtkWidget *hilbert;
	
	/* display */
	int col_set;
	int disp_hilbert;
	GdkPixbuf *pixbuf;
	
    /* data buffer */
	int fd;
	struct stat filestat;
    uint8_t *buf;
    unsigned long bufsize;
    unsigned long offset;
    unsigned int type;
	unsigned int buftype;
	char shmname[256];
	
    /* mmap section */
//    unsigned long mmap_offset;
//    unsigned long mmap_size;
//    uint8_t *mmap_ptr;

	int reload;
};

#endif
