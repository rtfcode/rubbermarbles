/*
 * Rubber Marbles - K Sheldrake
 * rb-gtk.h
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

#ifndef _RB_GTK_H
#define _RB_GTK_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "rb-data.h"
#include "rb-hilbert.h"
#include "rb-draw.h"
//#include "trigraph.h"
#include "rb-vis.h"
#include "macro.h"


void child_reap(int signo);
int load_file(char *fname);
int init_arrays();
int make_main_window();
int make_help_dialog();
void change_col(gpointer callback_data, guint callback_action,
                GtkWidget * menu_item);
void change_display(gpointer callback_data, guint callback_action,
                    GtkWidget * menu_item);
int update_children();
int kill_children();
int visualise_end(void *ctx);
void visualise(gpointer callback_data, guint callback_action,
               GtkWidget * menu_item);
void help(GtkWidget * w, gpointer data);
gboolean delete_help(GtkWidget * widget, gpointer data);
void file_open(GtkWidget * w, gpointer data);
GtkWidget *get_menubar_menu(GtkWidget * window);
gboolean configure_event(GtkWidget * widget, GdkEventConfigure * event,
                         gpointer data);
gboolean expose_event(GtkWidget * widget, GdkEventExpose * event,
                      gpointer data);
gboolean button_press_event(GtkWidget * widget, GdkEventButton * event,
                            gpointer data);
gboolean button_release_event(GtkWidget * widget, GdkEventButton * event,
                              gpointer data);
gboolean motion_notify_event(GtkWidget * widget, GdkEventMotion * event,
                             gpointer data);
void quit();
int auto_draw(GtkWidget * widget, int pixbufnum);

#endif
