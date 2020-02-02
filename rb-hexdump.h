/*
 * Rubber Marbles - K Sheldrake
 * rb-hexdump.h
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

#ifndef _HEXDUMP_H
#define _HEXDUMP_H

#include <stdlib.h>
#include <gtk/gtk.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "vis-shm.h"
#include "rb-shm.h"
#include "macro.h"
#include "rb-conf.h"

/* size of a character */
#ifdef __linux__
#define ELEXSIZE 8
#define ELEYSIZE 15
#elif __APPLE__
#define ELEXSIZE 6
#define ELEYSIZE 10
#endif
#define DRAGTIMEOUT 500                /* milliseconds */
#define ADDRSIZE 16
#define SCROLLWIDTH 18
#define BYTECOLWIDTH 3
#define MENUHEIGHT 21
#define ASCIISIZE 258

#define NUDGE_LEFT 0
#define NUDGE_RIGHT 1
#define NUDGE_COLLEFT 2
#define NUDGE_COLRIGHT 3


#define UPDATE_CONT 0
#define UPDATE_HOLD 1

#define HD_LITTLE_ENDIAN 0
#define HD_BIG_ENDIAN 1

struct hd_ctx {
    /* window dims */
    int xsize;
    int ysize;
    int xpos;
    int ypos;

	/* font */
	char gtkfont[PATH_MAX];
	
    /* hextable */
    guint timer_id;
    int elexsize;
    int eleysize;
    int cols;
    int rows;
    int totalrows;
    int tablecols;
    int tablerows;

    /* widgets */
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *hexevent;
    GtkWidget *hextable;
    GtkWidget **table_widgets;
    GtkObject *adj;
    GtkWidget *scroll;

    /* data buffer */
	int fd;
	struct stat filestat;
    uint8_t *buf;
    unsigned long bufsize;
    unsigned long offset;
    unsigned int type;
    unsigned int dsize;
    unsigned int endian;
	char shmname[256];
	
    /* scroll */
    unsigned long scroll_offset;
    int nudge;
    int colnudge;

    /* mmap section */
    unsigned long mmap_offset;
    unsigned long mmap_size;
    uint8_t *mmap_ptr;

	int reload;
};

void *hexdump_init(unsigned int xsize, unsigned int ysize, unsigned int x,
                   unsigned int y);
int hexdump_filedesc(void *rawctx, int fd, struct stat *filestat,
                     unsigned long offset, unsigned long size,
                     unsigned int dsize, int endian);
int hexdump_display(void *rawctx);
int hexdump_redraw(void *rawctx);
int hexdump_free(void *rawctx);

void sig_handler(int signo);
int set_scroll(struct hd_ctx *ctx);
void change_dsize(gpointer data, guint action, GtkWidget * widget);
void change_endian(gpointer data, guint action, GtkWidget * widget);
void set_nudge(gpointer data, guint action, GtkWidget * widget);
void set_update(gpointer data, guint action, GtkWidget * widget);
GtkWidget *hd_menubar_menu(struct hd_ctx *ctx);
void quit_local(gpointer data, guint action, GtkWidget * widget);
gboolean destroy_local(GtkWidget * widget, gpointer data);
gboolean scroll_changed(GtkWidget * widget, gpointer data);
gboolean scroll_event(GtkWidget * widget, GdkEventScroll * event,
                      gpointer data);
gboolean configure_event(GtkWidget * widget, GdkEventConfigure * event,
                         gpointer data);
gboolean resize_event(GtkWidget * widget, GdkEventConfigure * event,
                      gpointer data);
gboolean resize_table(gpointer data);
int create_eventbox(struct hd_ctx *ctx);
int destroy_table(struct hd_ctx *ctx);
int create_and_populate_table(struct hd_ctx *ctx);
int set_dims(struct hd_ctx *ctx);
int set_col_and_font(GtkWidget ** table, int tablecols, int dsize, int i,
                     int j, int col, int font);
int make_label(GtkWidget ** table, GtkWidget * hextable, int cols,
               int dsize, char *msg, int i, int j, int setcol,
               int setfont);
int enlarge_table(struct hd_ctx *ctx, int pcols, int prows);
int copyshm(struct hd_ctx *ctx);
int mapmem(struct hd_ctx *ctx);
int unmapmem(struct hd_ctx *ctx);
int set_col_cols(struct hd_ctx *ctx);
int create_table(struct hd_ctx *ctx);
int makevalue(struct hd_ctx *ctx, char *bytestr, int i, int j);
int makeascii(char *asciistr, uint8_t * buf, int bufsize, int size);
int constrain_nudge(struct hd_ctx *ctx);
int populate(struct hd_ctx *ctx);
int cleanup();

#endif
