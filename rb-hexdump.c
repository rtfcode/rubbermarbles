/*
 * Rubber Marbles - K Sheldrake
 * rb-hexdump.c
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 * 
 * Provides a Gtk Hexdump utility for viewing data.
 */


#include "rb-hexdump.h"

/* shared memory access */
struct shm_buf *shm_ctx;
struct rb_shm *shm;
struct hd_ctx *gctx;
sem_t *sem;
int shm_destroy;


/* enable_usr1 enables signal handling for SIGUSR1 */
int enable_usr1()
{
    if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
        FAIL_ERR("enable_usr1: cannot catch SIGUSR1\n");
        return 1;
    }

    return 0;
}


/* disable_usr1 disables signal handling for SIGUSR1 */
int disable_usr1()
{
    if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
        FAIL_ERR("disable_usr1: cannot ignore SIGUSR1\n");
        return 1;
    }

    return 0;
}


/* sig_handler is a signal callback.  SIGUSR1 means buffer has changed; SIGHUP means close window */
void sig_handler(int signo)
{
    if (signo == SIGUSR1) {
        if (disable_usr1() != 0) {
            FAIL_MSG("sig_handler: disable_usr1() failed\n");
            return;
        }

        gctx->reload = 1;
    } else if (signo == SIGHUP) {
        if (disable_usr1() != 0) {
            FAIL_MSG("sig_handler: disable_usr1() failed\n");
            return;
        }

		cleanup();
        exit(0);
    }
}


/* onIdle is run whenever there is time.  It monitors ctx->reload and
 * calls hexdump_redraw() and enable_usr1() when it is set.
 */
gboolean onIdle(gpointer data)
{
    struct hd_ctx *ctx = (struct hd_ctx *) data;

    if (ctx->reload) {
        if (hexdump_redraw(gctx) != 0) {
            FAIL_MSG("onIdle: hexdump_redraw() failed\n");
            return TRUE;
        }

        ctx->reload = 0;
        if (enable_usr1() != 0) {
            FAIL_MSG("onIdle: enable_usr1() failed\n");
            return TRUE;
        }

    }

    return TRUE;
}


/* set_scroll sets the values on the GtkAdjustment that the vertical scroll bar
 * uses.
*/
int set_scroll(struct hd_ctx *ctx)
{
    if (!ctx) {
        FAIL_MSG("set_scroll: invalid params\n");
        return 1;
    }

    /* check if the scroll offset is past the start of the last display window */
    if (ctx->scroll_offset >
        (ctx->bufsize - (ctx->rows * ctx->cols * ctx->dsize))) {
        /* check if the new scroll offset will be positive */
        if (ctx->bufsize > (ctx->rows * ctx->cols * ctx->dsize)) {
            ctx->scroll_offset =
                ctx->bufsize - (ctx->rows * ctx->cols * ctx->dsize);
        } else {
            ctx->scroll_offset = 0;
        }
    }

    /* set scroll values */
    int val = (float) ctx->scroll_offset / (ctx->cols * ctx->dsize);
    int upper = (float) ctx->totalrows;
    int pagei = (float) ctx->rows;

    /* set the adjustment and cause a redraw of the scroll bar */
    gtk_adjustment_configure(GTK_ADJUSTMENT(ctx->adj), val, 0.0, upper,
                             1.0, pagei, pagei);
    return 0;
}


/* change_dsize is a menu callback.  It changes the data word size of
 * the visualisation.
 */
void change_dsize(gpointer data, guint action, GtkWidget * widget)
{
    struct hd_ctx *ctx = (struct hd_ctx *) data;

    if (!ctx || !widget) {
        FAIL_MSG("change_dsize: invalid params\n");
        return;
    }


    /* check for a valid dsize */
    if ((action == 1) || (action == 2) || (action == 4) || (action == 8)) {
        /* make sure the menu item was activated rather than deactivated */
        if (GTK_CHECK_MENU_ITEM(widget)->active) {
            /* set the dsize */
            ctx->dsize = action;
            /* calculate the parameters, set the colour, set the scroll
             * and populate */
            /* initialise nudge */
            ctx->nudge = 0;
            ctx->colnudge = 0;
            if (set_dims(ctx) != 0) {
                FAIL_MSG("change_dsize: set_dims() failed\n");
                return;
            }

            if (set_col_cols(ctx) != 0) {
                FAIL_MSG("change_dsize: set_col_cols() failed\n");
                return;
            }

            if (set_scroll(ctx) != 0) {
                FAIL_MSG("change_dsize: set_scroll() failed\n");
                return;
            }

            if (populate(ctx) != 0) {
                FAIL_MSG("change_dsize: populate() failed\n");
                return;
            }

        }
    } else {
        fprintf(stderr, "change_dsize: invalid dsize\n");
        return;
    }

}


/* change_endian is a menu callback.  It changes the endianness of
 * the visualisation.
 */
void change_endian(gpointer data, guint action, GtkWidget * widget)
{
    struct hd_ctx *ctx = (struct hd_ctx *) data;

    if (!ctx || !widget) {
        FAIL_MSG("change_endian: invalid params\n");
        return;
    }


    /* check for valid endian */
    if ((action == HD_LITTLE_ENDIAN) || (action == HD_BIG_ENDIAN)) {
        /* make sure the menu item was activated rather than deactivated */
        if (GTK_CHECK_MENU_ITEM(widget)->active) {
            /* set the endian */
            ctx->endian = action;
            /* repopulate the array */
            if (populate(ctx) != 0) {
                FAIL_MSG("change_endian: populate() failed\n");
                return;
            }

        }
    } else {
        fprintf(stderr, "change_endian: invalid endian\n");
        return;
    }

}


/* set_nudge is a menu callback.  It nudges the visualised data left or
 * right, by byte or column, for pretty alignment purposes.
 */
void set_nudge(gpointer data, guint action, GtkWidget * widget)
{
    struct hd_ctx *ctx = (struct hd_ctx *) data;

    if (!ctx) {
        FAIL_MSG("set_nudge: invalid params\n");
        return;
    }


    /* check for valid endian */
    switch (action) {
    case NUDGE_LEFT:
        ctx->nudge = ctx->nudge + 1;
        break;
    case NUDGE_RIGHT:
        ctx->nudge = ctx->nudge - 1;
        break;
    case NUDGE_COLLEFT:
        ctx->colnudge = ctx->colnudge + 1;
        break;
    case NUDGE_COLRIGHT:
        ctx->colnudge = ctx->colnudge - 1;
        break;
    default:
        fprintf(stderr, "set_nudge: invalid action\n");
        return;
    }

    if (constrain_nudge(ctx) != 0) {
        FAIL_MSG("set_nudge: constrain_nudge() failed\n");
        return;
    }


    /* repopulate the array */
    if (populate(ctx) != 0) {
        FAIL_MSG("set_nudge: populate() failed\n");
        return;
    }


}


/* set_update is a menu callback.  It holds/unholds the updating of the
 * display */
void set_update(gpointer data, guint action, GtkWidget * widget)
{
    struct hd_ctx *ctx = (struct hd_ctx *) data;

    if (!ctx) {
        FAIL_MSG("set_update: invalid params\n");
        return;
    }


    /* check for valid endian */
    switch (action) {
    case UPDATE_CONT:
        enable_usr1();

        /* repopulate the array */
        if (hexdump_redraw(ctx) != 0) {
            FAIL_MSG("set_update: hexdump_redraw() failed\n");
            return;
        }

        break;
    case UPDATE_HOLD:
        disable_usr1();

        break;
    default:
        fprintf(stderr, "set_update: invalid action\n");
        return;
    }

}


/* Our menu, an array of GtkItemFactoryEntry structures that defines each menu item */
GtkItemFactoryEntry hd_menu_items[] = {
    {"/_File", NULL, NULL, 0, "<Branch>"}
    ,
    {"/File/_Quit", "<CTRL>Q", quit_local, 0, "<StockItem>", GTK_STOCK_QUIT}
    ,
    {"/_Word size", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Word size/uint8", "1", change_dsize, 1, "<RadioItem>"}
    ,
    {"/Word size/uint16", "2", change_dsize, 2, "/Word size/uint8"}
    ,
    {"/Word size/uint32", "4", change_dsize, 4, "/Word size/uint8"}
    ,
    {"/Word size/uint64", "8", change_dsize, 8, "/Word size/uint8"}
    ,
    {"/_Endian", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Endian/Little", "l", change_endian, HD_LITTLE_ENDIAN, "<RadioItem>"}
    ,
    {"/Endian/Big", "b", change_endian, HD_BIG_ENDIAN, "/Endian/Little"}
    ,
    {"/_Nudge", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Nudge/Byte left", "n", set_nudge, NUDGE_LEFT, "<Item>"}
    ,
    {"/Nudge/Byte right", "m", set_nudge, NUDGE_RIGHT, "<Item>"}
    ,
    {"/Nudge/Column left", "<shift>n", set_nudge, NUDGE_COLLEFT, "<Item>"}
    ,
    {"/Nudge/Column right", "<shift>m", set_nudge, NUDGE_COLRIGHT, "<Item>"}
    ,
    {"/_Update", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Update/Continuous", "c", set_update, UPDATE_CONT, "<RadioItem>"}
    ,
    {"/Update/Hold", "h", set_update, UPDATE_HOLD, "/Update/Continuous"}
    ,
};


/* Returns a menubar widget made from the above menu */
GtkWidget *hd_menubar_menu(struct hd_ctx * ctx)
{
    GtkWidget *window;
    GtkItemFactory *item_factory;
    GtkAccelGroup *accel_group;
    gint nmenu_items;

    if (!ctx) {
        FAIL_MSG("hd_menubar_menu: invalid params\n");
        return NULL;
    }


    window = ctx->window;
    if (sizeof(hd_menu_items[0]) == 0) {
        FAIL_MSG("hd_menubar_menu: hd_menu_items[0] size is 0\n");
        return NULL;
    }


    nmenu_items = sizeof(hd_menu_items) / sizeof(hd_menu_items[0]);

    /* Make an accelerator group (shortcut keys) */
    accel_group = gtk_accel_group_new();
    if (!accel_group) {
        FAIL_MSG("hd_menubar_menu: gtk_accel_group_new() failed\n");
        return NULL;
    }


    /* Make an ItemFactory (that makes a menubar) */
    item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",
                                        accel_group);
    if (!item_factory) {
        FAIL_MSG("hd_menubar_menu: gtk_item_factory_new() failed\n");
        return NULL;
    }


    /* This function generates the menu items. Pass the item factory,
       the number of items in the array, the array itself, and any
       callback data for the the menu items. */
    gtk_item_factory_create_items(item_factory, nmenu_items, hd_menu_items,
                                  ctx);

    /* Attach the new accelerator group to the window. */
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

    /* Finally, return the actual menu bar created by the item factory. */
    return gtk_item_factory_get_widget(item_factory, "<main>");
}


/* quit function destroys the hexdump window */
void quit_local(gpointer data, guint action, GtkWidget * widget)
{
    struct hd_ctx *ctx = (struct hd_ctx *) data;

    if (!ctx) {
        FAIL_MSG("quit_local: invalid params\n");
        return;
    }


    /* destroy the hexdump window */
    gtk_widget_destroy(ctx->window);
}

/* destroy function signals the end of the hexdump */
gboolean destroy_local(GtkWidget * widget, gpointer data)
{
    struct hd_ctx *ctx = (struct hd_ctx *) data;

    if (!ctx) {
        FAIL_MSG("destroy_local: invalid params\n");
        return TRUE;
    }


    gtk_widget_hide(ctx->window);

	cleanup();
    exit(0);

    return TRUE;
}


/* scroll_changed is the callback for the scrollbar */
gboolean scroll_changed(GtkWidget * widget, gpointer data)
{
    struct hd_ctx *ctx = (struct hd_ctx *) data;
    if (!ctx) {
        FAIL_MSG("scroll_changed: invalid params\n");
        return TRUE;
    }


    /* calculate scroll_offset from GtkAdjustment */
    ctx->scroll_offset =
        (unsigned long) gtk_adjustment_get_value(GTK_ADJUSTMENT(ctx->adj))
        * ctx->cols * ctx->dsize;
    /* repopulate */
    if (populate(ctx) != 0) {
        FAIL_MSG("scroll_changed: populate() failed\n");
        return TRUE;
    }

    return TRUE;
}


/* scroll_event is the callback for the mouse scroll event */
gboolean
scroll_event(GtkWidget * widget, GdkEventScroll * event, gpointer data)
{
    float value;
    struct hd_ctx *ctx = (struct hd_ctx *) data;
    if (!ctx || !event) {
        FAIL_MSG("scroll_event: invalid params\n");
        return TRUE;
    }


    /* get the old value */
    value = gtk_adjustment_get_value(GTK_ADJUSTMENT(ctx->adj));

    /* modify it */
    if (event->direction == GDK_SCROLL_UP) {
        value -= 10.0;
    } else if (event->direction == GDK_SCROLL_DOWN) {
        value += 10.0;
    }

    /* force value to within range */
    if (value > (ctx->totalrows - ctx->rows)) {
        value = ctx->totalrows - ctx->rows;
    }
    if (value < 0.0) {
        value = 0.0;
    }

    gtk_adjustment_set_value(GTK_ADJUSTMENT(ctx->adj), value);
    return TRUE;
}


/* resize_event is the callback for window resize */
gboolean
resize_event(GtkWidget * widget, GdkEventConfigure * event, gpointer data)
{
    struct hd_ctx *ctx = (struct hd_ctx *) data;
    int xsize, ysize;
    if (!ctx || !widget) {
        FAIL_MSG("resize_event: invalid params\n");
        return FALSE;
    }


    xsize = widget->allocation.width;
    ysize = widget->allocation.height;

    /* see if the window has been resized. This is important
     * as we're actually camping on the configure_event which
     * fires for moves as well as resizes. */
    if ((xsize != ctx->xsize) || (ysize != ctx->ysize)) {

        /* kill any existing timer */
        if (ctx->timer_id) {
            g_source_remove(ctx->timer_id);
        }

        /* create 500ms timer to delay the resize for performance reasons */
        ctx->timer_id = g_timeout_add(DRAGTIMEOUT, resize_table, ctx);
    }
    return FALSE;
}


/* resize_table does the actual resize. It is called when the timer fires */
gboolean resize_table(gpointer data)
{
    struct hd_ctx *ctx = (struct hd_ctx *) data;
    int xsize, ysize;
    int pcols, prows;

    if (!ctx || !ctx->window) {
        FAIL_MSG("resize_table: invalid params\n");
        return FALSE;
    }


    /* reset timer */
    ctx->timer_id = 0;

    xsize = ctx->window->allocation.width;
    ysize = ctx->window->allocation.height;

    /* check if window has changed size */
    if ((xsize != ctx->xsize) || (ysize != ctx->ysize)) {
        ctx->xsize = xsize;
        ctx->ysize = ysize;

        /* store the previous cols and rows values */
        pcols = ctx->cols;
        prows = ctx->rows;

        /* calculate the new dimensions */
        if (set_dims(ctx) != 0) {
            FAIL_MSG("resize_table: set_dims() failed\n");
            return FALSE;
        }


        /* see if the table needs to grow */
        if (((ctx->cols * ctx->dsize) > ctx->tablecols)
            || (ctx->rows > ctx->tablerows)) {
            /* table needs to be bigger */
            if (enlarge_table(ctx, pcols, prows) != 0) {
                FAIL_MSG("resize_table: enlarge_table() failed\n");
                return FALSE;
            }

        }

        if (set_scroll(ctx) != 0) {
            FAIL_MSG("resize_table: set_scroll() failed\n");
            return FALSE;
        }

        if (populate(ctx) != 0) {
            FAIL_MSG("resize_table: populate() failed\n");
            return FALSE;
        }

        if (set_col_cols(ctx) != 0) {
            FAIL_MSG("resize_table: set_col_cols() failed\n");
            return FALSE;
        }

    }

    return FALSE;
}


/* hexdump_free free()s the memory used by hexdump */
int hexdump_free(void *rawctx)
{
    struct hd_ctx *ctx = (struct hd_ctx *) rawctx;

    if (!ctx) {
        FAIL_MSG("hexdump_free: invalid params\n");
        return 1;
    }


    if (destroy_table(ctx) != 0) {
        FAIL_MSG("hexdump_free: destroy_table failed\n");
        return 2;
    }

    gtk_widget_destroy(ctx->scroll);
    gtk_widget_destroy(ctx->hexevent);
    gtk_widget_destroy(ctx->hbox);
    gtk_widget_destroy(ctx->vbox);
    gtk_widget_destroy(ctx->window);
    free(ctx);
    return 0;
}


/* hexdump_init initialises the context */
void *hexdump_init(unsigned int xsize, unsigned int ysize, unsigned int x,
                   unsigned int y)
{
	PangoFontDescription *pfd;
    struct hd_ctx *ctx;
	int charwidth;
    char *homepath = NULL;

    if (!xsize || !ysize) {
        FAIL_MSG("hexdump_init: invalid params\n");
        return NULL;
    }


    ctx = (struct hd_ctx *) malloc(sizeof(struct hd_ctx));
    if (!ctx) {
        FAIL_MSG("hexdump_init: malloc failed\n");
        return NULL;
    }


    memset(ctx, 0, sizeof(struct hd_ctx));

	homepath = getenv("HOME");
    if (!homepath) {
        FAIL_MSG("hexdump_init: HOME env var not set\n");
        return NULL;
    }

	if (configuration(homepath, "gtkfont", ctx->gtkfont) != 0) {
		FAIL_MSG("hexdump_init: configuration() failed\n");
		return NULL;
	}

	
	pfd = pango_font_description_from_string(ctx->gtkfont);
	if (!pfd) {
		FAIL_MSG
		("hexdump_init: pango_font_description_from_string() failed\n");
		return NULL;
	}

	charwidth = (pango_font_description_get_size(pfd) * 96 / (PANGO_SCALE * 72)) + 2;
	
    ctx->xsize = xsize;
    ctx->ysize = ysize;
    ctx->xpos = x;
    ctx->ypos = y;

//    ctx->elexsize = ELEXSIZE;
//    ctx->eleysize = ELEYSIZE;
    ctx->eleysize = charwidth;
    ctx->elexsize = (charwidth + 1) / 2;
    ctx->cols = 0;
    ctx->rows = 0;

    ctx->hextable = NULL;
    ctx->table_widgets = NULL;

    ctx->timer_id = 0;

    ctx->nudge = 0;
    ctx->colnudge = 0;

    ctx->reload = 0;

    return (void *) ctx;
}


/* hexdump_filedesc sets up the filedesc buffer details */
int
hexdump_filedesc(void *rawctx, int fd, struct stat *filestat,
                 unsigned long offset, unsigned long size,
                 unsigned int dsize, int endian)
{
    struct hd_ctx *ctx = (struct hd_ctx *) rawctx;

    if (!rawctx || !fd || !filestat || !size ||
        ((dsize != 1) && (dsize != 2) && (dsize != 4) && (dsize != 8)) ||
        ((endian != HD_LITTLE_ENDIAN) && (endian != HD_BIG_ENDIAN))) {
        FAIL_MSG("hexdump_filedesc: invalid params\n");
        return 1;
    }


    if (copyshm(ctx) != 0) {
        FAIL_MSG("hexdump_filedesc: copyshm() failed\n");
        return 2;
    }


    ctx->dsize = dsize;
    ctx->endian = endian;

    return 0;
}


/* hexdump_redraw indicates the shared memory values have changed */
int hexdump_redraw(void *rawctx)
{
    struct hd_ctx *ctx = (struct hd_ctx *) rawctx;

    if (!ctx || !ctx->hextable || !ctx->scroll) {
        FAIL_MSG("hexdump_redraw: invalid params\n");
        return 1;
    }


    /* copy the shared memory values */
    if (copyshm(ctx) != 0) {
        FAIL_MSG("hexdump_redraw: copyshm() failed\n");
        return 2;
    }

    /* set the dimensions */
    if (set_dims(ctx) != 0) {
        FAIL_MSG("hexdump_redraw: set_dims() failed\n");
        return 3;
    }

    /* set the scroll bar */
    if (set_scroll(ctx) != 0) {
        FAIL_MSG("hexdump_redraw: set_scroll() failed\n");
        return 4;
    }

    /* repopulate */
    if (populate(ctx) != 0) {
        FAIL_MSG("hexdump_redraw: populate() failed\n");
        return 5;
    }

    /* force the hextable and scrollbar to be redrawn */
    gtk_widget_queue_draw(ctx->hextable);
    gtk_widget_queue_draw(ctx->scroll);

    return 0;
}


/* create_eventbox creates an eventbox widget to hold the table
 * widget.  This is so we can get mouse scroll events. */
int create_eventbox(struct hd_ctx *ctx)
{
    if (!ctx) {
        FAIL_MSG("create_eventbox: invalid params\n");
        return 1;
    }


    ctx->hexevent = gtk_event_box_new();
    if (!ctx->hexevent) {
        FAIL_MSG("gtk_event_box_new() failed\n");
        return 2;
    }


    gtk_container_add(GTK_CONTAINER(ctx->hbox), ctx->hexevent);
    gtk_widget_show(ctx->hexevent);

    return 0;
}


/* destroy_table destroys the table widgets and the table */
int destroy_table(struct hd_ctx *ctx)
{
    int i;

    if (!ctx || !ctx->table_widgets || !ctx->hextable) {
        FAIL_MSG("destroy_table: invalid params\n");
        return 1;
    }


    /* destroy table widgets */
    for (i = 0; i < (ctx->tablecols + 2) * ctx->tablerows; i++) {
        gtk_widget_destroy(ctx->table_widgets[i]);
    }
    /* destroy table */
    gtk_widget_destroy(ctx->hextable);
    ctx->hextable = NULL;
    /* free array */
    free(ctx->table_widgets);
    ctx->table_widgets = NULL;

    return 0;
}


/* set_dims sets the dimensions of the table */
int set_dims(struct hd_ctx *ctx)
{
    int cols;
    if (!ctx) {
        FAIL_MSG("set_dims: invalid params\n");
        return 1;
    }


    /* calculate the number of columns using Kev's magic
     * formula :) 
     */
    cols = (((ctx->xsize - ADDRSIZE) / ctx->elexsize) - SCROLLWIDTH) /
        ((BYTECOLWIDTH * ctx->dsize) + 1);

    /* fix the number of cols based on the dsize.
     * This makes regular blocks of columns */
    if (ctx->dsize != 8) {
        ctx->cols = (cols / (4 / ctx->dsize)) * (4 / ctx->dsize);
    } else {
        ctx->cols = cols;
    }

    /* calc rows */
    ctx->rows = (ctx->ysize - MENUHEIGHT) / ctx->eleysize;

    /* also calc the total rows at this width for scrolling
     * and fix for a final partial row */
    ctx->totalrows = ctx->bufsize / (ctx->cols * ctx->dsize);
    if (ctx->totalrows * ctx->cols * ctx->dsize != ctx->bufsize) {
        ctx->totalrows++;
    }
    return 0;
}


/* set_col_and_font sets a label widget's colour and font */
int
set_col_and_font(GtkWidget ** table, int tablecols, int dsize, int i,
                 int j, int col, int font)
{
    GdkColor blue;
    GdkColor black;
    GdkColor green;
    PangoFontDescription *pfd;

    if (!table || !tablecols || ((dsize != 1) && (dsize != 2) &&
                                 (dsize != 4) && (dsize != 8))) {
        FAIL_MSG("set_col_and_font: invalid params\n");
        return 1;
    }


    /* check if we have nothing to do */
    if (!font && !col) {
        return 0;
    }

    /* set colour */
    if (col) {
        if (!gdk_color_parse("blue", &blue)
            || !gdk_color_parse("black", &black)
            || !gdk_color_parse("green", &green)) {
            FAIL_MSG("set_col_and_font: gdk_color_parse() failed\n");
            return 2;
        }


        if (i > 0) {
            /* colour alternates every 4 bytes for dsizes 1, 2 and 4
             * and every 8 bytes for dsize 8 */
            if (dsize != 8) {
                if (((i - 1) % (8 / dsize)) < (4 / dsize)) {
                    gtk_widget_modify_fg(table[(j * (tablecols + 2)) + i],
                                         GTK_STATE_NORMAL, &black);
                } else {
                    gtk_widget_modify_fg(table[(j * (tablecols + 2)) + i],
                                         GTK_STATE_NORMAL, &blue);
                }
            } else {
                if (((i - 1) % 2) < 1) {
                    gtk_widget_modify_fg(table[(j * (tablecols + 2)) + i],
                                         GTK_STATE_NORMAL, &black);
                } else {
                    gtk_widget_modify_fg(table[(j * (tablecols + 2)) + i],
                                         GTK_STATE_NORMAL, &blue);
                }
            }

        } else if (i == 0) {
            /* make the address labels green */
            gtk_widget_modify_fg(table[(j * (tablecols + 2))],
                                 GTK_STATE_NORMAL, &green);
        }
    }

    /* set the font */
    if (font) {
        pfd = pango_font_description_from_string(gctx->gtkfont);
        if (!pfd) {
            FAIL_MSG
                ("set_col_and_font: pango_font_description_from_string() failed\n");
            return 3;
        }

        gtk_widget_modify_font(table[(j * (tablecols + 2)) + i], pfd);
    }

    return 0;
}


/* make_label creates a new label widget within a table at a specified location.
 * It also sets the colour and font if requested. */
int make_label(GtkWidget ** table, GtkWidget * hextable, int cols,
               int dsize, char *msg, int i, int j, int setcol, int setfont)
{
    if (!table || !hextable || !cols
        || ((dsize != 1) && (dsize != 2) && (dsize != 4) && (dsize != 8))
        || !msg) {
        FAIL_MSG("make_label: invalid params\n");
        return 1;
    }


    /* create new empty label widget */
    table[(j * (cols + 2)) + i] = gtk_label_new(msg);
    if (!table[(j * (cols + 2)) + i]) {
        FAIL_MSG("make_label: gtk_label_new() failed\n");
        return 2;
    }

    if (set_col_and_font(table, cols, dsize, i, j, setcol, setfont) != 0) {
        FAIL_MSG("make_label: set_col_and_font() failed\n");
        return 3;
    }


    /* attach new widget to table */
    gtk_table_attach_defaults(GTK_TABLE(hextable),
                              table[(j * (cols + 2)) +
                                    i], i, i + 1, j, j + 1);
    gtk_widget_show(table[(j * (cols + 2)) + i]);

    return 0;
}

/* enlarge_table makes the table bigger by copying the current table
 * and adding new empty widgets */
int enlarge_table(struct hd_ctx *ctx, int pcols, int prows)
{
    int i, j;
    GtkWidget **newtable = NULL;
    int newcols, newrows;

    if (!ctx || !pcols || !prows) {
        FAIL_MSG("enlarge_table: invalid params\n");
        return 1;
    }


    /* check if the current table is big enough already */
    if (((ctx->cols * ctx->dsize) <= ctx->tablecols)
        && (ctx->rows <= ctx->tablerows)) {
        return 1;
    }

    /* calc new width */
    if ((ctx->cols * ctx->dsize) > ctx->tablecols) {
        newcols = ctx->cols * ctx->dsize;
    } else {
        newcols = ctx->tablecols;
    }

    /* calc new height */
    if (ctx->rows > ctx->tablerows) {
        newrows = ctx->rows;
    } else {
        newrows = ctx->tablerows;
    }

    /* create a new table */
    newtable =
        (GtkWidget **) malloc((newcols + 2) * newrows *
                              sizeof(GtkWidget *));
    if (!newtable) {
        FAIL_MSG("enlarge_table: malloc() failed\n");
        return 2;
    }


    /* resize the Gtk table object */
    gtk_table_resize(GTK_TABLE(ctx->hextable), newrows, newcols + 2);

    /* copy current rows and extend each if necessary */

    if (ctx->tablecols == newcols) {
        /* width is the same, just copy rows as a block */
        memcpy(newtable, ctx->table_widgets,
               (ctx->tablecols +
                2) * ctx->tablerows * sizeof(GtkWidget *));
    } else {
        /* new table is wider, so copy each row individually and extend */
        for (j = 0; j < ctx->tablerows; j++) {
            memcpy(newtable + (j * (newcols + 2)),
                   ctx->table_widgets + (j * (ctx->tablecols + 2)),
                   (ctx->tablecols + 2) * sizeof(GtkWidget *));
            /* set the colour of the old (black) ascii label */
            if (set_col_and_font(newtable, newcols, ctx->dsize,
                                 ctx->tablecols + 1, j, 1, 0) != 0) {
                FAIL_MSG("enlarge_table: set_col_and_font() failed\n");
                return 3;
            }


            /* extend row */
            for (i = ctx->tablecols + 2; i <= newcols; i++) {
                /* create new empty label widget */
                if (make_label
                    (newtable, ctx->hextable, newcols, ctx->dsize, "", i,
                     j, 1, 1) != 0) {
                    FAIL_MSG("enlarge_table: make_label() failed\n");
                    return 4;
                }

            }

            /* add empty ascii label */
            if (make_label
                (newtable, ctx->hextable, newcols, ctx->dsize, "",
                 newcols + 1, j, 0, 1) != 0) {
                FAIL_MSG("enlarge_table: make_label() failed\n");
                return 5;
            }

        }
    }

    /* add extra rows */
    for (j = ctx->tablerows; j < newrows; j++) {
        /* new address widget */
        if (make_label
            (newtable, ctx->hextable, newcols, ctx->dsize, "", 0, j, 1,
             1) != 0) {
            FAIL_MSG("enlarge_table: make_label() failed\n");
            return 6;
        }


        /* add column widgets */
        for (i = 0; i <= newcols; i++) {
            /* create new empty label widget */
            if (make_label
                (newtable, ctx->hextable, newcols, ctx->dsize, "", i, j, 1,
                 1) != 0) {
                FAIL_MSG("enlarge_table: make_label() failed\n");
                return 7;
            }

        }

        /* create new empty ascii label */
        if (make_label
            (newtable, ctx->hextable, newcols, ctx->dsize, "", newcols + 1,
             j, 0, 1) != 0) {
            FAIL_MSG("enlarge_table: make_label() failed\n");
            return 8;
        }

    }

    /* free the old table and point context at new table */
    free(ctx->table_widgets);
    ctx->table_widgets = newtable;
    ctx->tablecols = newcols;
    ctx->tablerows = newrows;

    return 0;
}


/* copyshm copies values from the shared memory via a semaphore */
int copyshm(struct hd_ctx *ctx)
{
    if (!ctx) {
        FAIL_MSG("copyshm: invalid params\n");
        return 1;
    }

    /* lock the shared memory */
    if (sem_wait(sem) != 0) {
        FAIL_ERR("copyshm: sem_wait() failed\n");
        return 2;
    }


    /* copy the params */
    ctx->bufsize = shm->bufsize;
    ctx->offset = shm->offset;
    ctx->type = shm->buf_type;

    /* unlock the shared memory */
    if (sem_post(sem) != 0) {
        FAIL_ERR("copyshm: sem_post() failed\n");
        return 3;
    }

    return 0;
}


/* mapmem mmap()s the visualised area of the file descriptor */
int mapmem(struct hd_ctx *ctx)
{
    if (!ctx) {
        FAIL_MSG("mapmem: invalid params\n");
        return 1;
    }


    /* shift the values to align with a page */
    ctx->mmap_offset = ctx->offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    ctx->mmap_size = ctx->bufsize + (ctx->offset - ctx->mmap_offset);

    /* mmap() it */
    ctx->mmap_ptr =
        mmap((caddr_t) 0, ctx->mmap_size, PROT_READ, MAP_SHARED, ctx->fd,
             ctx->mmap_offset);
//	fprintf(stderr, "mmap_offset = 0x%lx\n", ctx->mmap_offset);
//	fprintf(stderr, "mmap_size = 0x%lx\n", ctx->mmap_size);

    if (ctx->mmap_ptr == MAP_FAILED) {
        perror("mapmem: mmap() failed\n");
        fprintf(stderr, "mmap_offset = 0x%lx\n", ctx->mmap_offset);
        fprintf(stderr, "mmap_size = 0x%lx\n", ctx->mmap_size);
        fprintf(stderr, "fd = %d\n", ctx->fd);

        return 2;
    }

    /* set buffer pointer */
    ctx->buf = ctx->mmap_ptr + (ctx->offset - ctx->mmap_offset);

    return 0;
}


/* unmapmem munmap()s a mmap()ed segment */
int unmapmem(struct hd_ctx *ctx)
{
    if (!ctx) {
        FAIL_MSG("unmapmem: invalid params\n");
        return 1;
    }


    if (munmap(ctx->mmap_ptr, ctx->mmap_size) != 0) {
        FAIL_ERR("unmapmem: munmap() failed\n");
        return 2;
    }


    return 0;
}


/* set_col_cols sets the colours for all the widgets in the table */
int set_col_cols(struct hd_ctx *ctx)
{
    int i, j;
    GdkColor blue;
    GdkColor black;
    if (!ctx || !ctx->table_widgets || !ctx->tablecols) {
        FAIL_MSG("set_col_cols: invalid params\n");
        return 1;
    }


    if (!gdk_color_parse("blue", &blue)
        || !gdk_color_parse("black", &black)) {
        FAIL_MSG("set_col_cols: gdk_color_parse() failed\n");
        return 2;
    }


    for (j = 0; j < ctx->rows; j++) {
        for (i = 1; i <= ctx->cols; i++) {
            if (set_col_and_font(ctx->table_widgets, ctx->tablecols,
                                 ctx->dsize, i, j, 1, 0) != 0) {
                FAIL_MSG("set_col_cols: set_col_and_font() failed\n");
                return 3;
            }

        }
    }
    return 0;
}


/* create_and_populate_table creates and populates the initial table */
int create_and_populate_table(struct hd_ctx *ctx)
{
    int i, j;
    char addrstr[ADDRSIZE + 4];
    char bytestr[ADDRSIZE + 4];
    char asciistr[ASCIISIZE];

    if (!ctx) {
        FAIL_MSG("create_and_populate_table: invalid params\n");
        return 1;
    }


    /* set the size */
    ctx->tablecols = ctx->cols;
    ctx->tablerows = ctx->rows;

    /* create the array */
    ctx->table_widgets =
        (GtkWidget **) malloc((ctx->tablecols + 2) * ctx->rows *
                              sizeof(GtkWidget *));
    if (!(ctx->table_widgets)) {
        FAIL_MSG("create_and_populate_table: malloc() failed\n");
        return 2;
    }


    /* create the Gtk table */
    ctx->hextable = gtk_table_new(ctx->rows, ctx->tablecols + 2, FALSE);
    if (!ctx->hextable) {
        FAIL_MSG("create_and_populate_table: gtk_table_new() failed\n");
        return 3;
    }

    gtk_container_add(GTK_CONTAINER(ctx->hexevent), ctx->hextable);
    gtk_widget_show(ctx->hextable);


    /* map the memory */
    if (ctx->type == BUF_TYPE_FD) {
        if (mapmem(ctx) != 0) {
            FAIL_MSG("create_and_populate_table: mapmem() failed\n");
            return 4;
        }

    }

    /* loop for all rows */
    for (j = 0; j < ctx->rows; j++) {
        /* make address string */
        if (j < ctx->totalrows) {
            snprintf(addrstr, ADDRSIZE + 4, "%016lx:",
                     (long) (ctx->scroll_offset +
                             (j * ctx->cols * ctx->dsize) + ctx->offset));
        } else {
            memset(addrstr, ' ', ADDRSIZE + 1);
            addrstr[ADDRSIZE + 1] = 0x00;
        }
        /* make the label */
        if (make_label
            (ctx->table_widgets, ctx->hextable, ctx->tablecols, ctx->dsize,
             addrstr, 0, j, 1, 1) != 0) {
            FAIL_MSG("create_and_populate_table: make_label() failed\n");
            return 5;
        }

        /* loop for all columns */
        for (i = 1; i <= ctx->tablecols; i++) {
            /* if location is in range, make the value string, otherwise
             * set it to "" */
            if ((i <= ctx->cols)
                && (ctx->scroll_offset + (j * ctx->cols * ctx->dsize) +
                    ((i - 1) * ctx->dsize) + ctx->dsize <= ctx->bufsize)) {
                if (makevalue(ctx, bytestr, i, j) != 0) {
                    FAIL_MSG
                        ("create_and_populate_table: makevalue() failed\n");
                    return 6;
                }

            } else {
                bytestr[0] = 0x00;
            }
            /* make the label */
            if (make_label
                (ctx->table_widgets, ctx->hextable, ctx->tablecols,
                 ctx->dsize, bytestr, i, j, 1, 1) != 0) {
                FAIL_MSG
                    ("create_and_populate_table: make_label() failed\n");
                return 7;
            }


        }

        /* make the ascii string */
        if (makeascii(asciistr,
                      ctx->buf + ctx->scroll_offset +
                      (j * ctx->cols * ctx->dsize),
                      ctx->bufsize - ctx->scroll_offset -
                      (j * ctx->cols * ctx->dsize),
                      ctx->cols * ctx->dsize) != 0) {
            FAIL_MSG("create_and_populate_table: makeascii() failed\n");
            return 8;
        }


        if (make_label
            (ctx->table_widgets, ctx->hextable, ctx->tablecols, ctx->dsize,
             asciistr, ctx->tablecols + 1, j, 0, 1) != 0) {
            FAIL_MSG("create_and_populate_table: make_label() failed\n");
            return 9;
        }

    }

    /* unmap the memory */
    if (ctx->type == BUF_TYPE_FD) {
        if (unmapmem(ctx) != 0) {
            FAIL_MSG("create_and_populate_table: unmapmem() failed\n");
            return 10;
        }

    }


    return 0;
}


/* makevalue makes a hexstring from the given index */
int makevalue(struct hd_ctx *ctx, char *bytestr, int i, int j)
{
    int k;
    unsigned long value;
    unsigned long loc;
    int numbytes;
    int nudge;

    if (!ctx || !bytestr) {
        FAIL_MSG("makevalue: invalid params\n");
        return 1;
    }


    value = 0;
    nudge = (ctx->colnudge * ctx->dsize) + ctx->nudge;

    /* calculate the location */
    loc =
        ctx->scroll_offset + nudge + (j * ctx->cols * ctx->dsize) +
        ((i - 1) * ctx->dsize);
    numbytes = ctx->dsize;

    /* if there aren't enough bytes available, shorten the request */
    if ((loc + numbytes) > ctx->bufsize) {
        numbytes = ctx->bufsize - loc;
    }
    /* shift the bytes into the value */
    if (ctx->endian == HD_BIG_ENDIAN) {
        for (k = 0; k < numbytes; k++) {
            value = (value << 8) | ctx->buf[loc + k];
        }
    } else {
        for (k = numbytes - 1; k >= 0; k--) {
            value = (value << 8) | ctx->buf[loc + k];
        }
    }

    /* convert the value to a hexstring */
    switch (ctx->dsize) {
    case 1:
        snprintf(bytestr, 4, " %02x", (uint8_t) value);
        break;
    case 2:
        snprintf(bytestr, 6, " %04x", (uint16_t) value);
        break;
    case 4:
        snprintf(bytestr, 10, " %08x", (uint32_t) value);
        break;
    case 8:
#ifdef __linux__
        snprintf(bytestr, 18, " %016lx", (uint64_t) value);
#elif __APPLE__
        snprintf(bytestr, 18, " %016llx", (uint64_t) value);
#endif
        break;
    default:
        fprintf(stderr, "makevalue: invalid dsize\n");
        return 2;
    }

    return 0;
}


/* makeascii creates a printable ascii string from a buffer */
int makeascii(char *asciistr, uint8_t * buf, int bufsize, int size)
{
    int i;
    int max;

    if (!asciistr || !buf || !size || (size > ASCIISIZE - 2)) {
        FAIL_MSG("makeascii: invalid params\n");
        return 1;
    }


    if (bufsize <= 0) {
        memset(asciistr, ' ', size + 1);
        asciistr[size + 1] = 0x00;
        return 0;
    }

    /* calc buffer size */
    if (bufsize > size) {
        max = size;
    } else {
        max = bufsize;
        /* clear remaining buffer */
        memset(asciistr + max + 1, ' ', size - max);
    }

    /* insert a space before the ascii string */
    asciistr[0] = ' ';

    /* create printable ascii string */
    for (i = 0; i < max; i++) {
        if ((buf[i] >= 0x20) && (buf[i] < 0x7f)) {
            asciistr[i + 1] = buf[i];
        } else {
            asciistr[i + 1] = '.';
        }
    }

    /* terminate it */
    asciistr[size + 1] = 0x00;

    return 0;
}


/* constrain_nudge shifts the nudge and colnudge values so that they are in range */
int constrain_nudge(struct hd_ctx *ctx)
{
    if (!ctx) {
        FAIL_MSG("constrain_nudge: invalid params\n");
        return 1;
    }


    if (ctx->nudge < 0) {
        ctx->nudge = (ctx->nudge % 8) + 8;
    }
    if (ctx->nudge > 7) {
        ctx->nudge = ctx->nudge % 8;
    }
    if (ctx->colnudge < 0) {
        ctx->colnudge = (ctx->colnudge % ctx->cols) + ctx->cols;
    }
    if (ctx->colnudge > ctx->cols - 1) {
        ctx->colnudge = ctx->colnudge & ctx->cols;
    }

    return 0;
}

/* populate fills in the table values */
int populate(struct hd_ctx *ctx)
{
    int i, j;
    char addrstr[ADDRSIZE + 4];
    char bytestr[ADDRSIZE + 4];
    char asciistr[ASCIISIZE];
    int nudge;
    if (!ctx) {
        FAIL_MSG("populate: invalid params\n");
        return 1;
    }


    /* contrain nudge */
    if (constrain_nudge(ctx) != 0) {
        FAIL_MSG("populate: constrain_nudge() failed\n");
        return 2;
    }


    /* calc nudge */
    nudge = (ctx->colnudge * ctx->dsize) + ctx->nudge;

    /* map the memory */
    if (ctx->type == BUF_TYPE_FD) {
        if (mapmem(ctx) != 0) {
            FAIL_MSG("populate: mapmem() failed\n");
            return 2;
        }

    }
    /* loop for all rows */
    for (j = 0; j < ctx->rows; j++) {
        if (j < ctx->totalrows) {
            /* create address string */
            snprintf(addrstr, ADDRSIZE + 4, "%016lx:",
                     (long) (ctx->scroll_offset + nudge +
                             (j * ctx->cols * ctx->dsize) + ctx->offset));
        } else {
            addrstr[0] = 0x00;
        }
        /* set the address label */
        gtk_label_set_text(GTK_LABEL
                           (ctx->table_widgets
                            [(j * (ctx->tablecols + 2))]), addrstr);

        /* loop for all columns */
        for (i = 1; i <= ctx->tablecols; i++) {
            /* if location in range make value or empty string */
            if ((i <= ctx->cols)
                && (ctx->scroll_offset + nudge +
                    (j * ctx->cols * ctx->dsize) + ((i - 1) * ctx->dsize) +
                    ctx->dsize <= ctx->bufsize)) {
                if (makevalue(ctx, bytestr, i, j) != 0) {
                    FAIL_MSG("populate: makevalue() failed\n");
                    return 3;
                }

            } else {
                bytestr[0] = 0x00;
            }
            /* set the label */
            gtk_label_set_text(GTK_LABEL
                               (ctx->table_widgets
                                [(j * (ctx->tablecols + 2)) + i]),
                               bytestr);
        }
        /* add the ascii string */

        if (j < ctx->totalrows) {
            if (makeascii(asciistr,
                          ctx->buf + ctx->scroll_offset + nudge +
                          (j * ctx->cols * ctx->dsize),
                          ctx->bufsize - ctx->scroll_offset - nudge -
                          (j * ctx->cols * ctx->dsize),
                          ctx->cols * ctx->dsize) != 0) {
                FAIL_MSG("populate: makeascii() failed\n");
                return 4;
            }

        } else {
            asciistr[0] = 0x00;
        }

        /* set the label */

        gtk_label_set_text(GTK_LABEL
                           (ctx->table_widgets[(j * (ctx->tablecols + 2)) +
                                               ctx->tablecols + 1]),
                           asciistr);

    }
    /* fill in the hidden labels for nicer redraws */
    bytestr[0] = 0x00;
    for (j = ctx->rows; j < ctx->tablerows; j++) {
        for (i = 0; i < ctx->tablecols + 2; i++) {
            gtk_label_set_text(GTK_LABEL
                               (ctx->table_widgets
                                [(j * (ctx->tablecols + 2)) + i]),
                               bytestr);
        }
    }

    /* unmap the memory */
    if (ctx->type == BUF_TYPE_FD) {
        if (unmapmem(ctx) != 0) {
            FAIL_MSG("populate: unmapmem() failed\n");
            return 5;
        }

    }
    return 0;
}


/* hexdump_display starts the hexdump window */
int hexdump_display(void *rawctx)
{
    struct hd_ctx *ctx = (struct hd_ctx *) rawctx;
    GtkWidget *menubar;

    if (!ctx) {
        FAIL_MSG("hexdump_display: invalid params\n");
        return 1;
    }


    /* initialise values */
    ctx->scroll_offset = 0;
    ctx->hextable = NULL;
    ctx->table_widgets = NULL;

    /* create top window and set its name */
    ctx->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (!ctx->window) {
        FAIL_MSG("hexdump_display: gtk_window_new() failed\n");
        return 2;
    }

    gtk_window_set_title(GTK_WINDOW(ctx->window), "Hexdump");

    /* catch the destroy event */
    if (!g_signal_connect
        (ctx->window, "destroy", G_CALLBACK(destroy_local), ctx)) {
        FAIL_MSG("hexdump_display: g_signal_connect() failed\n");
        return 3;
    }


    /* set the resizing policy to accomodate shrinking */
    gtk_window_set_policy(GTK_WINDOW(ctx->window), TRUE, TRUE, TRUE);

    /* vbox to hold menu bar and hex table */
    ctx->vbox = gtk_vbox_new(FALSE, 0);
    if (!ctx->vbox) {
        FAIL_MSG("hexdump_display: gtk_vbox_new() failed\n");
        return 4;
    }

    gtk_container_add(GTK_CONTAINER(ctx->window), ctx->vbox);
    gtk_widget_show(ctx->vbox);

    /* menu bar */
    menubar = hd_menubar_menu(ctx);
    if (!menubar) {
        FAIL_MSG("hexdump_display: hd_menubar_menu() failed\n");
        return 5;
    }

    gtk_box_pack_start(GTK_BOX(ctx->vbox), menubar, FALSE, TRUE, 0);
    gtk_widget_show_all(menubar);

    /* hbox to contain hex table and scroll bar */
    ctx->hbox = gtk_hbox_new(FALSE, ctx->elexsize / 2);
    if (!ctx->hbox) {
        FAIL_MSG("hexdump_display: gtk_hbox_new() failed\n");
        return 6;
    }

    gtk_container_add(GTK_CONTAINER(ctx->vbox), ctx->hbox);
    gtk_widget_show(ctx->hbox);

    /* create event box to hold hex table */
    if (create_eventbox(ctx) != 0) {
        FAIL_MSG("hexdump_display: create_eventbox() failed\n");
        return 7;
    }


    /* set the dimensions for the table */
    if (set_dims(ctx) != 0) {
        FAIL_MSG("hexdump_display: set_dims() failed\n");
        return 8;
    }


    /* scroll bar */
    ctx->adj =
        gtk_adjustment_new(0.0, 0.0, (float) ctx->totalrows, 1.0,
                           (float) ctx->rows, (float) ctx->rows);
    if (!ctx->adj) {
        FAIL_MSG("hexdump_display: gtk_adjustment_new() failed\n");
        return 9;
    }

    ctx->scroll = gtk_vscrollbar_new(GTK_ADJUSTMENT(ctx->adj));
    if (!ctx->scroll) {
        FAIL_MSG("hexdump_display: gtk_vscrollbar_new() failed\n");
        return 10;
    }

    gtk_container_add(GTK_CONTAINER(ctx->hbox), ctx->scroll);
    gtk_widget_show(ctx->scroll);

    /* signal for scroll bar */
    if (!g_signal_connect(ctx->scroll, "value_changed",
                          G_CALLBACK(scroll_changed), ctx)) {
        FAIL_MSG("hexdump_display: g_signal_connect() failed\n");
        return 11;
    }


    /* signal for mouse scroll and resize events */
    gtk_widget_set_events(ctx->hexevent,
                          GDK_EXPOSURE_MASK | GDK_SCROLL_MASK);

    if (!g_signal_connect(ctx->hexevent, "scroll_event",
                          G_CALLBACK(scroll_event), ctx)) {
        FAIL_MSG("hexdump_display: g_signal_connect() failed\n");
        return 12;
    }


    if (!g_signal_connect(ctx->window, "configure_event",
                          G_CALLBACK(resize_event), ctx)) {
        FAIL_MSG("hexdump_display: g_signal_connect() failed\n");
        return 13;
    }


    /* make the initial table */
    if (create_and_populate_table(ctx) != 0) {
        FAIL_MSG("hexdump_display: create_and_populate_table() failed\n");
        return 14;
    }


    gtk_widget_show(ctx->window);

    return 0;
}


/* cleanup closes the shared memory */
int cleanup()
{
	if (shm_destroy) {
		if (rb_shm_close(shm_ctx, shm) != 0) {
			FAIL_MSG("cleanup: rb_shm_close() failed\n");
			return 1;
		}
	}
	
	return 0;
}


int main(int argc, char *argv[])
{
    struct hd_ctx *ctx = NULL;
    struct stat stattmp;

    if (argc != 2) {
        printf
            ("Usage: rb-hexdump shmpath|file\n\nVisualiser for Rubber Marbles\n");
        exit(1);
    }

    gtk_init(&argc, &argv);

    if (!strncmp(argv[1], "/rb_shm.", 8)) {

        /* argument is named shared memory buffer */
		shm_destroy = 0;
        if (shm_open_buffer(argv[1], &shm, &sem) != 0) {
            FAIL_MSG("main: shm_open_buffer() failed\n");
            return 2;
        }

    } else {

        /* argument is a file */
		shm_destroy = 1;
        if (rb_shm_init(&shm_ctx, &shm) != 0) {
            FAIL_MSG("main: rb_shm_init() failed\n");
            return 3;
        }

        if (stat(argv[1], &stattmp) != 0) {
            FAIL_ERR("main: stat() failed\n");
            return 4;
        }

        strncpy(shm->filename, argv[1], PATH_MAX - 1);
        shm->filename[PATH_MAX - 1] = 0x00;
        shm->offset = 0;
        shm->bufsize = stattmp.st_size;
        shm->buf_type = BUF_TYPE_FD;
        sem = shm_ctx->sem;

    }



    ctx = hexdump_init(600, 600, 200, 200);
    if (!ctx) {
        FAIL_MSG("main: hexdump_init() failed\n");
        return 5;
    }


    gctx = ctx;

    /* open the file */
    ctx->fd = open(shm->filename, O_RDONLY);
    if (ctx->fd == -1) {
        FAIL_ERR("main: cannot open file\n");
        return 6;
    }


    /* stat it */
    if (fstat(ctx->fd, &(ctx->filestat)) != 0) {
        FAIL_ERR("main: cannot stat file\n");
        return 7;
    }


    if (hexdump_filedesc(ctx, ctx->fd, &(ctx->filestat),
                         shm->offset, shm->bufsize,
                         1, HD_LITTLE_ENDIAN) != 0) {
        FAIL_MSG("main: hexdump_filedesc() failed\n");
        return 8;
    }


    if (hexdump_display(ctx) != 0) {
        FAIL_MSG("main: hexdump_display() failed\n");
        return 9;
    }


    if (enable_usr1() != 0) {
        FAIL_MSG("main: enable_usr1() failed\n");
        return 10;
    }


    if (signal(SIGHUP, sig_handler) == SIG_ERR) {
        FAIL_ERR("main: cannot catch SIGHUP\n");
        return 11;
    }


    if (g_idle_add(onIdle, ctx) <= 0) {
        FAIL_ERR("main: g_idle_add() failed\n");
        return 12;
    }


    gtk_main();

	cleanup();
    exit(0);


    return 0;
}
