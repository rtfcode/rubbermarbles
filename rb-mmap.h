/*
 * Rubber Marbles - K Sheldrake
 * rb-mmap.h
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


#ifndef _RB_MMAP_H
#define _RB_MMAP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

//#include "rb-data.h"
#include "macro.h"

#define MMAP_CHUNK_SIZE 32 * 1024 * 1024

/* mmap context */
struct filemmap {
    unsigned long mmap_size;
    unsigned long mmap_offset;
    unsigned char *mmap_ptr;
	unsigned char *filedata;
	unsigned char *ptr;
	unsigned long size;
	unsigned long offset;
};

void *rb_init_mmap();
int rb_mmap(struct filemmap *mmap_ctx, int fd, unsigned long offset, unsigned long filesize);
int rb_munmap(struct filemmap *mmap_ctx);


#endif
