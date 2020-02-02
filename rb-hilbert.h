/*
 * Rubber Marbles - K Sheldrake
 * rb-hilbert.h
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


#ifndef _RB_HILBERT_H
#define _RB_HILBERT_H

#include <stdio.h>
#include "macro.h"

int rot(int n, int *x, int *y, int rx, int ry);
int xy2d(int n, int x, int y);
int d2xy(int n, int d, int *x, int *y);

#endif
