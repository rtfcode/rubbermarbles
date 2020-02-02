/*
 * Rubber Marbles - K Sheldrake
 * rb-render.c
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


#include "rb-render.h"

/* shared memory access */
struct shm_buf *shm_ctx;
struct rb_shm *shm;
struct ren_ctx *gctx;
sem_t *sem;
int shm_destroy;

/* stored bitmap */
struct savepic save;

/* mmap context */
struct filemmap *mmap_ctx;


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
/*
		if (disable_usr1() != 0) {
            FAIL_MSG("sig_handler: disable_usr1() failed\n");
            return;
        }
*/
		
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
 * calls render_redraw() and enable_usr1() when it is set.
 */
gboolean onIdle(gpointer data)
{
    struct ren_ctx *ctx = (struct ren_ctx *) data;

    if (ctx->reload) {
        ctx->reload = 0;

        if (render_redraw(gctx) != 0) {
            FAIL_MSG("onIdle: render_redraw() failed\n");
            return TRUE;
        }

        if (enable_usr1() != 0) {
            FAIL_MSG("onIdle: enable_usr1() failed\n");
            return TRUE;
        }

    }

    return TRUE;
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
            && (callback_action <= COL_COLSCALE2)) {
            gctx->col_set = callback_action;
			if (render_redraw(gctx) != 0) {
				FAIL_MSG("change_col: render_redraw() failed\n");
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
            gctx->disp_hilbert = callback_action;
			if (render_redraw(gctx) != 0) {
				FAIL_MSG("change_col: render_redraw() failed\n");
				return;
			}

        }
    }
}


/* set_update is a menu callback.  It holds/unholds the updating of the
 * display */
void set_update(gpointer data, guint action, GtkWidget * widget)
{
    struct ren_ctx *ctx = (struct ren_ctx *) data;

    if (!ctx) {
        FAIL_MSG("set_update: invalid params\n");
        return;
    }


    /* check for valid endian */
    switch (action) {
    case UPDATE_CONT:
        enable_usr1();

        /* repopulate the array */
        if (render_redraw(ctx) != 0) {
            FAIL_MSG("set_update: render_redraw() failed\n");
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
GtkItemFactoryEntry ren_menu_items_hil[] = {
    {"/_File", NULL, NULL, 0, "<Branch>"}
    ,
    {"/File/_Quit", "<CTRL>Q", quit_local, 0, "<StockItem>", GTK_STOCK_QUIT}
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
    {"/Colours/ColScale2", NULL, change_col, COL_COLSCALE2,
	"/Colours/Cortesi"}
    ,
    {"/_Curve", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Curve/HilbertFlipped", NULL, change_display, DISP_HILBERTFLIPPED,
	"<RadioItem>"}
    ,
    {"/Curve/Hilbert", NULL, change_display, DISP_HILBERT,
	"/Curve/HilbertFlipped"}
    ,
    {"/_Update", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Update/Continuous", "c", set_update, UPDATE_CONT, "<RadioItem>"}
    ,
    {"/Update/Hold", "h", set_update, UPDATE_HOLD, "/Update/Continuous"}
    ,
};

GtkItemFactoryEntry ren_menu_items_shan[] = {
    {"/_File", NULL, NULL, 0, "<Branch>"}
    ,
    {"/File/_Quit", "<CTRL>Q", quit_local, 0, "<StockItem>", GTK_STOCK_QUIT}
    ,
    {"/_Colours", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Colours/ColScale2", NULL, change_col, COL_COLSCALE2,
	"<RadioItem>"}
	,
	{"/Colours/Cortesi", NULL, change_col, COL_CORTESI, 
	"/Colours/ColScale2"}
    ,
    {"/Colours/GreyScale", NULL, change_col, COL_GREYSCALE,
	"/Colours/ColScale2"}
    ,
    {"/Colours/ColScale", NULL, change_col, COL_COLSCALE,
	"/Colours/ColScale2"}
    ,
	{"/_Curve", NULL, NULL, 0, "<Branch>"}
	,
	{"/Curve/HilbertFlipped", NULL, change_display, DISP_HILBERTFLIPPED,
	"<RadioItem>"}
	,
	{"/Curve/Hilbert", NULL, change_display, DISP_HILBERT,
	"/Curve/HilbertFlipped"}
	,
	{"/_Update", NULL, NULL, 0, "<Branch>"}
    ,
    {"/Update/Continuous", "c", set_update, UPDATE_CONT, "<RadioItem>"}
    ,
    {"/Update/Hold", "h", set_update, UPDATE_HOLD, "/Update/Continuous"}
    ,
};


/* Returns a menubar widget made from the above menu */
GtkWidget *ren_menubar_menu(struct ren_ctx * ctx)
{
    GtkWidget *window;
    GtkItemFactory *item_factory;
    GtkAccelGroup *accel_group;
    gint nmenu_items;

    if (!ctx) {
        FAIL_MSG("ren_menubar_menu: invalid params\n");
        return NULL;
    }


    window = ctx->window;
    if ((sizeof(ren_menu_items_hil[0]) == 0) || (sizeof(ren_menu_items_shan[0]) == 0)) {
        FAIL_MSG("ren_menubar_menu: ren_menu_items[0] size is 0\n");
        return NULL;
    }


	if (ctx->buftype == REN_HILBERT) {
		nmenu_items = sizeof(ren_menu_items_hil) / sizeof(ren_menu_items_hil[0]);
	} else {
		nmenu_items = sizeof(ren_menu_items_shan) / sizeof(ren_menu_items_shan[0]);
	}
	

    /* Make an accelerator group (shortcut keys) */
    accel_group = gtk_accel_group_new();
    if (!accel_group) {
        FAIL_MSG("ren_menubar_menu: gtk_accel_group_new() failed\n");
        return NULL;
    }


    /* Make an ItemFactory (that makes a menubar) */
    item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>",
                                        accel_group);
    if (!item_factory) {
        FAIL_MSG("ren_menubar_menu: gtk_item_factory_new() failed\n");
        return NULL;
    }


    /* This function generates the menu items. Pass the item factory,
       the number of items in the array, the array itself, and any
       callback data for the the menu items. */
    if (ctx->buftype == REN_HILBERT) {
		gtk_item_factory_create_items(item_factory, nmenu_items, ren_menu_items_hil,
                                  ctx);
	} else {
		gtk_item_factory_create_items(item_factory, nmenu_items, ren_menu_items_shan,
									  ctx);
	}
	
	
    /* Attach the new accelerator group to the window. */
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

    /* Finally, return the actual menu bar created by the item factory. */
    return gtk_item_factory_get_widget(item_factory, "<main>");
}


/* quit function destroys the render window */
void quit_local(gpointer data, guint action, GtkWidget * widget)
{
    struct ren_ctx *ctx = (struct ren_ctx *) data;

    if (!ctx) {
        FAIL_MSG("quit_local: invalid params\n");
        return;
    }


    /* destroy the render window */
    gtk_widget_destroy(ctx->window);
}


/* destroy function signals the end of the render */
gboolean destroy_local(GtkWidget * widget, gpointer data)
{
    struct ren_ctx *ctx = (struct ren_ctx *) data;

    if (!ctx) {
        FAIL_MSG("destroy_local: invalid params\n");
        return TRUE;
    }


    gtk_widget_hide(ctx->window);

	cleanup();
    exit(0);

    return TRUE;
}


/* render_free free()s the memory used by render */
int render_free(void *rawctx)
{
    struct ren_ctx *ctx = (struct ren_ctx *) rawctx;

    if (!ctx) {
        FAIL_MSG("render_free: invalid params\n");
        return 1;
    }


    gtk_widget_destroy(ctx->hilbert);
    gtk_widget_destroy(ctx->vbox);
    gtk_widget_destroy(ctx->window);
    free(ctx);
    return 0;
}


/* configure_event creates a new backing pixmap of the appropriate size */
gboolean
configure_event(GtkWidget * widget, GdkEventConfigure * event,
                gpointer data)
{

    struct ren_ctx *ctx = (struct ren_ctx *)data;

    if (!widget || !ctx) {
        FAIL_MSG("configure_event: invalid params\n");
        return TRUE;
    }


    if (render_redraw(ctx) != 0) {
        FAIL_MSG("configure_event: render_redraw() failed\n");
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
	struct ren_ctx *ctx = (struct ren_ctx *)data;

    if (!widget || !ctx) {
        FAIL_MSG("expose_event: invalid params\n");
        return FALSE;
    }


    pb = ctx->pixbuf;

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


/* init_arrays sets the arrays to default values */
int init_arrays(struct ren_ctx *ctx)
{
	if (!ctx) {
		FAIL_MSG("init_arrays: invalid params\n");
		return 1;
	}
	
	
    ctx->col_set = COL_CORTESI;
    ctx->disp_hilbert = DISP_HILBERTFLIPPED;
	ctx->pixbuf = NULL;

    /* clear the savepic struct */
	save.pic = NULL;
	save.col_set = -1;
	save.disp_hilbert = -1;
	save.start = -2;
	save.end = -2;

    return 0;
}




/* render_init initialises the context */
void *render_init(unsigned int xsize, unsigned int ysize, unsigned int x,
                   unsigned int y)
{
    struct ren_ctx *ctx;

    if (!xsize || !ysize) {
        FAIL_MSG("render_init: invalid params\n");
        return NULL;
    }


    ctx = (struct ren_ctx *) malloc(sizeof(struct ren_ctx));
    if (!ctx) {
        FAIL_ERR("render_init: malloc() failed\n");
        return NULL;
    }


    memset(ctx, 0, sizeof(struct ren_ctx));

    ctx->xsize = xsize;
    ctx->ysize = ysize;
    ctx->xpos = x;
    ctx->ypos = y;

    ctx->reload = 0;
	
	init_arrays(ctx);
	build_shannon_lookup();
	
	
	mmap_ctx = rb_init_mmap();

    return (void *) ctx;
}


/* render_filedesc sets up the filedesc buffer details */
int
render_filedesc(void *rawctx, int fd, struct stat *filestat,
                 unsigned long offset, unsigned long size,
				 unsigned int buftype)
{
    struct ren_ctx *ctx = (struct ren_ctx *) rawctx;

    if (!rawctx || !fd || !filestat || !size) {
        FAIL_MSG("render_filedesc: invalid params\n");
        return 1;
    }


    if (copyshm(ctx) != 0) {
        FAIL_MSG("render_filedesc: copyshm() failed\n");
        return 2;
    }

	ctx->buftype = buftype;
	
    return 0;
}


/* render_redraw indicates the shared memory values have changed */
int render_redraw(void *rawctx)
{
    struct ren_ctx *ctx = (struct ren_ctx *) rawctx;

    if (!ctx) {
        FAIL_MSG("render_redraw: invalid params\n");
        return 1;
    }


    /* copy the shared memory values */
    if (copyshm(ctx) != 0) {
        FAIL_MSG("render_redraw: copyshm() failed\n");
        return 2;
    }

	
    /* repopulate */
    if (draw_img(ctx) != 0) {
        FAIL_MSG("render_redraw: draw_img() failed\n");
        return 4;
    }

		
	/* and update it */
    gtk_widget_queue_draw_area(ctx->hilbert, 0, 0, ctx->xsize, ctx->ysize);

    return 0;
}


/* copyshm copies values from the shared memory via a semaphore */
int copyshm(struct ren_ctx *ctx)
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


/* render_display starts the render window */
int render_display(void *rawctx)
{
    struct ren_ctx *ctx = (struct ren_ctx *) rawctx;
    GtkWidget *menubar;

    if (!ctx) {
        FAIL_MSG("render_display: invalid params\n");
        return 1;
    }


    /* create top window and set its name */
    ctx->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (!ctx->window) {
        FAIL_MSG("render_display: gtk_window_new() failed\n");
        return 2;
    }

	if (ctx->buftype == REN_HILBERT) {
		gtk_window_set_title(GTK_WINDOW(ctx->window), "Hilbert");
	} else if (ctx->buftype == REN_SHANNON) {
		gtk_window_set_title(GTK_WINDOW(ctx->window), "Shannon Entropy");
	} else {
		FAIL_MSG("render_display: invalid buftype\n");
		ctx->buftype = REN_HILBERT;
		gtk_window_set_title(GTK_WINDOW(ctx->window), "Hilbert");
	}
	
    /* catch the destroy event */
    if (!g_signal_connect
        (ctx->window, "destroy", G_CALLBACK(destroy_local), ctx)) {
        FAIL_MSG("render_display: g_signal_connect() failed\n");
        return 3;
    }


    /* set the resizing policy to disallow resizing */
    gtk_window_set_policy(GTK_WINDOW(ctx->window), FALSE, FALSE, TRUE);

    /* vbox to hold menu bar and hex table */
    ctx->vbox = gtk_vbox_new(FALSE, 0);
    if (!ctx->vbox) {
        FAIL_MSG("render_display: gtk_vbox_new() failed\n");
        return 4;
    }

    gtk_container_add(GTK_CONTAINER(ctx->window), ctx->vbox);
    gtk_widget_show(ctx->vbox);

    /* menu bar */
    menubar = ren_menubar_menu(ctx);
    if (!menubar) {
        FAIL_MSG("render_display: ren_menubar_menu() failed\n");
        return 5;
    }

    gtk_box_pack_start(GTK_BOX(ctx->vbox), menubar, FALSE, TRUE, 0);
    gtk_widget_show_all(menubar);

    ctx->hilbert = gtk_drawing_area_new();
    if (!ctx->hilbert) {
        FAIL_MSG("render_display: gtk_drawing_area_new() failed\n");
        return 6;
    }

    gtk_widget_set_size_request(GTK_WIDGET(ctx->hilbert), ctx->xsize, ctx->ysize);

    gtk_box_pack_start(GTK_BOX(ctx->vbox), ctx->hilbert, TRUE, TRUE, 0);
    gtk_widget_show(ctx->hilbert);

    /* Signals used to handle backing pixmap */
    if (!g_signal_connect(ctx->hilbert, "expose_event",
                          G_CALLBACK(expose_event),
                          (void *) ctx)) {
        FAIL_MSG("render_display: g_signal_connect() failed\n");
        return 7;
    }

    if (!g_signal_connect(ctx->hilbert, "configure_event",
                          G_CALLBACK(configure_event),
                          (void *) ctx)) {
        FAIL_MSG("render_display: g_signal_connect() failed\n");
        return 8;
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
    struct ren_ctx *ctx = NULL;
    struct stat stattmp;
	char *ptr = NULL;

	if (argc <= 0) {
		printf
			("Usage: rb-render shmpath|file\n\nVisualiser for Rubber Marbles\n");
        exit(1);
    } else if (argc != 2) {
        printf
            ("Usage: %s shmpath|file\n\nVisualiser for Rubber Marbles\n",
					argv[0]);
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


    ptr = strrchr(argv[0], '/');
    if (!ptr) {
        ptr = argv[0];
    } else {
        ptr++;
    }

    ctx = render_init(256, 256, 200, 200);
    if (!ctx) {
        FAIL_MSG("main: render_init() failed\n");
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


	if (strncmp(ptr, "rb-render", 10) == 0) {
		if (render_filedesc(ctx, ctx->fd, &(ctx->filestat),
							 shm->offset, shm->bufsize, REN_HILBERT) != 0) {
			FAIL_MSG("main: render_filedesc() failed\n");
			return 8;
		}
	} else if (strncmp(ptr, "rb-shannon", 11) == 0) {
		ctx->col_set = COL_COLSCALE2;
		if (render_filedesc(ctx, ctx->fd, &(ctx->filestat),
							shm->offset, shm->bufsize, REN_SHANNON) != 0) {
			FAIL_MSG("main: render_filedesc() failed\n");
			return 8;
		}
	} else {

        FAIL_MSG("unknown program name\n");

        exit(1);
    }
	

    if (render_display(ctx) != 0) {
        FAIL_MSG("main: render_display() failed\n");
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

    return 0;
}
