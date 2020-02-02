/*
 * Rubber Marbles - K Sheldrake
 * rb-ren-draw.c
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 *
 * Provides functions for drawing the hilbert displays.
 */


#include "rb-ren-draw.h"

extern struct filemmap *mmap_ctx;
extern struct savepic save;
float shannon_lookup[SHANNON_BS+1];

/* colscalecolbyte sets the colour values based on the byte b */
int colscalecolbyte(int b, guchar * red, guchar * green, guchar * blue)
{
    if (!red || !green || !blue) {
        FAIL_MSG("colscalecolbyte: invalid params\n");
        return 1;
    }


    int col = b % 256;

    *red = col * COLSCALEREDF;
    *green = col * COLSCALEGREENF;
    *blue = col * COLSCALEBLUEF;

    return 0;
}


/* colscalecolbyte sets the colour values based on the byte b */
int colscalecolbyte2(int b, guchar * red, guchar * green, guchar * blue)
{
    float e, v, r, bf;
	
	
	if (!red || !green || !blue) {
        FAIL_MSG("colscalecolbyte2: invalid params\n");
        return 1;
    }


    int col = b % 256;

	e = (float)col / 255;
	
	if (e > 0.5) {
		v = e - 0.5;
		r = powf((4.0 * v) - (4.0 * v * v), 4.0);
		if (r < 0.0) {
			r = 0.0;
		}
	} else {
		r = 0.0;
	}
	
	bf = e * e;
	
	
	
    *red = (int)(255 *r);
    *green = 0;
    *blue = (int)(255 * bf);

    return 0;
}


/* greyscalecolbyte sets the colour values based on the byte b */
int greyscalecolbyte(int b, guchar * red, guchar * green, guchar * blue)
{
    if (!red || !green || !blue) {
        FAIL_MSG("greyscalecolbyte: invalid params\n");
        return 1;
    }


    int d = b % 256;

    *red = d;
    *green = d;
    *blue = d;

    return 0;
}


/* cortesicolbyte sets the colour values based on the byte b.
Cortesi colouring is: 0x00 black, 0xff white, ascii blue,
low green, high red. */
int cortesicolbyte(int b, guchar * red, guchar * green, guchar * blue)
{
    if (!red || !green || !blue) {
        FAIL_MSG("cortesicolbyte: invalid params\n");
        return 1;
    }


    int d = b % 256;

    if (d == 0x00) {
        /* 0x00 byte is black */
        *red = 0x00;
        *green = 0x00;
        *blue = 0x00;
    } else if (d == 0xff) {
        /* 0xff byte is white */
        *red = 0xff;
        *green = 0xff;
        *blue = 0xff;
    } else if (((d >= 0x20) && (d < 0x7f)) || (d == 0x0a) || (d == 0x0d)
               || (d == 0x09)) {
        /* ascii is blue */
        *red = CORTESIASCIIR;
        *green = CORTESIASCIIG;
        *blue = CORTESIASCIIB;
    } else if ((d >= 0) && (d < 0x20)) {
        /* low is green */
        *red = CORTESILOWR;
        *green = CORTESILOWG;
        *blue = CORTESILOWB;
    } else {
        /* high is red */
        *red = CORTESIHIGHR;
        *green = CORTESIHIGHG;
        *blue = CORTESIHIGHB;
    }

    return 0;
}


/* build_shannon_lookup fills in the shannon lookup table */
void build_shannon_lookup()
{
	unsigned int i;
	float p;
	
	float logbase = log(SHANNON_BS);
	
	shannon_lookup[0] = 0.0;
	
	for (i=1; i<=SHANNON_BS; i++) {
		p = (float) i / SHANNON_BS;
		shannon_lookup[i] = (p * log(p) / logbase);
	}
}


/* shannon_char returns an unsigned char representation of the Shannon entropy
 * of the 32 byte buffer centred on the data_index.
 * This is reasonably quick because it uses a lookup table for the entropy values. */
unsigned char shannon_char(unsigned char *buf, unsigned long bufsize, unsigned long data_index)
{
	unsigned long start;
	unsigned char count[256];
	unsigned long i;
	float entropy = 0.0;	
	
	if (!buf || !bufsize || data_index >= bufsize || SHANNON_BS >= bufsize) {
		FAIL_MSG("shannon_char: invalid params\n");
		return 0;
	}
	
	/* init array */
	memset(count, 0, 256);
	
	/* calc start of block to measure */
	if (data_index < (SHANNON_BS / 2)) {
		start = 0;
	} else if (data_index > (bufsize - (SHANNON_BS / 2))) {
		start = bufsize - SHANNON_BS;
	} else {
		start = data_index - (SHANNON_BS / 2);
	}
	
	/* count byte frequency */
	for (i=start; i<start + SHANNON_BS; i++) {
		count[buf[i]]++;
	}
	
	
	/* calculate entropy  - see Cortesi scurve util.py for details */
	entropy = 0.0;
	for (i=0; i<256; i++) {
		if (count[i]) {
			entropy += shannon_lookup[count[i]];
		}
	}

	return (unsigned char)(entropy * -255);
}
	



/* plot_point draws a point onto the rgb bitmap */
int
plot_point(struct ren_ctx *ctx, struct rgb *pic, int width, int height, int x, int y,
           int colraw)
{
    if (!ctx || !pic || !width || !height || (x < 0) || (x >= width) || (y < 0)
        || (y >= height)) {
        FAIL_MSG("plot_point: invalid params\n");
        return 1;
    }


    int col = colraw % 256;
    guchar colred, colgreen, colblue;

    switch (ctx->col_set) {
    case COL_CORTESI:
        if (cortesicolbyte(col, &colred, &colgreen, &colblue) != 0) {
            FAIL_MSG("plot_point: cortesicolbyte() failed\n");
            return 2;
        }

        break;
    case COL_GREYSCALE:
        if (greyscalecolbyte(col, &colred, &colgreen, &colblue) != 0) {
            FAIL_MSG("plot_point: greyscalecolbyte() failed\n");
            return 3;
        }

        break;
    case COL_COLSCALE:
        if (colscalecolbyte(col, &colred, &colgreen, &colblue) != 0) {
            FAIL_MSG("plot_point: colscalecolbyte() failed\n");
            return 4;
        }

        break;
    case COL_COLSCALE2:
        if (colscalecolbyte2(col, &colred, &colgreen, &colblue) != 0) {
            FAIL_MSG("plot_point: colscalecolbyte() failed\n");
            return 4;
        }

        break;
    default:
        fprintf(stderr, "plot_point: invalid col_set");
        return 5;
    }

    pic[(width * y) + x].red = colred;
    pic[(width * y) + x].green = colgreen;
    pic[(width * y) + x].blue = colblue;

    return 0;
}


/* freepic is a callback that frees the data when the pixbuf is destroyed.
 * It has to be void to meet the fn spec */
void freepic(guchar * pixels, gpointer data)
{
    if (!pixels) {
        FAIL_MSG("freepic: pixels is NULL\n");
        return;
    }


    free(pixels);
}


/* getxy finds the coords in a pixbuf given a point index */
int getxy(struct ren_ctx *ctx, int width, int point_index, int *x, int *y)
{
    if (!ctx || !width || !x || !y) {
        FAIL_MSG("getxy: invalid params\n");
        return 1;
    }


    /* find the location */
	if (ctx->disp_hilbert == DISP_HILBERTFLIPPED) {
		if (d2xy(width, point_index, y, x) != 0) {
			FAIL_MSG("getxy: d2xy() failed\n");
			return 2;
		}

	} else {
		if (d2xy(width, point_index, x, y) != 0) {
			FAIL_MSG("getxy: d2xy() failed\n");
			return 3;
		}

	}

    return 0;
}


/* draw_img draws a hilbert or zigzag pixbuf */
int draw_img(struct ren_ctx *ctx)
{
    struct rgb *pic = NULL;
    int x, y;
    long data_start, point_index;
    float data_index;
    int drawsize;
    float step;
	unsigned char value;
	int width, height;

    if (!ctx) {
        FAIL_MSG("draw_img: invalid params\n");
        return 1;
    }


    width = ctx->xsize;
    height = ctx->ysize;

    /* create a rgb bitmap */
    pic = (struct rgb *) malloc(sizeof(struct rgb) * ctx->xsize * ctx->ysize);
    if (!pic) {
        FAIL_MSG("draw_img: malloc() failed\n");
        return 2;
    }


    /* set the pixmap to black */
    memset(pic, 0, sizeof(struct rgb) * width * height);

    /* initialise some vars */
    data_start = ctx->offset;
    point_index = 0;
    drawsize = width * height;

    /* set step */
	step = (float)ctx->bufsize / drawsize;
	

    /* draw it */

	/* initialise the data index */
	data_index = data_start;

	/* this is the draw loop - it runs until we run out of input or we fill
	   the box */
	while ((data_index < ctx->offset + ctx->bufsize)
		   && (point_index < drawsize)) {

		if (!mmap_ctx->ptr || (data_index < mmap_ctx->offset) || (data_index > mmap_ctx->offset + mmap_ctx->size)) {
			/* need to mmap a chunk */
			if (rb_mmap(mmap_ctx, ctx->fd, (unsigned long)data_index, ctx->filestat.st_size) != 0) {
				FAIL_MSG("rb_mmap() failed\n");
				return 3;
			}
			ctx->buf = mmap_ctx->ptr;
		}
		
		
		/* find the location */
		if (getxy(ctx, width, point_index, &x, &y) != 0) {
			FAIL_MSG("draw_img: getxy() failed\n");
			return 4;
		}


		/* get data value */
		if (ctx->buftype == REN_HILBERT) {
			value = ctx->buf[(long)data_index - mmap_ctx->offset];
		} else if (ctx->buftype == REN_SHANNON) {
			value = shannon_char(ctx->buf, mmap_ctx->size, (long)data_index - mmap_ctx->offset);
		} else {
			FAIL_MSG("draw_img: invalid buftype\n");
			value = ctx->buf[(long)data_index - mmap_ctx->offset];
		}
		
		/* and plot it */
		if (plot_point(ctx, pic, width, height, x, y,
					   value) != 0) {
			FAIL_MSG("draw_img: plot_point() failed\n");
			return 5;
		}


		/* update counters */
		point_index++;
		data_index = data_start + (point_index * step);
	}


    /* copy the bitmap to the pixbuf */
    if (ctx->pixbuf)
        g_object_unref(ctx->pixbuf);

    ctx->pixbuf = gdk_pixbuf_new_from_data((const guchar *) pic,        /* the data */
                                                       GDK_COLORSPACE_RGB,        /* colorspace */
                                                       FALSE,        /* has alpha */
                                                       8,        /* bits per sample */
                                                       width,        /* width */
                                                       height,        /* height */
                                                       width * 3,        /* rowstride */
                                                       &freepic,        /* destroy fn */
                                                       NULL);        /* destroy data */

    return 0;
}
