/*
 * Rubber Marbles - K Sheldrake
 * rb-hilbert.c
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 * 
 * Provides functions to map between linear locations and two
 * dimensional co-ordinates in a Hilbert plot.
 * Code taken from Wikipedia.
 */

#include "rb-hilbert.h"



/* hilbert routines from wikipedia
 * now with added error checking! */

int rot(int n, int *x, int *y, int rx, int ry)
{
    if (!x || !y) {
        FAIL_MSG("rot: invalid params\n");
        return 1;
    }


    if (ry == 0) {
        if (rx == 1) {
            *x = n - 1 - *x;
            *y = n - 1 - *y;
        }

        int t = *x;
        *x = *y;
        *y = t;
    }

    return 0;
}


int xy2d(int n, int x, int y)
{
    int rx, ry, s, d = 0;
    for (s = n / 2; s > 0; s /= 2) {
        rx = (x & s) > 0;
        ry = (y & s) > 0;
        d += s * s * ((3 * rx) ^ ry);
        if (rot(s, &x, &y, rx, ry) != 0) {
            FAIL_MSG("xy2d: rot() failed\n");
            return 1;
        }

    }
    return d;
}

int d2xy(int n, int d, int *x, int *y)
{
    int rx, ry, s, t = d;

    if (!x || !y) {
        FAIL_MSG("d2xy: invalid params\n");
        return 1;
    }


    *x = *y = 0;
    for (s = 1; s < n; s *= 2) {
        rx = 1 & (t / 2);
        ry = 1 & (t ^ rx);
        if (rot(s, x, y, rx, ry) != 0) {
            FAIL_MSG("d2xy: rot() failed\n");
            return 2;
        }

        *x += s * rx;
        *y += s * ry;
        t /= 4;
    }

    return 0;
}
