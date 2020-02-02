/*
 * Rubber Marbles - K Sheldrake
 * rb-mmap.c
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 *
 * Provides functions for mmapping files.
 */


#include "rb-mmap.h"


/* rb_init_mmap clears the values in the mmap_ctx */
void *rb_init_mmap()
{
	struct filemmap *mmap_ctx;
	
	mmap_ctx = (struct filemmap *)malloc(sizeof(struct filemmap));
	if (!mmap_ctx) {
		FAIL_ERR("rb_init_mmap: malloc() failed\n");
		return NULL;
	}

	mmap_ctx->mmap_size = 0;
	mmap_ctx->mmap_offset = 0;
	mmap_ctx->mmap_ptr = NULL;
	mmap_ctx->size = 0;
	mmap_ctx->offset = 0;
	mmap_ctx->ptr = NULL;
	
	return mmap_ctx;
}


/* rb_mmap maps a section of the file from the offset.
 * it stores the real mmap params in mmap_ctx->mmap_* params, and the user params in
 * mmap_ctx->data and mmap_ctx->size.
*/
int rb_mmap(struct filemmap *mmap_ctx, int fd, unsigned long offset, unsigned long filesize)
{
	
	if (!mmap_ctx || !fd || (offset > filesize)) {
		FAIL_MSG("rb_mmap: invalid params\n");
		return 1;
	}
	
	
	/* if we already have a chunk, then unmap it */
	if (mmap_ctx->mmap_ptr) {
		if (munmap(mmap_ctx->mmap_ptr, mmap_ctx->mmap_size) != 0) {
			FAIL_ERR("rb_mmap: munmap() failed\n");
			return 2;
		}

	}

	/* invalid pointers */
	mmap_ctx->mmap_ptr = NULL;
	mmap_ctx->ptr = NULL;
	
	/* shift the values to align with a page */
    mmap_ctx->mmap_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
	
	if ((mmap_ctx->mmap_offset + MMAP_CHUNK_SIZE) < filesize) {
		mmap_ctx->mmap_size = MMAP_CHUNK_SIZE;
	} else {
		mmap_ctx->mmap_size = filesize - mmap_ctx->mmap_offset;
	}

	/* set user size; e.g. the size of the chunk from the user offset */
	mmap_ctx->size = mmap_ctx->mmap_size - (offset - mmap_ctx->mmap_offset);
	mmap_ctx->offset = offset;
	
    /* mmap() it */
    mmap_ctx->mmap_ptr =
        mmap((caddr_t) 0, mmap_ctx->mmap_size, PROT_READ, MAP_SHARED, fd,
             mmap_ctx->mmap_offset);

    if (mmap_ctx->mmap_ptr == MAP_FAILED) {
        perror("rb_mmap: mmap() failed\n");
        fprintf(stderr, "mmap_offset = 0x%lx\n", mmap_ctx->mmap_offset);
        fprintf(stderr, "mmap_size = 0x%lx\n", mmap_ctx->mmap_size);
        fprintf(stderr, "fd = %d\n", fd);

        return 3;
    }

    /* set user data pointer */
    mmap_ctx->ptr = mmap_ctx->mmap_ptr + (offset - mmap_ctx->mmap_offset);
	
	return 0;
}
	

/* rb_munmap unmaps a section of the file.
*/
int rb_munmap(struct filemmap *mmap_ctx)
{
	
	if (!mmap_ctx) {
		FAIL_MSG("rb_munap: invalid params\n");
		return 1;
	}
	
	
	/* if we already have a chunk, then unmap it */
	if (mmap_ctx->mmap_ptr) {
		if (munmap(mmap_ctx->mmap_ptr, mmap_ctx->mmap_size) != 0) {
			FAIL_ERR("rb_mmap: munmap() failed\n");
			return 2;
		}

	}

	/* invalid pointers */
	mmap_ctx->mmap_ptr = NULL;
	mmap_ctx->ptr = NULL;
	mmap_ctx->size = 0;
	mmap_ctx->mmap_offset = 0;
	mmap_ctx->mmap_size = 0;
	
	return 0;
}

	
	
