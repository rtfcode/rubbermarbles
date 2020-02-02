/*
 * Rubber Marbles - K Sheldrake
 * vis-shm.c
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 * 
 * Provides functions to open shared memory buffers.
 */


#include "rb-shm.h"

extern struct shm_buf *shm_ctx;
extern struct rb_shm *shm;

/* shm_create_buffer creates a shm buffer of size bytes and clears it if clear is set */
struct shm_buf *shm_create_buffer(unsigned long size, int clear)
{
    struct shm_buf *shmb;
    struct timeval tv;

    if (!size) {
        FAIL_MSG("shm_create_buffer: invalid params\n");
        return NULL;
    }


    shmb = (struct shm_buf *) calloc(1, sizeof(struct shm_buf));
    if (!shmb) {
        FAIL_ERR("shm_create_buffer: calloc() failed\n");
        return NULL;
    }


    if (gettimeofday(&tv, NULL) != 0) {
        FAIL_MSG("shm_create_buffer: gettimeofday() failed\n");
        return NULL;
    }


    snprintf(shmb->buf_name, 256, "/rb_shm.%016lx.%05x", tv.tv_sec,
             (int) tv.tv_usec);

    shmb->buf_fd = shm_open(shmb->buf_name, O_RDWR | O_CREAT, 0600);
    if (shmb->buf_fd < 0) {
        FAIL_ERR("shm_create_buffer: shm_open() failed\n");
        return NULL;
    }


    if (ftruncate(shmb->buf_fd, size) != 0) {
        FAIL_ERR("shm_create_buffer: ftruncate() failed\n");
        return NULL;
    }


    shmb->size = size;

    shmb->buffer =
        (uint8_t *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                         shmb->buf_fd, 0);
    if (shmb->buffer == MAP_FAILED) {
        FAIL_ERR("shm_create_buffer: mmap() failed\n");
        return NULL;
    }


    if (clear) {
        memset(shmb->buffer, 0, size);
    }

    return shmb;
}


/* rb_shm_init initialises the shared memory between parent and child */
int rb_shm_init(struct shm_buf **shm_ctx, struct rb_shm **shm)
{
    struct timeval tv;

	if (!shm_ctx || !shm) {
		FAIL_MSG("rb_shm_init: invalid params\n");
		return 1;
	}
	
    *shm_ctx =
        (struct shm_buf *) shm_create_buffer(sizeof(struct rb_shm), 1);
    if (!(*shm_ctx)) {
        FAIL_MSG("rb_shm_init: shm_create_buffer() failed\n");
        return 2;
    }


    *shm = (struct rb_shm *) ((*shm_ctx)->buffer);

    if (gettimeofday(&tv, NULL) != 0) {
        FAIL_MSG("rb_shm_init: gettimeofday() failed\n");
        return 3;
    }


    snprintf((*shm)->semname, 256, "/rb_sem.%016lx.%05x", tv.tv_sec,
             (int) tv.tv_usec);
    (*shm_ctx)->sem = sem_open((*shm)->semname, O_CREAT | O_RDWR, 0600, 1);
    if ((*shm_ctx)->sem == SEM_FAILED) {
        FAIL_ERR("rb_shm_init: sem_open() failed\n");
        return 4;
    }


    (*shm)->fd = 0;
    (*shm)->buf = NULL;
    (*shm)->offset = 0;
    (*shm)->bufsize = 0;

    return 0;
}


/* rb_shm_close closes the semaphore and shared memory buffer */
int rb_shm_close(struct shm_buf *shm_ctx, struct rb_shm *shm)
{

	if (!shm_ctx || !shm) {
		FAIL_MSG("rb_shm_close: invalid params\n");
		return 1;
	}
	
	
    /* close semaphore and shared memory */
    if (sem_close(shm_ctx->sem) != 0) {
        FAIL_ERR("rb_shm_close: sem_close() failed\n");
		return 2;
    }


    if (sem_unlink(shm->semname) != 0) {
        FAIL_ERR("rb_shm_close: sem_unlink() failed\n");
		return 3;
    }

    if (shm_unlink(shm_ctx->buf_name) != 0) {
        FAIL_ERR("rb_shm_close: shm_unlink() failed\n");
		return 4;
    }

	return 0;
}
