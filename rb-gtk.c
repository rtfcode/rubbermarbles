/*
 * Rubber Marbles - K Sheldrake
 * rb-gtk.c
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 * 
 * Provides functions to create and manage the Gtk windows.
 */


#include "rb-gtk.h"

/* memory access */
extern struct filemmap mmap_ctx;
extern struct shm_buf *shm_ctx;
extern struct rb_shm *shm;

/* the widgets container */
struct widgets wx;
/* display settings */
struct displayset disp;

/* the five pixbufs - whole hilbert and zigzag, zoom hilbert and zigzag, and markers */
struct pixbuf displays[5];
/* two selection windows */
struct window zoom[2];
/* stored bitmaps */
struct savepic save[5];

/* running visualisers */
struct vis child[MAX_VIS];
/* all visualisers */
extern struct visualiser *visualisers;
extern unsigned int visualiser_count;



/* child_reap is a callback for SIGCHLD.
 * It is called when a forked visualiser ends and it removes the
 * visualiser's entry from the table */
void child_reap(int signo)
{
    int status;
    int i;
    int saved_errno = errno;

    pid_t pid = wait(&status);

    if (pid > 0) {
        /* find pid slot */
        i = 0;
        while ((i < MAX_VIS) && (child[i].pid != pid)) {
            i++;
        }

        /* clear pid slot */
        if (i < MAX_VIS) {
            child[i].visid = -1;
            child[i].pid = 0;
        } else {
            fprintf(stderr, "child_reap: invalid pid\n");
        }
    }

    errno = saved_errno;
}


/*init_arrays sets the arrays to default values */
int init_arrays()
{
    int i;

    disp.col_set = COL_CORTESI;
    disp.disp_hilbert = DISP_HILBERTFLIPPED;
    disp.disp_zigzag = DISP_LINEAR;
    disp.drag = 0;
    disp.dragy = -1;
    disp.whole_hide = 0;

    /* clear the savepic array */
    for (i = 0; i < 5; i++) {
        save[i].pic = NULL;
        save[i].col_set = -1;
        save[i].disp_hilbert = -1;
        save[i].disp_zigzag = -1;
        save[i].start = -2;
        save[i].end = -2;
    }

    /* clear the vis array */
    for (i = 0; i < MAX_VIS; i++) {
        child[i].visid = -1;
        child[i].pid = 0;
    }

    return 0;
}


/* make_main_window builds the main window */
int make_main_window()
{
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *menubar;

    char title_buf[1024];
    char *tmpptr;

    wx.main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (!wx.main_window) {
        FAIL_MSG("RubberMarbles: gtk_window_new() failed\n");
        return 6;
    }

    gtk_widget_set_name(wx.main_window, "Rubber Marbles");

    tmpptr = strrchr(shm->filename, '/');
    if (!tmpptr) {
        tmpptr = shm->filename;
    } else {
        tmpptr++;
    }

    snprintf(title_buf, 1024, "Rubber Marbles - %s\n", tmpptr);
    gtk_window_set_title(GTK_WINDOW(wx.main_window), title_buf);

    if (!g_signal_connect
        (wx.main_window, "destroy", G_CALLBACK(quit), NULL)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 8;
    }


    vbox = gtk_vbox_new(FALSE, 0);
    if (!vbox) {
        FAIL_MSG("RubberMarbles: gtk_vbox_new() failed\n");
        return 7;
    }

    gtk_container_add(GTK_CONTAINER(wx.main_window), vbox);
    gtk_widget_show(vbox);

    /* menu bar */
    menubar = get_menubar_menu(wx.main_window);
    if (!menubar) {
        FAIL_MSG("RubberMarbles: get_menubar_menu() failed\n");
        return 7;
    }

    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, TRUE, 0);
    gtk_widget_show_all(menubar);

    /* main hbox */
    hbox = gtk_hbox_new(FALSE, 1);
    if (!hbox) {
        FAIL_MSG("RubberMarbles: gtk_hbox_new() failed\n");
        return 9;
    }

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    /* whole file hilbert */
    displays[PIXBUF_WHOLE_HILBERT].width = 512;
    displays[PIXBUF_WHOLE_HILBERT].height = 512;
    displays[PIXBUF_WHOLE_HILBERT].buf = NULL;

    wx.hilbert_whole = gtk_drawing_area_new();
    if (!wx.hilbert_whole) {
        FAIL_MSG("RubberMarbles: gtk_drawing_area_new() failed\n");
        return 10;
    }

    gtk_widget_set_size_request(GTK_WIDGET(wx.hilbert_whole), 512, 512);

    gtk_box_pack_start(GTK_BOX(hbox), wx.hilbert_whole, TRUE, TRUE, 0);
    gtk_widget_show(wx.hilbert_whole);

    /* whole file zigzag */
    displays[PIXBUF_WHOLE_ZIGZAG].width = 128;
    displays[PIXBUF_WHOLE_ZIGZAG].height = 512;
    displays[PIXBUF_WHOLE_ZIGZAG].buf = NULL;

    wx.zigzag_whole = gtk_drawing_area_new();
    if (!wx.zigzag_whole) {
        FAIL_MSG("RubberMarbles: gtk_drawing_area_new() failed\n");
        return 11;
    }

    gtk_widget_set_size_request(GTK_WIDGET(wx.zigzag_whole), 128, 512);

    gtk_box_pack_start(GTK_BOX(hbox), wx.zigzag_whole, TRUE, TRUE, 0);
    gtk_widget_show(wx.zigzag_whole);

    /* zigzag window markers */
    displays[PIXBUF_WIN].width = 8;
    displays[PIXBUF_WIN].height = 512;
    displays[PIXBUF_WIN].buf = NULL;

    wx.zigzag_win = gtk_drawing_area_new();
    if (!wx.zigzag_win) {
        FAIL_MSG("RubberMarbles: gtk_drawing_area_new() failed\n");
        return 12;
    }

    gtk_widget_set_size_request(GTK_WIDGET(wx.zigzag_win), 8, 512);

    gtk_box_pack_start(GTK_BOX(hbox), wx.zigzag_win, TRUE, TRUE, 0);
    gtk_widget_show(wx.zigzag_win);

    /* zoom file zigzag */
    displays[PIXBUF_ZOOM_ZIGZAG].width = 128;
    displays[PIXBUF_ZOOM_ZIGZAG].height = 512;
    displays[PIXBUF_ZOOM_ZIGZAG].buf = NULL;

    wx.zigzag_zoom = gtk_drawing_area_new();
    if (!wx.zigzag_zoom) {
        FAIL_MSG("RubberMarbles: gtk_drawing_area_new() failed\n");
        return 13;
    }

    gtk_widget_set_size_request(GTK_WIDGET(wx.zigzag_zoom), 128, 512);

    gtk_box_pack_start(GTK_BOX(hbox), wx.zigzag_zoom, TRUE, TRUE, 0);
    gtk_widget_show(wx.zigzag_zoom);

    /* zoom file hilbert */
    displays[PIXBUF_ZOOM_HILBERT].width = 512;
    displays[PIXBUF_ZOOM_HILBERT].height = 512;
    displays[PIXBUF_ZOOM_HILBERT].buf = NULL;

    wx.hilbert_zoom = gtk_drawing_area_new();
    if (!wx.hilbert_zoom) {
        FAIL_MSG("RubberMarbles: gtk_drawing_area_new() failed\n");
        return 14;
    }

    gtk_widget_set_size_request(GTK_WIDGET(wx.hilbert_zoom), 512, 512);

    gtk_box_pack_start(GTK_BOX(hbox), wx.hilbert_zoom, TRUE, TRUE, 0);
    gtk_widget_show(wx.hilbert_zoom);

    /* Signals used to handle backing pixmap */
    if (!g_signal_connect(wx.hilbert_whole, "expose_event",
                          G_CALLBACK(expose_event),
                          (void *) PIXBUF_WHOLE_HILBERT)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 15;
    }

    if (!g_signal_connect(wx.hilbert_whole, "configure_event",
                          G_CALLBACK(configure_event),
                          (void *) PIXBUF_WHOLE_HILBERT)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 16;
    }

    if (!g_signal_connect(wx.zigzag_whole, "expose_event",
                          G_CALLBACK(expose_event),
                          (void *) PIXBUF_WHOLE_ZIGZAG)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 17;
    }

    if (!g_signal_connect(wx.zigzag_whole, "configure_event",
                          G_CALLBACK(configure_event),
                          (void *) PIXBUF_WHOLE_ZIGZAG)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 18;
    }

    if (!g_signal_connect
        (wx.zigzag_win, "expose_event", G_CALLBACK(expose_event),
         (void *) PIXBUF_WIN)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 19;
    }

    if (!g_signal_connect
        (wx.zigzag_win, "configure_event", G_CALLBACK(configure_event),
         (void *) PIXBUF_WIN)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 20;
    }

    if (!g_signal_connect
        (wx.zigzag_zoom, "expose_event", G_CALLBACK(expose_event),
         (void *) PIXBUF_ZOOM_ZIGZAG)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 21;
    }

    if (!g_signal_connect
        (wx.zigzag_zoom, "configure_event", G_CALLBACK(configure_event),
         (void *) PIXBUF_ZOOM_ZIGZAG)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 22;
    }

    if (!g_signal_connect
        (wx.hilbert_zoom, "expose_event", G_CALLBACK(expose_event),
         (void *) PIXBUF_ZOOM_HILBERT)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 23;
    }

    if (!g_signal_connect
        (wx.hilbert_zoom, "configure_event", G_CALLBACK(configure_event),
         (void *) PIXBUF_ZOOM_HILBERT)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 24;
    }


    /* Event signals */
    if (!g_signal_connect(wx.hilbert_whole, "motion_notify_event",
                          G_CALLBACK(motion_notify_event),
                          (void *) PIXBUF_WHOLE_HILBERT)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 24;
    }

    if (!g_signal_connect(wx.hilbert_whole, "button_press_event",
                          G_CALLBACK(button_press_event),
                          (void *) PIXBUF_WHOLE_HILBERT)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 25;
    }

    if (!g_signal_connect(wx.hilbert_whole, "button_release_event",
                          G_CALLBACK(button_release_event),
                          (void *) PIXBUF_WHOLE_HILBERT)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 26;
    }

    if (!g_signal_connect(wx.zigzag_whole, "motion_notify_event",
                          G_CALLBACK(motion_notify_event),
                          (void *) PIXBUF_WHOLE_ZIGZAG)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 27;
    }

    if (!g_signal_connect(wx.zigzag_whole, "button_press_event",
                          G_CALLBACK(button_press_event),
                          (void *) PIXBUF_WHOLE_ZIGZAG)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 28;
    }

    if (!g_signal_connect(wx.zigzag_whole, "button_release_event",
                          G_CALLBACK(button_release_event),
                          (void *) PIXBUF_WHOLE_ZIGZAG)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 29;
    }

    if (!g_signal_connect(wx.zigzag_zoom, "motion_notify_event",
                          G_CALLBACK(motion_notify_event),
                          (void *) PIXBUF_ZOOM_ZIGZAG)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 30;
    }

    if (!g_signal_connect(wx.zigzag_zoom, "button_press_event",
                          G_CALLBACK(button_press_event),
                          (void *) PIXBUF_ZOOM_ZIGZAG)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 31;
    }

    if (!g_signal_connect(wx.zigzag_zoom, "button_release_event",
                          G_CALLBACK(button_release_event),
                          (void *) PIXBUF_ZOOM_ZIGZAG)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 32;
    }

    if (!g_signal_connect(wx.hilbert_zoom, "motion_notify_event",
                          G_CALLBACK(motion_notify_event),
                          (void *) PIXBUF_ZOOM_HILBERT)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 33;
    }

    if (!g_signal_connect(wx.hilbert_zoom, "button_press_event",
                          G_CALLBACK(button_press_event),
                          (void *) PIXBUF_ZOOM_HILBERT)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 34;
    }

    if (!g_signal_connect(wx.hilbert_zoom, "button_release_event",
                          G_CALLBACK(button_release_event),
                          (void *) PIXBUF_ZOOM_HILBERT)) {
        FAIL_MSG("RubberMarbles: g_signal_connect() failed\n");
        return 35;
    }


    gtk_widget_set_events(wx.hilbert_whole, GDK_EXPOSURE_MASK
                          | GDK_LEAVE_NOTIFY_MASK
                          | GDK_BUTTON_PRESS_MASK
                          | GDK_BUTTON_RELEASE_MASK
                          | GDK_POINTER_MOTION_MASK
                          | GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_set_events(wx.zigzag_whole, GDK_EXPOSURE_MASK
                          | GDK_LEAVE_NOTIFY_MASK
                          | GDK_BUTTON_PRESS_MASK
                          | GDK_BUTTON_RELEASE_MASK
                          | GDK_POINTER_MOTION_MASK
                          | GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_set_events(wx.zigzag_win, GDK_EXPOSURE_MASK);
    gtk_widget_set_events(wx.zigzag_zoom, GDK_EXPOSURE_MASK
                          | GDK_LEAVE_NOTIFY_MASK
                          | GDK_BUTTON_PRESS_MASK
                          | GDK_BUTTON_RELEASE_MASK
                          | GDK_POINTER_MOTION_MASK
                          | GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_set_events(wx.hilbert_zoom, GDK_EXPOSURE_MASK
                          | GDK_LEAVE_NOTIFY_MASK
                          | GDK_BUTTON_PRESS_MASK
                          | GDK_BUTTON_RELEASE_MASK
                          | GDK_POINTER_MOTION_MASK
                          | GDK_POINTER_MOTION_HINT_MASK);


    gtk_widget_show(wx.main_window);

    return 0;
}


/* make_help_dialog builds the help box */
int make_help_dialog()
{
    GtkWidget *vbox;
    GtkWidget *title;
    GtkWidget *msg;
    char *helptxt = HELPTEXT;

    wx.help_dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (!wx.help_dialog) {
        FAIL_MSG("make_help_dialog: gtk_window_new() failed\n");
        return 1;
    }

    gtk_widget_set_name(wx.help_dialog, "Rubber Marbles Help");
    gtk_window_set_title(GTK_WINDOW(wx.help_dialog),
                         "Rubber Marbles Help");

    vbox = gtk_vbox_new(FALSE, 0);
    if (!vbox) {
        FAIL_MSG("make_help_dialog: gtk_vbox_new() failed\n");
        return 2;
    }

    gtk_container_add(GTK_CONTAINER(wx.help_dialog), vbox);
    gtk_widget_show(vbox);

    title = gtk_label_new("Rubber Marbles");
    if (!title) {
        FAIL_MSG("make_help_dialog: gtk_label_new() failed\n");
        return 3;
    }

    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);
    gtk_widget_show(title);

    msg = gtk_label_new(helptxt);
    if (!msg) {
        FAIL_MSG("make_help_dialog: gtk_label_new() failed\n");
        return 4;
    }

    gtk_box_pack_start(GTK_BOX(vbox), msg, FALSE, FALSE, 0);
    gtk_widget_show(msg);

    if (!g_signal_connect(wx.help_dialog, "delete_event",
                          G_CALLBACK(delete_help), NULL)) {
        FAIL_MSG("make_help_dialog: g_signal_connect() failed\n");
        return 5;
    }


    return 0;
}


/* change_col is a menu callback for changing the colour of the hilbert and zigzag */
void
change_col(gpointer callback_data, guint callback_action,
           GtkWidget * menu_item)
{
    if (!menu_item) {
        FAIL_MSG("change_col: invalid params\n");
        return;
    }


    if (GTK_CHECK_MENU_ITEM(menu_item)->active) {
        if ((callback_action >= COL_CORTESI)
            && (callback_action <= COL_COLSCALE)) {
            disp.col_set = callback_action;
            if (auto_draw(wx.hilbert_whole, PIXBUF_WHOLE_HILBERT) != 0) {
                FAIL_MSG("change_col: auto_draw(hilbert_whole) failed\n");
                return;
            }

            if (auto_draw(wx.zigzag_whole, PIXBUF_WHOLE_ZIGZAG) != 0) {
                FAIL_MSG("change_col: auto_draw(zigzag_whole) failed\n");
                return;
            }

            if (auto_draw(wx.zigzag_zoom, PIXBUF_ZOOM_ZIGZAG) != 0) {
                FAIL_MSG("change_col: auto_draw(zigzag_zoom) failed\n");
                return;
            }

            if (auto_draw(wx.hilbert_zoom, PIXBUF_ZOOM_HILBERT) != 0) {
                FAIL_MSG("change_col: auto_draw(hilbert_zoom) failed\n");
                return;
            }

        }
    }
}


/* change_display is a menu callback for changing the way the hilbert and zigzag are displayed */
void
change_display(gpointer callback_data, guint callback_action,
               GtkWidget * menu_item)
{
    if (!menu_item) {
        FAIL_MSG("change_display: invalid params\n");
        return;
    }


    if (GTK_CHECK_MENU_ITEM(menu_item)->active) {
        if ((callback_action >= DISP_HILBERTFLIPPED)
            && (callback_action <= DISP_HILBERT)) {
            disp.disp_hilbert = callback_action;
            if (auto_draw(wx.hilbert_whole, PIXBUF_WHOLE_HILBERT) != 0) {
                FAIL_MSG
                    ("change_display: auto_draw(hilbert_whole) failed\n");
                return;
            }

            if (auto_draw(wx.hilbert_zoom, PIXBUF_ZOOM_HILBERT) != 0) {
                FAIL_MSG
                    ("change_display: auto_draw(hilbert_zoom) failed\n");
                return;
            }

        } else if ((callback_action >= DISP_LINEAR)
                   && (callback_action <= DISP_ZIGZAG)) {
            disp.disp_zigzag = callback_action;
            if (auto_draw(wx.zigzag_whole, PIXBUF_WHOLE_ZIGZAG) != 0) {
                FAIL_MSG
                    ("change_display: auto_draw(zigzag_whole) failed\n");
                return;
            }

            if (auto_draw(wx.zigzag_zoom, PIXBUF_ZOOM_ZIGZAG) != 0) {
                FAIL_MSG
                    ("change_display: auto_draw(zigzag_zoom) failed\n");
                return;
            }

        }
    }
}


/* hide_whole is a menu callback for hiding/showing the whole hilbert and zigzag */
void
hide_whole(gpointer callback_data, guint callback_action,
           GtkWidget * menu_item)
{

    disp.whole_hide = !disp.whole_hide;

    if (disp.whole_hide) {
        gtk_widget_hide(wx.hilbert_whole);
        gtk_widget_hide(wx.zigzag_whole);
        gtk_window_resize(GTK_WINDOW(wx.main_window), 649, 533);
    } else {
        gtk_widget_show(wx.hilbert_whole);
        gtk_widget_show(wx.zigzag_whole);
        gtk_window_resize(GTK_WINDOW(wx.main_window), 1291, 533);
    }
}


/* go_window is a menu callback for moving the selection windows to the start and end */
void
go_window(gpointer callback_data, guint callback_action,
          GtkWidget * menu_item)
{
    long delta;
    unsigned long wholestart, wholeend, zoomstart, zoomend;

    if (calcwindows(&wholestart, &wholeend, &zoomstart, &zoomend) != 0) {
        FAIL_MSG("go_window: calcwindows() failed\n");
        return;
    }



    switch (callback_action) {
    case GO_WHOLE_TOP:
        delta = wholestart;
        zoom[ZOOM_WHOLE].end = wholeend - delta;
        zoom[ZOOM_WHOLE].start = 0;
        if (auto_draw(wx.hilbert_whole, PIXBUF_WHOLE_HILBERT) != 0) {
            FAIL_MSG("change_display: auto_draw(hilbert_whole) failed\n");
            return;
        }

        if (auto_draw(wx.hilbert_zoom, PIXBUF_ZOOM_HILBERT) != 0) {
            FAIL_MSG("change_display: auto_draw(hilbert_zoom) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_whole, PIXBUF_WHOLE_ZIGZAG) != 0) {
            FAIL_MSG("change_display: auto_draw(zigzag_whole) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_zoom, PIXBUF_ZOOM_ZIGZAG) != 0) {
            FAIL_MSG("change_display: auto_draw(zigzag_zoom) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_win, PIXBUF_WIN) != 0) {
            FAIL_MSG("change_display: auto_draw(zigzag_win) failed\n");
            return;
        }

        break;

    case GO_WHOLE_BOTTOM:
        delta = shm->filestat.st_size - wholeend;
        zoom[ZOOM_WHOLE].start = wholestart + delta;
        zoom[ZOOM_WHOLE].end = shm->filestat.st_size;
        if (auto_draw(wx.hilbert_whole, PIXBUF_WHOLE_HILBERT) != 0) {
            FAIL_MSG("change_display: auto_draw(hilbert_whole) failed\n");
            return;
        }

        if (auto_draw(wx.hilbert_zoom, PIXBUF_ZOOM_HILBERT) != 0) {
            FAIL_MSG("change_display: auto_draw(hilbert_zoom) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_whole, PIXBUF_WHOLE_ZIGZAG) != 0) {
            FAIL_MSG("change_display: auto_draw(zigzag_whole) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_zoom, PIXBUF_ZOOM_ZIGZAG) != 0) {
            FAIL_MSG("change_display: auto_draw(zigzag_zoom) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_win, PIXBUF_WIN) != 0) {
            FAIL_MSG("change_display: auto_draw(zigzag_win) failed\n");
            return;
        }

        break;

    case GO_ZOOM_TOP:
        delta = zoomstart - wholestart;
        zoom[ZOOM_ZOOM].end = zoomend - delta;
        zoom[ZOOM_ZOOM].start = wholestart;
        if (auto_draw(wx.hilbert_zoom, PIXBUF_ZOOM_HILBERT) != 0) {
            FAIL_MSG("change_display: auto_draw(hilbert_zoom) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_zoom, PIXBUF_ZOOM_ZIGZAG) != 0) {
            FAIL_MSG("change_display: auto_draw(zigzag_zoom) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_win, PIXBUF_WIN) != 0) {
            FAIL_MSG("change_display: auto_draw(zigzag_win) failed\n");
            return;
        }

        if (update_children() != 0) {
            FAIL_MSG("change_display: update_children() failed\n");
            return;
        }

        break;

    case GO_ZOOM_BOTTOM:
        delta = wholeend - zoomend;
        zoom[ZOOM_ZOOM].start = zoomstart + delta;
        zoom[ZOOM_ZOOM].end = wholeend;
        if (auto_draw(wx.hilbert_zoom, PIXBUF_ZOOM_HILBERT) != 0) {
            FAIL_MSG("change_display: auto_draw(hilbert_zoom) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_zoom, PIXBUF_ZOOM_ZIGZAG) != 0) {
            FAIL_MSG("change_display: auto_draw(zigzag_zoom) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_win, PIXBUF_WIN) != 0) {
            FAIL_MSG("change_display: auto_draw(zigzag_win) failed\n");
            return;
        }

        if (update_children() != 0) {
            FAIL_MSG("change_display: update_children() failed\n");
            return;
        }

        break;

    default:
        fprintf(stderr, "go_window: invalid action\n");
    }
}


/* update_children prods all the running visualisers to indicate that the data in the shared memory has
 * changed.  It prods forked visualisers by sending a SIGUSR1 to them and it prods non-forked ones by
 * calling VIS_redraw().
 */
int update_children()
{
    int i;
    unsigned long wholestart, wholeend, start, end;
    unsigned long size;

    /* find the zoomed window start and size */
    if (calcwindows(&wholestart, &wholeend, &start, &end) != 0) {
        FAIL_MSG("update_children: calcwindows() failed\n");
        return 1;
    }

    size = end - start;

    /* set the values in the shared memory */
    if (sem_wait(shm_ctx->sem) != 0) {
        FAIL_ERR("update_children: sem_wait() failed\n");
        return 2;
    }

    shm->offset = start;
    shm->bufsize = size;
    if (sem_post(shm_ctx->sem) != 0) {
        FAIL_ERR("update_children: sem_post() failed\n");
        return 3;
    }


    /* send SIGUSR1 to all children OR call VIS_redraw() */
    for (i = 0; i < MAX_VIS; i++) {
        if (child[i].pid) {
            if (kill(child[i].pid, SIGUSR1) != 0) {
                FAIL_ERR("update_children: kill failed\n");
                return 4;
            }

        }
    }

    return 0;
}


/* kill_children kills the running forked visualisers */
int kill_children()
{
    int i;

    /* send SIGHUP to all children */
    for (i = 0; i < MAX_VIS; i++) {
        if (child[i].pid) {
            if (kill(child[i].pid, SIGHUP) != 0) {
                FAIL_ERR("kill_children: kill failed\n");
                return 0;
            }

        }
        /* don't worry about removing the pid from the child[] array as
         * the SIGCHLD handler will do that for us. */
    }

    return 0;
}


/* visualise is a menu callback that launches a visualiser */
void
visualise(gpointer callback_data, guint callback_action,
          GtkWidget * menu_item)
{
    int newvis;

    if (callback_action < visualiser_count) {
        /* find empty visualiser slot */
        newvis = 0;
        while ((newvis < MAX_VIS) && (child[newvis].pid)) {
            newvis++;
        }

        if (newvis >= MAX_VIS) {
            FAIL_MSG("Too many visualisers already\n");
            return;
        }


        /* set the visualiser id */
        child[newvis].visid = callback_action;

        /* fork it */
        child[newvis].pid = fork();
        if (child[newvis].pid < 0) {
            perror("visualise: fork() failed\n");
            return;
        } else if (child[newvis].pid == 0) {
            /* child */

            if (execlp
                (visualisers[callback_action].exe,
                 visualisers[callback_action].exe, shm_ctx->buf_name,
                 NULL) != 0) {
                FAIL_ERR("visualise: execlp() failed\n");
                return;
            }

            exit(1);

        } else {
            /* parent */
        }
    }
}


/* help is a menu callback for launching the help dialog */
void help(GtkWidget * w, gpointer data)
{
    gtk_widget_show(wx.help_dialog);
}


/* delete_help is a delete event callback that hides rather than destroys the window */
gboolean delete_help(GtkWidget * widget, gpointer data)
{
    gtk_widget_hide(wx.help_dialog);
    return TRUE;
}


/* file_open is a menu callback for launching the file open dialog */
void file_open(GtkWidget * w, gpointer data)
{

    GtkWidget *dialog;
    char *tmpfilename = NULL;
    char title_buf[1024];
    char *tmpptr;

    dialog = gtk_file_chooser_dialog_new("Open File",
                                         GTK_WINDOW(wx.main_window),
                                         GTK_FILE_CHOOSER_ACTION_OPEN,
                                         GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_CANCEL,
                                         GTK_STOCK_OPEN,
                                         GTK_RESPONSE_ACCEPT, NULL);
    if (!dialog) {
        FAIL_MSG("file_open: gtk_file_chooser_dialog_new() failed\n");
        return;
    }


    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {

        tmpfilename =
            gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        if (load_file(tmpfilename)) {
            fprintf(stderr, "file open: cannot open file\n");
            g_free(tmpfilename);
            gtk_widget_destroy(dialog);
            return;
        }

        tmpptr = strrchr(tmpfilename, '/');
        if (!tmpptr) {
            tmpptr = tmpfilename;
        } else {
            tmpptr++;
        }

        snprintf(title_buf, 1024, "Rubber Marbles - %s\n", tmpptr);
        gtk_window_set_title(GTK_WINDOW(wx.main_window), title_buf);

        g_free(tmpfilename);
        gtk_widget_destroy(dialog);

        if (auto_draw(wx.hilbert_whole, PIXBUF_WHOLE_HILBERT) != 0) {
            FAIL_MSG("file_open: auto_draw(hilbert_whole) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_whole, PIXBUF_WHOLE_ZIGZAG) != 0) {
            FAIL_MSG("file_open: auto_draw(zigzag_whole) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_win, PIXBUF_WIN) != 0) {
            FAIL_MSG("file_open: auto_draw(zigzag_win) failed\n");
            return;
        }

        if (auto_draw(wx.zigzag_zoom, PIXBUF_ZOOM_ZIGZAG) != 0) {
            FAIL_MSG("file_open: auto_draw(zigzag_zoom) failed\n");
            return;
        }

        if (auto_draw(wx.hilbert_zoom, PIXBUF_ZOOM_HILBERT) != 0) {
            FAIL_MSG("file_open: auto_draw(hilbert_zoom) failed\n");
            return;
        }


    } else {
        gtk_widget_destroy(dialog);
    }

}


/* Our menu, an array of GtkItemFactoryEntry structures that defines each menu item */
GtkItemFactoryEntry menu_items[] = {
    {"/_File", NULL, NULL, 0, "<Branch>"}
    ,
    {"/File/_Open", "<control>O", file_open, 0, "<StockItem>",
     GTK_STOCK_OPEN}
    ,
    {"/File/sep1", NULL, NULL, 0, "<Separator>"}
    ,
    {"/File/_Quit", "<CTRL>Q", quit, 0, "<StockItem>",
     GTK_STOCK_QUIT}
    ,
    {"/_Colours", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Colours/Cortesi", NULL, change_col, COL_CORTESI, "<RadioItem>"}
    ,
    {"/Colours/GreyScale", NULL, change_col, COL_GREYSCALE,
     "/Colours/Cortesi"}
    ,
    {"/Colours/ColScale", NULL, change_col, COL_COLSCALE,
     "/Colours/Cortesi"}
    ,
    {"/_Hilbert", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Hilbert/HilbertFlipped", NULL, change_display, DISP_HILBERTFLIPPED,
     "<RadioItem>"}
    ,
    {"/Hilbert/Hilbert", NULL, change_display, DISP_HILBERT,
     "/Hilbert/HilbertFlipped"}
    ,
    {"/_Zigzag", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Zigzag/Linear", NULL, change_display, DISP_LINEAR, "<RadioItem>"}
    ,
    {"/Zigzag/Zigzag", NULL, change_display, DISP_ZIGZAG, "/Zigzag/Linear"}
    ,
    {"/_Go", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Go/Whole top", "t", go_window, GO_WHOLE_TOP, "<Item>"}
    ,
    {"/Go/Whole bottom", "b", go_window, GO_WHOLE_BOTTOM, "<Item>"}
    ,
    {"/Go/Zoom top", "<shift>t", go_window, GO_ZOOM_TOP, "<Item>"}
    ,
    {"/Go/Zoom bottom", "<shift>b", go_window, GO_ZOOM_BOTTOM, "<Item>"}
    ,
    {"/_Visualise", NULL, NULL, 0, "<Branch>"}
    ,
};

/*
  { "/Visualise/Trigraph",  NULL,         visualise, VIS_TRIGRAPH, "<Item>" },
  { "/Visualise/Delayed Trigraph",  NULL,         visualise, VIS_DELAYED_TRIGRAPH, "<Item>" },
*/

GtkItemFactoryEntry help_menu_items[] = {
    {"/_Window", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Window/Whole Display on-off", NULL, hide_whole, 0, "<Item>"}
    ,
    {"/_Help", NULL, NULL, 0, "<LastBranch>"}
    ,
    {"/_Help/About", NULL, help, 0, "<Item>"}
    ,
};


/* get_menubar_menu returns a menubar widget made from the above menus combined with the visualisers */
GtkWidget *get_menubar_menu(GtkWidget * window)
{
    GtkItemFactory *item_factory;
    GtkAccelGroup *accel_group;
    gint nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);
    gint nhelp_menu_items =
        sizeof(help_menu_items) / sizeof(help_menu_items[0]);
    GtkItemFactoryEntry *dyn_menu_items = NULL;
    GtkItemFactoryEntry *ptr = NULL;
    int i;
    gint ndyn_menu_items =
        nmenu_items + visualiser_count + nhelp_menu_items;

    if (!window) {
        FAIL_MSG("get_menubar_menu: invalid params\n");
        return NULL;
    }


    /* build menu with dynamic visualisers */
    dyn_menu_items =
        (GtkItemFactoryEntry *) calloc(ndyn_menu_items,
                                       sizeof(menu_items[0]));
    if (!dyn_menu_items) {
        FAIL_ERR("get_menubar_menu: calloc failed\n");
        return NULL;
    }


    memcpy(dyn_menu_items, menu_items, sizeof(menu_items));
    ptr = dyn_menu_items + nmenu_items;

    for (i = 0; i < visualiser_count; i++) {
        ptr->path = (gchar *) malloc(300);
        if (!(ptr->path)) {
            FAIL_ERR("get_menubar_menu: malloc failed\n");
            return NULL;
        }


        snprintf(ptr->path, 300, "/Visualise/%s", visualisers[i].name);
        ptr->accelerator = NULL;
        ptr->callback = visualise;
        ptr->callback_action = i;
        ptr->item_type = (gchar *) malloc(7);
        if (!(ptr->item_type)) {
            FAIL_ERR("get_menubar_menu: malloc failed\n");
            return NULL;
        }

        strncpy(ptr->item_type, "<Item>", 7);

        ptr++;
    }

    memcpy(ptr, help_menu_items, sizeof(help_menu_items));

    /* Make an accelerator group (shortcut keys) */
    accel_group = gtk_accel_group_new();
    if (!accel_group) {
        FAIL_MSG("get_menubar_menu: gtk_accel_group_new() failed\n");
        return NULL;
    }


    /* Make an ItemFactory (that makes a menubar) */
    item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",
                                        accel_group);
    if (!item_factory) {
        FAIL_MSG("get_menubar_meun: gtk_item_factory_new() failed\n");
        return NULL;
    }


    /* This function generates the menu items. Pass the item factory,
       the number of items in the array, the array itself, and any
       callback data for the the menu items. */
    gtk_item_factory_create_items(item_factory, ndyn_menu_items,
                                  dyn_menu_items, NULL);

    /* Attach the new accelerator group to the window. */
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

    /* Finally, return the actual menu bar created by the item factory. */
    return gtk_item_factory_get_widget(item_factory, "<main>");
}


/* configure_event creates a new backing pixmap of the appropriate size */
gboolean
configure_event(GtkWidget * widget, GdkEventConfigure * event,
                gpointer data)
{

    int width;
    int twidth;
    int n;
    int pixbufnum = (int) (long) data;

    if (!widget) {
        FAIL_MSG("configure_event: invalid params\n");
        return TRUE;
    }


    width = widget->allocation.width;
    if (widget->allocation.height < width) {
        width = widget->allocation.height;
    }

    if ((pixbufnum == PIXBUF_WHOLE_HILBERT)
        || (pixbufnum == PIXBUF_ZOOM_HILBERT)) {

        twidth = width;
        n = 0;
        while (twidth) {
            twidth = twidth >> 1;
            n++;
        }
        if (n > 0)
            n--;

        width = pow(2, n);

        if (displays[pixbufnum].width != width) {
            displays[pixbufnum].width = width;
            displays[pixbufnum].height = width;
        }
    }

    if (draw_img(pixbufnum) != 0) {
        FAIL_MSG("configure_event: draw_img() failed\n");
        return TRUE;
    }


    return TRUE;
}


/* expose_event redraws the screen from the backing pixbuf */
gboolean
expose_event(GtkWidget * widget, GdkEventExpose * event, gpointer data)
{
    cairo_t *cr;
    GdkPixbuf *pb;
    int pixbufnum = (int) (long) data;

    if (!widget) {
        FAIL_MSG("expose_event: invalid params\n");
        return FALSE;
    }


    pb = displays[pixbufnum].buf;

    cr = gdk_cairo_create(gtk_widget_get_window(widget));
    if (!cr) {
        FAIL_MSG("expose_event: gdk_cairo_create() failed\n");
        return FALSE;
    }


    gdk_cairo_set_source_pixbuf(cr, pb, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);

    return FALSE;
}


/* get_location returns the linear location in a pixbuf given its x and y */
unsigned long get_location(int pixbufnum, int width, int x, int y)
{
    unsigned long location = 0;
    float step = 0;

    if ((x < 0) || (y < 0)) {
        FAIL_MSG("get_location: invalid params\n");
        return 1;
    }


    switch (pixbufnum) {
    case PIXBUF_WHOLE_HILBERT:
        if (disp.disp_hilbert == DISP_HILBERTFLIPPED) {
            location = xy2d(width, y, x);
        } else {
            location = xy2d(width, x, y);
        }
        step = zoom[ZOOM_WHOLE].step_hilbert;
        location = location * step;
        break;
    case PIXBUF_ZOOM_HILBERT:
        if (disp.disp_hilbert == DISP_HILBERTFLIPPED) {
            location = xy2d(width, y, x);
        } else {
            location = xy2d(width, x, y);
        }
        step = zoom[ZOOM_ZOOM].step_hilbert;
        location = zoom[ZOOM_WHOLE].start + (location * step);
        break;
    case PIXBUF_WHOLE_ZIGZAG:
        if (disp.disp_zigzag == DISP_LINEAR) {
            location = (y * width) + x;
        } else {
            if ((y % 2) == 0) {
                location = (y * width) + x;
            } else {
                location = (y * width) + (width - x - 1);
            }
        }
        step = zoom[ZOOM_WHOLE].step_zigzag;
        location = location * step;
        break;
    case PIXBUF_ZOOM_ZIGZAG:
        if (disp.disp_zigzag == DISP_LINEAR) {
            location = (y * width) + x;
        } else {
            if ((y % 2) == 0) {
                location = (y * width) + x;
            } else {
                location = (y * width) + (width - x - 1);
            }
        }
        step = zoom[ZOOM_ZOOM].step_zigzag;
        location = zoom[ZOOM_WHOLE].start + (location * step);
        break;
    default:
        fprintf(stderr, "get_location: invalid pixbufnum\n");
        return 2;
    }

    if (location > shm->filestat.st_size) {
        return shm->filestat.st_size;
    } else {
        return location;
    }
}


/* button_event processes the click/drag callbacks */
int
button_event(GtkWidget * widget, int x, int y, int button,
             gpointer data, int state)
{
    int width;
    unsigned long location, draglocation;
    int pixbufnum = (int) (long) data;
    unsigned long wholestart, wholeend, zoomstart, zoomend;
    int adjusted = 0;

    if (!widget) {
        FAIL_MSG("button_event: invalid params\n");
        return 1;
    }


    /* if the mouse has dragged outside of the window, ignore
     * the update */
    if ((x < 0) || (y < 0)) {
        return 0;
    }

    /* calc current window starts and ends */
    if (calcwindows(&wholestart, &wholeend, &zoomstart, &zoomend) != 0) {
        FAIL_MSG("button_event: calcwindows() failed\n");
        return 2;
    }


    if (((button == 1) || (button == 3)) && widget != NULL) {
        width = widget->allocation.width;

        /* find location and step */
        location = get_location(pixbufnum, width, x, y);
        if (disp.drag && (disp.dragy > 0)) {
            draglocation = get_location(pixbufnum, width, x, disp.dragy);
        } else {
            draglocation = location;
        }

        /* move marker */
        if ((pixbufnum == PIXBUF_WHOLE_HILBERT)
            || (pixbufnum == PIXBUF_WHOLE_ZIGZAG)) {
            if (button == 1) {
                /* check if we're outside the current window and button down */
                if ((state == BUTTON_DOWN)
                    && ((location < wholestart) || (location > wholeend))) {
                    wholestart = location;
                    zoom[ZOOM_WHOLE].start = wholestart;
                    adjusted = 1;
                } else if ((state == BUTTON_UP) && (location > wholestart)
                           && (location < wholeend) && (disp.drag == 0)) {
                    /* inside current region and button up and not dragging */
                    wholestart = location;
                    zoom[ZOOM_WHOLE].start = wholestart;
                    adjusted = 1;
                } else if ((state == BUTTON_DRAG) && disp.drag
                           && (disp.dragy > 0)
                           && (draglocation > wholestart)
                           && (draglocation < wholeend)) {
                    /* dragging */
                    if ((wholestart + location > draglocation) &&
                        (wholeend + (location - draglocation) <
                         shm->filestat.st_size)) {

                        wholestart =
                            wholestart + (location - draglocation);
                        wholeend = wholeend + (location - draglocation);

                        zoom[ZOOM_WHOLE].start = wholestart;
                        zoom[ZOOM_WHOLE].end = wholeend;
                        disp.dragy = y;
                        adjusted = 1;
                    }
                }

                if (!disp.drag && adjusted) {
                    if (location > wholeend) {
                        wholeend = shm->filestat.st_size;
                        zoom[ZOOM_WHOLE].end = wholeend;
                    }
                }

            } else if ((button == 3) && (state == BUTTON_DOWN)) {
                zoom[ZOOM_WHOLE].end = location;
                wholeend = location;
                if (location < wholestart) {
                    zoom[ZOOM_WHOLE].start = 0;
                    wholestart = 0;
                }
                adjusted = 1;
            }

            if (adjusted) {
                /* set the zoomed step */

                zoom[ZOOM_ZOOM].step_hilbert =
                    (long double) (wholeend - wholestart) / (512 * 512);
                zoom[ZOOM_ZOOM].step_zigzag =
                    (long double) (wholeend - wholestart) / (128 * 512);

                if (auto_draw(wx.hilbert_whole, PIXBUF_WHOLE_HILBERT) != 0) {
                    FAIL_MSG
                        ("button_event: auto_draw(hilbert_whole) failed\n");
                    return 3;
                }

                if (auto_draw(wx.zigzag_whole, PIXBUF_WHOLE_ZIGZAG) != 0) {
                    FAIL_MSG
                        ("button_event: auto_draw(zigzag_whole) failed\n");
                    return 4;
                }

                if (auto_draw(wx.zigzag_win, PIXBUF_WIN) != 0) {
                    FAIL_MSG
                        ("button_event: auto_draw(zigzag_win) failed\n");
                    return 5;
                }

                if (auto_draw(wx.zigzag_zoom, PIXBUF_ZOOM_ZIGZAG) != 0) {
                    FAIL_MSG
                        ("button_event: auto_draw(zigzag_zoom) failed\n");
                    return 6;
                }

                if (auto_draw(wx.hilbert_zoom, PIXBUF_ZOOM_HILBERT) != 0) {
                    FAIL_MSG
                        ("button_event: auto_draw(hilbert_zoom) failed\n");
                    return 7;
                }

            }
        } else {
            /* zoom hilbert and zoom zigzag */
            if (button == 1) {
                /* check if we're outside the current window and button down */
                if ((state == BUTTON_DOWN)
                    && ((location < zoomstart) || (location > zoomend))) {
                    zoom[ZOOM_ZOOM].start = location;
                    adjusted = 1;
                } else if ((state == BUTTON_UP) && (location > zoomstart)
                           && (location < zoomend) && (disp.drag == 0)) {
                    /* inside current region and button up and not dragging */
                    zoom[ZOOM_ZOOM].start = location;
                    adjusted = 1;
                } else if ((state == BUTTON_DRAG) && disp.drag
                           && (disp.dragy > 0)
                           && (draglocation > zoomstart)
                           && (draglocation < zoomend)) {
                    /* dragging */
                    if ((zoomstart + location > draglocation + wholestart)
                        && (zoomend + (location - draglocation) <
                            wholeend)) {

                        zoomstart = zoomstart + (location - draglocation);
                        zoomend = zoomend + (location - draglocation);

                        zoom[ZOOM_ZOOM].start = zoomstart;
                        zoom[ZOOM_ZOOM].end = zoomend;
                        disp.dragy = y;
                        adjusted = 1;
                    } else {
                    }
                }

                if (!disp.drag && adjusted && (location > zoomend)) {
                    zoom[ZOOM_ZOOM].end = wholeend;
                }

            } else if ((button == 3) && (state == BUTTON_DOWN)) {
                zoom[ZOOM_ZOOM].end = location;
                if (location < zoomstart) {
                    zoom[ZOOM_ZOOM].start = wholestart;
                }
                adjusted = 1;
            }
            if (adjusted) {
                if (update_children() != 0) {
                    FAIL_MSG("button_event: update_children() failed\n");
                    return 8;
                }

                if (auto_draw(wx.zigzag_zoom, PIXBUF_ZOOM_ZIGZAG) != 0) {
                    FAIL_MSG
                        ("button_event: auto_draw(zigzag_zoom) failed\n");
                    return 9;
                }

                if (auto_draw(wx.hilbert_zoom, PIXBUF_ZOOM_HILBERT) != 0) {
                    FAIL_MSG
                        ("button_event: auto_draw(hilbert_zoom) failed\n");
                    return 10;
                }

                if (auto_draw(wx.zigzag_win, PIXBUF_WIN) != 0) {
                    FAIL_MSG
                        ("button_event: auto_draw(zigzag_win) failed\n");
                    return 11;
                }

            }
        }
    }
    return 0;
}


/* button_press_event is a mouse click callback */
gboolean
button_press_event(GtkWidget * widget, GdkEventButton * event,
                   gpointer data)
{
    disp.drag = 0;

    if (!widget || !event) {
        FAIL_MSG("button_press_event: invalid params\n");
        return TRUE;
    }


    disp.dragy = (int) (event->y);

    if (button_event
        (widget, (int) (event->x), (int) (event->y), event->button, data,
         BUTTON_DOWN) != 0) {
        FAIL_MSG("button_press_event: button_event() failed\n");
        return TRUE;
    }


    return TRUE;
}


/* button_press_release is a mouse click callback */
gboolean
button_release_event(GtkWidget * widget, GdkEventButton * event,
                     gpointer data)
{
    if (!widget || !event) {
        FAIL_MSG("button_release_event: invalid params\n");
        return TRUE;
    }


    if (!disp.drag) {
        if (button_event(widget, (int) (event->x), (int) (event->y),
                         event->button, data, BUTTON_UP) != 0) {
            FAIL_MSG("button_release_event: button_event() failed\n");
            return TRUE;
        }

    }

    disp.dragy = -1;

    return TRUE;
}


/* motion_notify_event is a mouse drag callback */
gboolean
motion_notify_event(GtkWidget * widget, GdkEventMotion * event,
                    gpointer data)
{
    int button = 0;

    if (!widget || !event) {
        FAIL_MSG("motion_notify_event: invalid params\n");
        return TRUE;
    }


    if (event->state & GDK_BUTTON1_MASK) {
        button = 1;
        disp.drag = 1;
    } else if (event->state & GDK_BUTTON3_MASK) {
        button = 3;
        disp.drag = 1;
    } else {
        disp.drag = 0;
    }

    if (disp.drag) {
        if (button_event
            (widget, (int) (event->x), (int) (event->y), button, data,
             BUTTON_DRAG) != 0) {
            FAIL_MSG("motion_notify_event: button_event() failed\n");
            return TRUE;
        }

    }

    return TRUE;
}


/* quit is a window destroy callback */
void quit()
{
    if (kill_children() != 0)
        FAIL_MSG("quit: kill_children() failed\n");


    /* close semaphore and shared memory */

    if (sem_close(shm_ctx->sem) != 0) {
        FAIL_ERR("quit: sem_close() failed\n");
    }


    if (sem_unlink(shm->semname) != 0) {
        FAIL_ERR("quit: sem_unlink() failed\n");
    }

    if (shm_unlink(shm_ctx->buf_name) != 0) {
        FAIL_ERR("quit: shm_unlink() failed\n");
    }


    exit(0);
}


/* auto_draw is a wrapper for draw_img() */
int auto_draw(GtkWidget * widget, int pixbufnum)
{
    if (!widget) {
        FAIL_MSG("auto_draw: invalid params\n");
        return 1;
    }


    int width = displays[pixbufnum].width;
    int height = displays[pixbufnum].height;
    if (draw_img(pixbufnum) != 0) {
        FAIL_MSG("auto_draw: draw_img() failed\n");
        return 2;
    }


    /* and update it */
    gtk_widget_queue_draw_area(widget, 0, 0, width, height);

    return 0;
}
