/*
 * Rubber Marbles - K Sheldrake
 * rb-conf.h
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


#ifndef _RB_CONF_H
#define _RB_CONF_H

#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
//#include "rb-data.h"
#include "macro.h"

int get_conf(char *confpath, char *param, char *value);
int configuration(char *homepath, char *param, char *value);

#endif
