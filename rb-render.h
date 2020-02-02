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

#ifndef _RB_RENDER_H
#define _RB_RENDER_H

#include <stdlib.h>
#include <gtk/gtk.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "vis-shm.h"
#include "rb-shm.h"
#include "rb-ren-data.h"
#include "rb-ren-draw.h"
#include "rb-data.h"
#include "rb-mmap.h"
#include "macro.h"




int cleanup();
void *render_init(unsigned int xsize, unsigned int ysize, unsigned int x,
                   unsigned int y);
int render_filedesc(void *rawctx, int fd, struct stat *filestat,
                     unsigned long offset, unsigned long size,
					 unsigned int buftype);
int render_display(void *rawctx);
int render_redraw(void *rawctx);
int render_free(void *rawctx);

void sig_handler(int signo);
void set_update(gpointer data, guint action, GtkWidget * widget);
GtkWidget *hd_menubar_menu(struct ren_ctx *ctx);
void quit_local(gpointer data, guint action, GtkWidget * widget);
gboolean destroy_local(GtkWidget * widget, gpointer data);
gboolean configure_event(GtkWidget * widget, GdkEventConfigure * event,
                         gpointer data);
int copyshm(struct ren_ctx *ctx);
//int mapmem(struct ren_ctx *ctx);
//int unmapmem(struct ren_ctx *ctx);

#endif
