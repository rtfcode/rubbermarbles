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
 * Provides functions to create shared memory buffers.
 */


#include "vis-shm.h"


/* shm_open_buffer opens an existing named shared memory area and initialises the semaphore */
int shm_open_buffer(char *shmpath, struct rb_shm **shm, sem_t ** sem)
{
    int fd;

    if (!shmpath || !shm || !sem) {
        FAIL_MSG("shm_open_buffer: invalid params\n");
        return 1;
    }


    fd = shm_open(shmpath, O_RDWR, 0600);
    if (fd < 0) {
        FAIL_ERR("shm_open_buffer: shm_open() failed\n");
        return 2;
    }


    *shm =
        (struct rb_shm *) mmap(NULL, sizeof(struct rb_shm),
                               PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (*shm == MAP_FAILED) {
        FAIL_ERR("shm_open_buffer: mmap() failed\n");
        return 3;
    }


    if (!((*shm)->semname[0])) {
        FAIL_MSG("shm_open_buffer: semname is empty\n");
        return 4;
    }


    *sem = sem_open((*shm)->semname, O_CREAT | O_RDWR, 0600, 1);
    if (*sem == SEM_FAILED) {
        FAIL_ERR("shm_open_buffer: sem_open() failed\n");
        return 5;
    }


    return 0;
}
