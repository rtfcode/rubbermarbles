/*
 * Rubber Marbles - K Sheldrake
 * rubbermarbles.c
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 * 
 * This is the main shebang that gets the show on the road.
 */


#include "rubbermarbles.h"

extern struct window zoom[2];
extern struct savepic save[5];
/* memory access */
struct filemmap mmap_ctx;
struct shm_buf *shm_ctx = NULL;
struct rb_shm *shm = NULL;
sem_t *rbsem = NULL;

/* simple hexdump - only used for debugging, hence no real error checking */
void hexdump(unsigned char *data, int data_len)
{
    int i;

    if (!data) {
        printf("hexdump: invalid parameters\n");
        return;
    }

    printf("Hexdump from %p:\n", data);

    for (i = 0; i < data_len; i++) {
        if ((i % HEX_PER_ROW) == 0) {
            printf("\n0x%04x: ", i);
        }
        printf("%02x ", data[i]);
    }
    printf("\n\n");
}


/* load_file loads a file into memory 
 * note: returns fd on success, or 0 on failure */
int load_file(char *fname)
{
    int filed, i;
    struct stat filestat;

    if (!fname) {
        FAIL_MSG("load_file: invalid params\n");
        return 1;
    }


    /* open the file */
    filed = open(fname, O_RDONLY);
    if (filed == -1) {
        FAIL_ERR("load_file: cannot open file\n");
        return 2;
    }


    /* stat it */
    if (fstat(filed, &filestat) != 0) {
        FAIL_ERR("load_file: cannot stat file\n");
        return 3;
    }


    /* kill the running visualisers */
    if (kill_children() != 0) {
        FAIL_ERR("load_file: kill_children() failed\n");
        return 4;
    }


    /* unmap and close the current file */
    if (mmap_ctx.filedata) {
        if (munmap(mmap_ctx.filedata, mmap_ctx.mmap_size) != 0) {
            FAIL_MSG("load_file: munmap() failed\n");
            return 5;
        }

    }

    if (shm->fd) {
        if (close(shm->fd) != 0) {
            FAIL_ERR("load_file: close() failed\n");
            return 6;
        }

    }

    /* initialise shared memory to new file */
    if (sem_wait(shm_ctx->sem) != 0) {
        FAIL_ERR("load_file: sem_wait() failed\n");
        return 7;
    }

    memcpy(&(shm->filestat), &filestat, sizeof(filestat));
    strncpy(shm->filename, fname, PATH_MAX);
    shm->filename[PATH_MAX - 1] = 0x00;
    shm->fd = filed;
    shm->offset = 0;
    shm->bufsize = filestat.st_size;
    shm->buf_type = BUF_TYPE_FD;
    if (sem_post(shm_ctx->sem) != 0) {
        FAIL_ERR("load_file: sem_post() failed\n");
        return 8;
    }


    /* reset the selection windows */
    zoom[ZOOM_WHOLE].start = -1;
    zoom[ZOOM_WHOLE].end = -1;
    zoom[ZOOM_ZOOM].start = -1;
    zoom[ZOOM_ZOOM].end = -1;

    zoom[ZOOM_WHOLE].step_hilbert =
        (long double) filestat.st_size / (512 * 512);
    zoom[ZOOM_WHOLE].step_zigzag =
        (long double) filestat.st_size / (128 * 512);
    zoom[ZOOM_ZOOM].step_hilbert =
        (long double) filestat.st_size / (512 * 512);
    zoom[ZOOM_ZOOM].step_zigzag =
        (long double) filestat.st_size / (128 * 512);

    /* invalidate any saved bitmaps */
    for (i = 0; i < 5; i++) {
        if (save[i].pic) {
            free(save[i].pic);
            save[i].pic = NULL;
        }
    }

    /* set the mmap params */
    mmap_ctx.mmap_offset = 0;
    mmap_ctx.mmap_size = MMAP_CHUNK_SIZE;
    if (mmap_ctx.mmap_size > filestat.st_size) {
        mmap_ctx.mmap_size = filestat.st_size;
    }

    /* mmap it */
    mmap_ctx.filedata =
        mmap((caddr_t) 0, mmap_ctx.mmap_size, PROT_READ, MAP_SHARED, filed,
             mmap_ctx.mmap_offset);

    if (mmap_ctx.filedata == MAP_FAILED) {
        FAIL_ERR("load_file: mmap() failed\n");
        return 9;
    }


    /* update any remaining children */
    if (update_children() != 0) {
        FAIL_ERR("load_file: update_children() failed\n");
        return 10;
    }


    return 0;
}


/* RubberMarbles main() function */
int main(int argc, char *argv[])
{
    struct sigaction sa;
    char *homepath = NULL;

    gdk_threads_init();
    gdk_threads_enter();

    printf("Rubber Marbles v%s (beta) - K Sheldrake, %s\n\n",
           RBVER, RBDATE);

    gtk_init(&argc, &argv);

    if (rb_shm_init(&shm_ctx, &shm) != 0) {
        FAIL_MSG("RubberMarbles: rb_shm_init() failed\n");
        return 1;
    }


    if (argc != 2) {
        printf("usage: rubbermarbles filename\n");
        exit(1);
    }

    mmap_ctx.filedata = NULL;

    if (load_file(argv[1]) != 0) {
        FAIL_MSG("RubberMarbles: load_file() failed\n");
        return 2;
    }


    sa.sa_handler = &child_reap;
    if (sigemptyset(&sa.sa_mask) != 0) {
        FAIL_ERR("RubberMarbles: sigemptyset() failed\n");
        return 3;
    }

    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) != 0) {
        FAIL_ERR("RubberMarbles: cannot set SIGCHLD handler\n");
        return 4;
    }


    if (init_arrays() != 0) {
        FAIL_MSG("RubberMarbles: init_arrays() failed\n");
        return 5;
    }


    homepath = getenv("HOME");
    if (!homepath) {
        FAIL_MSG("RubberMarbles: HOME env var not set\n");
        return 6;
    }


    if (load_visualisers(homepath) != 0) {
        FAIL_MSG("RubberMarbles: load_visualisers() failed\n");
        return 7;
    }


    if (make_main_window() != 0) {
        FAIL_MSG("RubberMarbles: make_main_window() failed\n");
        return 8;
    }


    if (make_help_dialog() != 0) {
        FAIL_MSG("RubberMarbles: make_help_dialog() failed\n");
        return 9;
    }


    gtk_main();

    gdk_threads_leave();

    return 0;
}
