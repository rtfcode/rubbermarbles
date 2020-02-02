/*
 * Rubber Marbles - K Sheldrake
 * rb-vis.h
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


#ifndef _VISUALISER_H
#define _VISUALISER_H

#include <stdlib.h>
#include <string.h>
#include "rb-data.h"
#include "macro.h"

int load_vis(char *str);
int load_visfile(char *confpath);
int load_visualisers();


#endif
