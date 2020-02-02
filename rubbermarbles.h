/*
 * Rubber Marbles - K Sheldrake
 * rubbermarbles.h
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


#ifndef _RB_H
#define _RB_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <signal.h>

#include "rb-hilbert.h"
#include "rb-data.h"
#include "rb-gtk.h"
#include "rb-draw.h"
#include "trigraph.h"
#include "rb-vis.h"
#include "macro.h"

void hexdump(unsigned char *data, int data_len);
int load_file(char *fname);
int main(int argc, char *argv[]);

#endif
