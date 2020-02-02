/*
 * Rubber Marbles - K Sheldrake
 * rb-draw.c
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 *
 * Provides functions for drawing the hilbert and zigzag displays.
 */


#include "rb-draw.h"

extern struct filemmap mmap_ctx;
extern struct rb_shm *shm;
extern struct pixbuf displays[5];
extern struct window zoom[2];
extern struct savepic save[5];
extern struct displayset disp;


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


/* plot_point draws a point onto the rgb bitmap */
int
plot_point(struct rgb *pic, int width, int height, int x, int y,
           int colraw)
{
    if (!pic || !width || !height || (x < 0) || (x >= width) || (y < 0)
        || (y >= height)) {
        FAIL_MSG("plot_point: invalid params\n");
        return 1;
    }


    int col = colraw % 256;
    guchar colred, colgreen, colblue;

    switch (disp.col_set) {
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
    default:
        fprintf(stderr, "plot_point: invalid col_set");
        return 5;
    }

    pic[(width * y) + x].red = colred;
    pic[(width * y) + x].green = colgreen;
    pic[(width * y) + x].blue = colblue;

    return 0;
}


/* calcwindows finds the absolute values for the window starts and ends */
int calcwindows(unsigned long *wholestart, unsigned long *wholeend,
                unsigned long *zoomstart, unsigned long *zoomend)
{
    if (!wholestart || !wholeend || !zoomstart || !zoomend) {
        FAIL_MSG("calcwindows: invalid params\n");
        return 1;
    }


    if (zoom[ZOOM_WHOLE].start != -1) {
        *wholestart = zoom[ZOOM_WHOLE].start;
    } else {
        *wholestart = 0;
    }

    if (zoom[ZOOM_WHOLE].end != -1) {
        *wholeend = zoom[ZOOM_WHOLE].end;
    } else {
        *wholeend = shm->filestat.st_size;
    }

    if (zoom[ZOOM_ZOOM].start != -1) {
        *zoomstart = zoom[ZOOM_ZOOM].start;
    } else {
        *zoomstart = *wholestart;
    }
    if (zoom[ZOOM_ZOOM].end != -1) {
        *zoomend = zoom[ZOOM_ZOOM].end;
    } else {
        *zoomend = *wholeend;
    }

    return 0;
}

/* plot_markers draws the green and red markers on the thin pixbuf
 * between the two zigzag plots. */
int
plot_markers(struct rgb *pic, int width, int height, int pixbufnum,
             int marker, int x)
{
    int i, y;
    unsigned long wholestart, wholeend, zoomstart, zoomend;

    if (!pic || !width || !height) {
        FAIL_MSG("plot_markers: invalid params\n");
        return 1;
    }


    if (calcwindows(&wholestart, &wholeend, &zoomstart, &zoomend) != 0) {
        FAIL_MSG("plot_markers: calcwindows() failed\n");
        return 2;
    }


    /* calc y position for the start marker */
    if (pixbufnum == PIXBUF_WHOLE_ZIGZAG) {
        y = wholestart / (displays[pixbufnum].width *
                          zoom[marker].step_zigzag);
    } else {
        y = (zoomstart -
             wholestart) / (displays[pixbufnum].width *
                            zoom[marker].step_zigzag);
    }

    /* draw start marker */
    if ((y >= 0) && (y < displays[pixbufnum].height)) {
        for (i = 0; i < 3; i++) {
            pic[(width * y) + x + i].red = 0x00;
            pic[(width * y) + x + i].green = 0xff;
            pic[(width * y) + x + i].blue = 0x00;
        }
    }

    /* calc y position for the end marker */
    if (pixbufnum == PIXBUF_WHOLE_ZIGZAG) {
        y = wholeend / (displays[pixbufnum].width *
                        zoom[marker].step_zigzag);
    } else {
        y = (zoomend -
             wholestart) / (displays[pixbufnum].width *
                            zoom[marker].step_zigzag);
    }

    /* draw end marker */
    if ((y >= 0) && (y < displays[pixbufnum].height)) {
        for (i = 0; i < 3; i++) {
            pic[(width * y) + x + i].red = 0xff;
            pic[(width * y) + x + i].green = 0x00;
            pic[(width * y) + x + i].blue = 0x00;
        }
    }

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
int getxy(int pixbufnum, int width, int point_index, int *x, int *y)
{
    if (!width || !x || !y) {
        FAIL_MSG("getxy: invalid params\n");
        return 1;
    }


    /* find the location */
    switch (pixbufnum) {
    case PIXBUF_WHOLE_HILBERT:
    case PIXBUF_ZOOM_HILBERT:
        if (disp.disp_hilbert == DISP_HILBERTFLIPPED) {
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
        break;
    case PIXBUF_WHOLE_ZIGZAG:
    case PIXBUF_ZOOM_ZIGZAG:
        if (disp.disp_zigzag == DISP_LINEAR) {
            *y = point_index / width;
            *x = point_index % width;
        } else {
            *y = point_index / width;
            if ((*y % 2) == 0) {
                *x = point_index % width;
            } else {
                *x = width - (point_index % width) - 1;
            }
        }
        break;
    default:
        fprintf(stderr, "getxy: invalid pixbufnum %d\n", pixbufnum);
        return 4;
    }

    return 0;
}


/* highlight_point adds a highlight to an existing RGB point */
int highlight_point(struct rgb *pic, int width, int x, int y)
{
    if (!pic || !width || (x >= width)) {
        FAIL_MSG("highlight_point: invalid params\n");
        return 1;
    }


    switch (disp.col_set) {
    case COL_CORTESI:
        switch (pic[(width * y) + x].red) {
        case 0:
            pic[(width * y) + x].red = CORTESIHL_BLACKR;
            pic[(width * y) + x].green = CORTESIHL_BLACKG;
            pic[(width * y) + x].blue = CORTESIHL_BLACKB;
            break;
        case 0xff:
            pic[(width * y) + x].red = CORTESIHL_WHITER;
            pic[(width * y) + x].green = CORTESIHL_WHITEG;
            pic[(width * y) + x].blue = CORTESIHL_WHITEB;
            break;
        case CORTESIASCIIR:
            pic[(width * y) + x].red = CORTESIHL_ASCIIR;
            pic[(width * y) + x].green = CORTESIHL_ASCIIG;
            pic[(width * y) + x].blue = CORTESIHL_ASCIIB;
            break;
        case CORTESILOWR:
            pic[(width * y) + x].red = CORTESIHL_LOWR;
            pic[(width * y) + x].green = CORTESIHL_LOWG;
            pic[(width * y) + x].blue = CORTESIHL_LOWB;
            break;
        case CORTESIHIGHR:
            pic[(width * y) + x].red = CORTESIHL_HIGHR;
            pic[(width * y) + x].green = CORTESIHL_HIGHG;
            pic[(width * y) + x].blue = CORTESIHL_HIGHB;
            break;
        default:
            fprintf(stderr,
                    "highlight_point: unexpected colour in cortesi highlight\n");
            return 2;
        }
        break;
    case COL_GREYSCALE:
    case COL_COLSCALE:
        /* bias the blue to distinguish colour from unhighlighted pixels */
        if (pic[(width * y) + x].blue > COLSCALETHRESHOLD) {
            pic[(width * y) + x].blue = 0;
        } else {
            pic[(width * y) + x].blue = COLSCALEHL;
        }
        break;
    default:
        perror("highlight_point: invalid col_set");
        return 3;
    }

    return 0;
}


/* add_highlight highlights the points within the window selection */
int
add_highlight(int pixbufnum, int windownum, struct rgb *pic,
              int width, int height, long data_start, float step)
{
    int point_index;
    int point_start;
    int point_end;
    int x, y;
    int drawsize;
    long winstart, winend;
    unsigned long wholestart, wholeend, zoomstart, zoomend;

    if (!pic || !width || !height || (step == 0.0)) {
        FAIL_MSG("add_highlight: invalid params\n");
        return 1;
    }


    /* check if nothing is highlighted */
    if ((zoom[windownum].start == -1) && (zoom[windownum].end == -1)) {
        return 0;
    }

    if (calcwindows(&wholestart, &wholeend, &zoomstart, &zoomend) != 0) {
        FAIL_MSG("add_highlight: calcwindows() failed\n");
        return 2;
    }


    /* set the selection window dims */
    if (windownum == ZOOM_WHOLE) {
        winstart = wholestart;
        winend = wholeend;
    } else {
        winstart = zoomstart;
        winend = zoomend;
    }

    /* calc the indices point_start and point_end */
    drawsize = width * height;
    if (winstart >= data_start) {
        point_start = (winstart - data_start) / step;
    } else {
        point_start = 0;
    }
    if (winend >= data_start) {
        point_end = (winend - data_start) / step;
    } else {
        return 0;
    }
    if (point_start > drawsize) {
        return 0;
    }
    if (point_end > drawsize) {
        point_end = drawsize;
    }

    /* loop over the selection */
    for (point_index = point_start; point_index < point_end; point_index++) {
        /* find the location */
        if (getxy(pixbufnum, width, point_index, &x, &y) != 0) {
            FAIL_MSG("add_highlight: getxy() failed\n");
            return 3;
        }


        /* highlight the point */
        if (highlight_point(pic, width, x, y) != 0) {
            FAIL_MSG("add_highlight: highlight_point() failed\n");
            return 4;
        }

    }

    return 0;
}


/* draw_img draws a hilbert or zigzag pixbuf */
int draw_img(int pixbufnum)
{
    struct rgb *pic = NULL;
    int x, y;
    long data_start, point_index;
    float data_index;
    int drawsize;
    int windownum;
    float step;
    unsigned long wholestart;

    if ((pixbufnum != PIXBUF_WHOLE_HILBERT) &&
        (pixbufnum != PIXBUF_WHOLE_ZIGZAG) &&
        (pixbufnum != PIXBUF_WIN) &&
        (pixbufnum != PIXBUF_ZOOM_ZIGZAG) &&
        (pixbufnum != PIXBUF_ZOOM_HILBERT)) {
        FAIL_MSG("draw_img: invalid params\n");
        return 1;
    }


    int width = displays[pixbufnum].width;
    int height = displays[pixbufnum].height;

    /* find the start of the whole window selection */
    if (zoom[ZOOM_WHOLE].start != -1) {
        wholestart = zoom[ZOOM_WHOLE].start;
    } else {
        wholestart = 0;
    }

    /* create a rgb bitmap */
    pic = (struct rgb *) malloc(sizeof(struct rgb) * width * height);
    if (!pic) {
        FAIL_MSG("draw_img: malloc() failed\n");
        return 2;
    }


    /* set the pixmap to black */
    memset(pic, 0, sizeof(struct rgb) * width * height);

    /* initialise some vars */
    data_start = 0;
    if ((pixbufnum == PIXBUF_ZOOM_HILBERT)
        || (pixbufnum == PIXBUF_ZOOM_ZIGZAG)) {
        data_start = wholestart;
    }
    point_index = 0;
    drawsize = width * height;

    /* set window number and step */
    switch (pixbufnum) {
    case PIXBUF_WHOLE_HILBERT:
        windownum = ZOOM_WHOLE;
        step = zoom[windownum].step_hilbert;
        break;
    case PIXBUF_WHOLE_ZIGZAG:
        windownum = ZOOM_WHOLE;
        step = zoom[windownum].step_zigzag;
        break;
    case PIXBUF_WIN:
        windownum = -1;
        break;
    case PIXBUF_ZOOM_HILBERT:
        windownum = ZOOM_ZOOM;
        step = zoom[windownum].step_hilbert;
        break;
    case PIXBUF_ZOOM_ZIGZAG:
        windownum = ZOOM_ZOOM;
        step = zoom[windownum].step_zigzag;
        break;
    default:
        fprintf(stderr, "draw_img: invalid pixbufnum %d\n", pixbufnum);
        return 3;
    }

    /* draw it
     * if the pixbuf is a hilbert or zigzag and there isn't a saved bitmap
     * or something has changed that invalidates the bitmap, then redraw
     * it and highlight it;
     * otherwise if it's a hilbert or zigzag and there is a saved bitmap,
     * draw that and highlight it;
     * otherwise if PIXBUF_WIN, draw the selection window markers.
     */
    if ((pixbufnum != PIXBUF_WIN)
        && ((disp.col_set != save[pixbufnum].col_set)
            ||
            (((pixbufnum == PIXBUF_WHOLE_HILBERT)
              || (pixbufnum == PIXBUF_ZOOM_HILBERT))
             && (disp.disp_hilbert != save[pixbufnum].disp_hilbert))
            ||
            (((pixbufnum == PIXBUF_WHOLE_ZIGZAG)
              || (pixbufnum == PIXBUF_ZOOM_ZIGZAG))
             && (disp.disp_zigzag != save[pixbufnum].disp_zigzag))
            || !(save[pixbufnum].pic)
            || ((windownum == ZOOM_ZOOM)
                && ((save[pixbufnum].start != zoom[ZOOM_WHOLE].start)
                    || (save[pixbufnum].end != zoom[ZOOM_WHOLE].end))))) {
        /* check if the current mmap chunk is at least before our target */
        if (data_start < mmap_ctx.mmap_offset) {
            /* invalidate the chunk */
            mmap_ctx.mmap_offset = -(mmap_ctx.mmap_size);
        }

        /* initialise the data index */
        data_index = data_start;

        /* this is the draw loop - it runs until we run out of input or we fill
           the box */
        while ((data_index < shm->filestat.st_size)
               && (point_index < drawsize)) {

            /* see if we need to mmap */
            while (data_index >=
                   (mmap_ctx.mmap_offset + mmap_ctx.mmap_size)) {
                /* if we already have a chunk, then unmap it */
                if (mmap_ctx.filedata) {
                    if (munmap(mmap_ctx.filedata, mmap_ctx.mmap_size) != 0) {
                        FAIL_ERR("draw_img: munmap() failed\n");
                        return 4;
                    }

                }

                /* calculate new chunk params */
                mmap_ctx.mmap_offset =
                    mmap_ctx.mmap_offset + mmap_ctx.mmap_size;
                mmap_ctx.mmap_size = (MMAP_CHUNK_SIZE);
                if ((mmap_ctx.mmap_offset + mmap_ctx.mmap_size) >
                    shm->filestat.st_size) {
                    mmap_ctx.mmap_size =
                        shm->filestat.st_size - mmap_ctx.mmap_offset;
                }

                /* mmap it */
                mmap_ctx.filedata =
                    mmap((caddr_t) 0, mmap_ctx.mmap_size, PROT_READ,
                         MAP_SHARED, shm->fd, mmap_ctx.mmap_offset);
                if (mmap_ctx.filedata == MAP_FAILED) {
                    FAIL_MSG("draw_img: mmap() failed\n");
                    return 5;
                }

            }

            /* find the location */
            if (getxy(pixbufnum, width, point_index, &x, &y) != 0) {
                FAIL_MSG("draw_img: getxy() failed\n");
                return 6;
            }


            /* and plot it */
            if (plot_point(pic, width, height, x, y,
                           mmap_ctx.filedata[(long)
                                             (data_index -
                                              mmap_ctx.mmap_offset)]) !=
                0) {
                FAIL_MSG("draw_img: plot_point() failed\n");
                return 6;
            }


            /* update counters */
            point_index++;
            data_index = data_start + (point_index * step);
        }

        /* save a copy of the pic for future use */

        /* free the old one */
        if (save[pixbufnum].pic) {
            free(save[pixbufnum].pic);
        }

        /* create the new one */
        save[pixbufnum].pic =
            (struct rgb *) malloc(sizeof(struct rgb) * width * height);
        if (!(save[pixbufnum].pic)) {
            FAIL_ERR("draw_img: malloc() failed\n");
            return 7;
        }


        /* copy the bitmap */
        memcpy(save[pixbufnum].pic, pic,
               sizeof(struct rgb) * width * height);

        /* store the parameters */
        if (windownum == ZOOM_ZOOM) {
            save[pixbufnum].start = zoom[ZOOM_WHOLE].start;
            save[pixbufnum].end = zoom[ZOOM_WHOLE].end;
        }
        save[pixbufnum].col_set = disp.col_set;
        save[pixbufnum].disp_hilbert = disp.disp_hilbert;
        save[pixbufnum].disp_zigzag = disp.disp_zigzag;

        /* and highlight it */
        if (add_highlight
            (pixbufnum, windownum, pic, width, height, data_start,
             step) != 0) {
            FAIL_MSG("draw_img: add_highlight() failed\n");
            return 8;
        }


    } else if (pixbufnum != PIXBUF_WIN) {
        /* window dimensions haven't changed, so just copy the old one and highlight it */
        memcpy(pic, save[pixbufnum].pic,
               sizeof(struct rgb) * width * height);
        if (add_highlight
            (pixbufnum, windownum, pic, width, height, data_start,
             step) != 0) {
            FAIL_MSG("draw_img: add_highlight failed\n");
            return 9;
        }


    } else {
        /* this is the PIXBUF_WIN between the two zigzags */
        if (plot_markers
            (pic, width, height, PIXBUF_WHOLE_ZIGZAG, ZOOM_WHOLE,
             0) != 0) {
            FAIL_MSG("draw_img: plot_markers() failed\n");
            return 10;
        }

        if (plot_markers
            (pic, width, height, PIXBUF_ZOOM_ZIGZAG, ZOOM_ZOOM, 5) != 0) {
            FAIL_MSG("draw_img: plot_markers() failed\n");
            return 11;
        }

    }

    /* copy the bitmap to the pixbuf */
    if (displays[pixbufnum].buf)
        g_object_unref(displays[pixbufnum].buf);

    displays[pixbufnum].buf = gdk_pixbuf_new_from_data((const guchar *) pic,        /* the data */
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
