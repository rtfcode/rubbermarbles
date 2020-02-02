/*
 * Rubber Marbles - K Sheldrake
 * rb-shm.h
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <semaphore.h>
#include <signal.h>

#ifdef __linux__
#include <linux/limits.h>
#elif __APPLE__
#include <sys/syslimits.h>
#endif

#include "macro.h"
#include "rb-shmdata.h"

#ifndef _RB_SHM_H
#define _RB_SHM_H


struct shm_buf *shm_create_buffer(unsigned long size, int clear);
int rb_shm_init(struct shm_buf **shm_ctx, struct rb_shm **shm);
int rb_shm_close(struct shm_buf *shm_ctx, struct rb_shm *shm);

#endif
