/*
 * Rubber Marbles - K Sheldrake
 * rb-shmdata.h
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


#ifndef _RB_SHMDATA_H
#define _RB_SHMDATA_H

#define BUF_TYPE_SHM 0
#define BUF_TYPE_FD 1

/* struct for shared memory object */
struct shm_buf {
    int buf_fd;
    char buf_name[256];
    uint8_t *buffer;
    unsigned long size;
	/* semaphore */
    sem_t *sem;
};

/* shared memory object for buffer details */
struct rb_shm {
    /* file */
    char filename[PATH_MAX];
	char semname[256];
    int fd;
    struct stat filestat;
    /* data buffer */
    int buf_type;
    uint8_t *buf;
    unsigned long offset;
    unsigned long bufsize;
};


#endif
