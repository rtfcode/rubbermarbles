/*
 * Rubber Marbles - K Sheldrake
 * trigraph.c
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 * 
 * Provides Trigraph and Delayed Trigraph visualisers in OpenGL.
 */

#include "trigraph.h"

struct tg_ctx *ctx = NULL;
struct rb_shm *shm;
struct shm_buf *shm_ctx;
sem_t *sem;
int shm_destroy;

GLfloat axis_verts[] = {
    -1.1, -1.1, -1.1,
    -0.9, -1.1, -1.1,
    -1.1, -1.1, -1.1,
    -1.1, -0.9, -1.1,
    -1.1, -1.1, -1.1,
    -1.1, -1.1, -0.9,
    -1.1, -1.1, -1.1};

GLfloat axis_cols[] = {
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0,
    1.0, 1.0, 1.0};

int axis_vert_count = 7;

/* enable_usr1 enables signal handling for SIGUSR1 */
int enable_usr1()
{
    if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
        FAIL_ERR("enable_usr1: cannot catch SIGUSR1\n");
        return 1;
    }

    return 0;
}


/* disable_usr1 disables signal handling for SIGUSR1 */
int disable_usr1()
{
    if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {
        FAIL_ERR("disable_usr1: cannot ignore SIGUSR1\n");
        return 1;
    }

    return 0;
}


/* sig_handler is a signal callback.  SIGUSR1 means buffer has changed; SIGHUP means close window */
void sig_handler(int signo)
{
    if (signo == SIGUSR1) {
        if (disable_usr1() != 0) {
            FAIL_MSG("sig_handler: disable_usr1() failed\n");
            return;
        }

        ctx->reload = 1;
        ctx->display = 1;
    } else if (signo == SIGHUP) {
        if (disable_usr1() != 0) {
            FAIL_MSG("sig_handler: disable_usr1() failed\n");
            return;
        }

        ctx->quit = 1;
    }
}


/* delayedtrigraph_init is the VIS_init() fn for delayedtrigraph */
void *delayedtrigraph_init(unsigned int xsize, unsigned int ysize,
                           unsigned int x, unsigned int y)
{
    void *ctx = trigraph_init(xsize, ysize, x, y);
    if (!ctx) {
        FAIL_MSG("delayedtrigraph_init: trigraph_init() failed\n");
        return NULL;
    }

    return ctx;
}


/* trigraph_free free()s the context */
int trigraph_free(void *rawctx)
{
    struct tg_ctx *ctx = rawctx;

    if (!rawctx) {
        FAIL_MSG("trigraph_free: invalid params\n");
        return 1;
    }


    if (ctx->vertices) {
        free(ctx->vertices);
    }
    if (ctx->colours) {
        free(ctx->colours);
    }
    free(ctx);

    return 0;
}


/* delayedtrigraph_free free()s the context */
int delayedtrigraph_free(void *rawctx)
{
    int retv = trigraph_free(rawctx);
    if (retv) {
        FAIL_MSG("delayedtrigraph_free: trigraph_free() failed\n");
        return retv;
    }

    return 0;
}


/* trigraph_init initialises the trigraph context */
void *trigraph_init(unsigned int xsize, unsigned int ysize, unsigned int x,
                    unsigned int y)
{
    if (!xsize || !ysize || (xsize > 4096) || (ysize > 4096) || (x > 4096)
        || (y > 4096)) {
        FAIL_MSG("trigraph_init: invalid params\n");
        return NULL;
    }


    ctx = (struct tg_ctx *) calloc(1, sizeof(struct tg_ctx));

    if (!ctx) {
        FAIL_ERR("trigraph_init: calloc failed\n");
        return NULL;
    }


    ctx->screen_width = xsize;
    ctx->screen_height = ysize;
    ctx->screen_xpos = x;
    ctx->screen_ypos = y;
    ctx->angle = 0.0;
    ctx->running = 1;
    ctx->rot_start_time = 0;
    ctx->angle_start_delta = 0.0;
    ctx->colset = 0;

    return (void *) ctx;
}


/* tg_fd_initialised checks if the fd context is initialised */
int tg_fd_initialised()
{
    if (!ctx || !shm
        || ((ctx->dsize != 1) && (ctx->dsize != 2) && (ctx->dsize != 4)
            && (ctx->dsize != 8))
        || ((ctx->endian != TG_LITTLE_ENDIAN)
            && (ctx->endian != TG_BIG_ENDIAN))) {
        FAIL_MSG("tg_fd_initialised: fd not initialised\n");
        return 1;
    }

    return 0;
}


/* tg_buf_initialised checks if the buf context is initialised */
int tg_buf_initialised()
{
    if (!ctx || !shm || !(ctx->buf) || !(ctx->bufsize) ||
        ((ctx->dsize != 1) && (ctx->dsize != 2) && (ctx->dsize != 4)
         && (ctx->dsize != 8)) || ((ctx->endian != TG_LITTLE_ENDIAN)
                                   && (ctx->endian != TG_BIG_ENDIAN))) {
        FAIL_MSG("tg_buf_initialised: buffer not initialised\n");
        return 1;
    }

    return 0;
}


/* tg_get_value pulls a value from the buf with context data size and endian */
unsigned long tg_get_value(unsigned long loc)
{
    int k;
    unsigned long value;

    if (tg_buf_initialised() != 0) {
        FAIL_MSG("tg_get_value: invalid params\n");
        return 0;
    }


    value = 0;
    if (ctx->endian == TG_BIG_ENDIAN) {
        for (k = 0; k < ctx->dsize; k++) {
            value = (value << 8) | ctx->buf[(loc * ctx->dsize) + k];
        }
    } else {
        for (k = ctx->dsize - 1; k >= 0; k--) {
            value = (value << 8) | ctx->buf[(loc * ctx->dsize) + k];
        }
    }

    return value;
}


/* tg_load_data loads the vertices and colours from buf or fd */
int tg_load_data()
{
    int retv;

    if (ctx->buftype == BUF_TYPE_SHM) {
        if (tg_buf_initialised() != 0) {
            FAIL_MSG("tg_load_data: invalid params\n");
            return 1;
        }


        /* copy shared mem values to context */
        if (sem_wait(sem) != 0) {
            FAIL_ERR("tg_load_data: sem_wait() failed\n");
            return 2;
        }

        ctx->buf = shm->buf;
        ctx->bufsize = shm->bufsize;
        if (sem_post(sem) != 0) {
            FAIL_ERR("tg_load_data: sem_post() failed\n");
            return 3;
        }


        /* load the buffer */
        retv = tg_load_buffer();
        if (retv) {
            FAIL_MSG("tg_load_data: tg_load_buffer() failed\n");
            return retv;
        }


        return 0;

    } else if (ctx->buftype == BUF_TYPE_FD) {
        if (tg_fd_initialised() != 0) {
            FAIL_MSG("tg_load_data: invalid params\n");
            return 4;
        }


        /* load buffer from fd */
        retv = tg_load_fd();
        if (retv) {
            FAIL_MSG("tg_load_data: tg_load_fd() failed\n");
            return retv;
        }


        return 0;

    } else {
        fprintf(stderr, "tg_load_data: invalid buf type\n");
        return 5;
    }
}


/* tg_load_fd loads the vertices and colours from fd.
 * this is implemented by setting buf and using tg_load_buffer() */
int tg_load_fd()
{
    unsigned long offset;
    long size;

    if (tg_fd_initialised() != 0) {
        FAIL_MSG("tg_load_fd: invalid params\n");
        return 1;
    }


    /* check if we need to unmap an old segment */
    if (ctx->mmap_ptr) {
        if (munmap(ctx->mmap_ptr, ctx->mmap_size) != 0) {
            FAIL_ERR("tg_load_fd: munmap failed\n");
            return 2;
        }

    }

    if (sem_wait(sem) != 0) {
        FAIL_ERR("tg_load_fd: sem_wait() failed\n");
        return 3;
    }


    /* shift the offset to 8-byte alignment */
    if (shm->offset & 0x7) {
        offset = (shm->offset + 8) & ~0x7;
        size = shm->bufsize - (offset - shm->offset);
    } else {
        offset = shm->offset;
        size = shm->bufsize;
    }

    if (size <= 0) {
        FAIL_MSG("tg_load_fd: buffer is too small\n");
        return 4;
    }


    /* shift the offset to align with a page */
    ctx->mmap_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    ctx->mmap_size = size + (offset - ctx->mmap_offset);
    ctx->mmap_ptr =
        mmap((caddr_t) 0, ctx->mmap_size, PROT_READ, MAP_SHARED, ctx->fd,
             ctx->mmap_offset);

    if (ctx->mmap_ptr == MAP_FAILED) {
        FAIL_MSG("tg_load_fd: mmap() failed\n");
        return 5;
    }


    ctx->buf = ctx->mmap_ptr + (offset - ctx->mmap_offset);
    ctx->bufsize = size;
    ctx->offset = offset;
    if (sem_post(sem) != 0) {
        FAIL_ERR("tg_load_fd: sem_post() failed\n");
        return 6;
    }


    if (tg_load_buffer() != 0) {
        FAIL_MSG("tg_load_fd: tg_load_buffer() failed\n");
        return 7;
    }


    return 0;
}


/* tg_load_buffer loads the vertices and colours from buf */
int tg_load_buffer()
{
    unsigned long total_elements, i;
    int j;
    unsigned long long value;
    unsigned long long value2;
    unsigned long long divider;

    if (tg_buf_initialised() != 0) {
        FAIL_MSG("tg_load_buffer: invalid params\n");
        return 1;
    }


    total_elements = ctx->bufsize / ctx->dsize;

    if (ctx->vertices) {
        free(ctx->vertices);
    }
    if (ctx->colours) {
        free(ctx->colours);
    }

    switch (ctx->type) {
        case BG_NORMAL:
            ctx->vert_count = total_elements - 1;
            break;
        case TG_NORMAL:
        case BG_DELAYED:
            ctx->vert_count = total_elements - 2;
            break;
        case TG_DELAYED:
            ctx->vert_count = total_elements - 3;
            break;
        default:
            fprintf(stderr,
                    "tg_load_buffer: invalid buffer type, defaulting to trigraph normal\n");
            ctx->vert_count = total_elements - 2;
            ctx->type = TG_NORMAL;
    }

    ctx->vertices =
        (GLfloat *) calloc(ctx->vert_count * 3, sizeof(GLfloat));
    ctx->colours =
        (GLfloat *) calloc(ctx->vert_count * 3, sizeof(GLfloat));
    if (!(ctx->vertices)
        || !(ctx->colours)) {
        FAIL_ERR("tg_load_buffer: cannot calloc\n");
        return 2;
    }


    divider = pow(2, (ctx->dsize * 8) - 1);

    if (sem_wait(sem) != 0) {
        FAIL_ERR("tg_load_buffer: sem_wait() failed\n");
        return 3;
    }

    for (i = 0; i < ctx->vert_count; i++) {
        for (j = 0; j < 3; j++) {
            switch (ctx->type) {
                case TG_NORMAL:
                    value = tg_get_value(i + j);
                    ctx->vertices[(i * 3) + j] =
                        ((GLfloat) (value) / divider) - 1.0;
                    break;
                case TG_DELAYED:
                    value = tg_get_value(i + j);
                    value2 = tg_get_value(i + j + 1);
                    ctx->vertices[(i * 3) + j] =
                        (((GLfloat) (value2) -
                          (GLfloat) (value)) / divider) / 2.0;
                    break;
                case BG_NORMAL:
                    if (j < 2) {
                        value = tg_get_value(i + j);
                        ctx->vertices[(i * 3) + j] =
                            ((GLfloat) (value) / divider) - 1.0;
                    } else {
                        ctx->vertices[(i * 3) + j] = 0.0;
                    }
                    break;
                case BG_DELAYED:
                    if (j < 2) {
                        value = tg_get_value(i + j);
                        value2 = tg_get_value(i + j + 1);
                        ctx->vertices[(i * 3) + j] =
                            (((GLfloat) (value2) -
                              (GLfloat) (value)) / divider) / 2.0;
                    } else {
                        ctx->vertices[(i * 3) + j] = 0.0;
                    }
                    break;
            }
        }
        ctx->colours[(i * 3)] = (GLfloat) i / ctx->vert_count;
        ctx->colours[(i * 3) + 1] = 0.0;
        ctx->colours[(i * 3) + 2] = 1.0;
    }

    if (sem_post(sem) != 0) {
        FAIL_ERR("tg_load_buffer: sem_post() failed\n");
        return 4;
    }


    return 0;
}


/* trigraph_buffer defines and sets the buf and generates vertices and colours from it */
int
trigraph_buffer(void *unused_ctx, uint8_t * buf, unsigned long size,
                unsigned int type, unsigned int dsize, int endian)
{
    if (!ctx || !shm || !buf || !size
        || ((dsize != 1) && (dsize != 2) && (dsize != 4) && (dsize != 8))
        || ((endian != TG_LITTLE_ENDIAN) && (endian != TG_BIG_ENDIAN))
        || ((type != TG_NORMAL)
            && (type != TG_DELAYED))) {
        FAIL_MSG("trigraph_buffer: invalid params\n");
        return 1;
    }


    ctx->buf = buf;
    ctx->bufsize = size;
    ctx->buftype = BUF_TYPE_SHM;
    ctx->type = type;
    ctx->dsize = dsize;
    ctx->endian = endian;

    if (tg_load_buffer() != 0) {
        FAIL_MSG("trigraph_buffer: failed tg_load_buffer()\n");
        return 2;
    }


    return 0;
}


/* trigraph_filedesc is the VIS_filedesc function for trigraph */
int
trigraph_filedesc(void *unused_ctx, int fd, struct stat *filestat,
                  unsigned long offset, unsigned long size,
                  unsigned int dsize, int endian)
{
    return tg_filedesc(fd, filestat, offset, size, TG_NORMAL, dsize,
                       endian);
}


/* delayedtrigraph_filedesc is the VIS_filedesc function for delayedtrigraph */
int
delayedtrigraph_filedesc(void *unused_ctx, int fd,
                         struct stat *filestat, unsigned long offset,
                         unsigned long size, unsigned int dsize,
                         int endian)
{
    return tg_filedesc(fd, filestat, offset, size, TG_DELAYED, dsize,
                       endian);
}


/* bigraph_filedesc is the VIS_filedesc function for bigraph */
int
bigraph_filedesc(void *unused_ctx, int fd, struct stat *filestat,
                  unsigned long offset, unsigned long size,
                  unsigned int dsize, int endian)
{
    return tg_filedesc(fd, filestat, offset, size, BG_NORMAL, dsize,
                       endian);
}


/* delayedbigraph_filedesc is the VIS_filedesc function for delayedbigraph */
int
delayedbigraph_filedesc(void *unused_ctx, int fd,
                         struct stat *filestat, unsigned long offset,
                         unsigned long size, unsigned int dsize,
                         int endian)
{
    return tg_filedesc(fd, filestat, offset, size, BG_DELAYED, dsize,
                       endian);
}


/* tg_filesec defines and sets the fd and generates vertices and colours from it */
int
tg_filedesc(int fd, struct stat *filestat, unsigned long offset,
            unsigned long size, unsigned int type, unsigned int dsize,
            int endian)
{
    if (!ctx || !shm || !fd || !filestat || !size
        || ((dsize != 1) && (dsize != 2) && (dsize != 4) && (dsize != 8))
        || ((endian != TG_LITTLE_ENDIAN) && (endian != TG_BIG_ENDIAN))
        || ((type != TG_NORMAL)
            && (type != TG_DELAYED)
            && (type != BG_NORMAL)
            && (type != BG_DELAYED))) {
        FAIL_MSG("trigraph_filedesc: invalid params\n");
        return 1;
    }


    ctx->offset = offset;
    ctx->bufsize = size;
    ctx->buftype = BUF_TYPE_FD;
    ctx->type = type;
    ctx->dsize = dsize;
    ctx->endian = endian;
    ctx->connected = 1;

    if (tg_load_fd() != 0) {
        FAIL_MSG("trigraph_filedesc: failed tg_load_fd()\n");
        return 2;
    }


    return 0;
}


/* calcs the view and the projview matrices. projview = proj * view */
int set_projview()
{
    GLfloat m_eye[] = { 0.0, -0.01, 0.2 };
    GLfloat m_target[] = { 0.0, 0.0, 0.0 };
    GLfloat m_up[] = { 0.0, 1.0, 0.0 };

    GLfloat m_view[4][4];

    if (lookat(m_view, m_eye, m_target, m_up) != 0) {
        FAIL_MSG("set_projview: lookat() failed\n");
        return 1;
    }

    if (rotx(m_view, 30) != 0) {
        FAIL_MSG("set_projview: rotx() failed\n");
        return 2;
    }


    GLfloat m_projection[4][4];
    if (perspective(m_projection, radians(40.0),
                    1.0 * ctx->screen_width / ctx->screen_height, 0.1,
                    20.0) != 0) {
        FAIL_MSG("set_projview: perspective() failed\n");
        return 3;
    }


    if (multm4(ctx->m_projview, m_projection, m_view) != 0) {
        FAIL_MSG("set_projview: multm4() failed\n");
        return 4;
    }


    return 0;
}


/* calcs the view and the projview matrices. projview = proj * view */
int set_projview2d()
{
    GLfloat m_view[4][4];

    if (setidentitym4(m_view) != 0) {
        FAIL_MSG("set_projview2d: setidentitym4() failed\n");
        return 1;
    }


    if (translatev3(m_view, 0.0, 2.1, 1.8) != 0) {
        FAIL_MSG("set_projview2d: translatev3() failed\n");
        return 2;
    }


    GLfloat m_projection[4][4];

    if (perspective(m_projection, radians(40.0),
                    1.0 * ctx->screen_width / ctx->screen_height, 0.1,
                    20.0) != 0) {
        FAIL_MSG("set_projview: perspective() failed\n");
        return 3;
    }


    if (multm4(ctx->m_projview, m_projection, m_view) != 0) {
        FAIL_MSG("set_projview: multm4() failed\n");
        return 4;
    }


    return 0;
}


/* enableArray enables the vertices and normals arrays */
int enableArrays(GLfloat *verts, GLfloat *cols, unsigned int vert_count)
{
    if (!verts || !cols || !vert_count) {
        FAIL_MSG("enableArrays: invalid params\n");
        return 1;
    }

    glEnableVertexAttribArray(ctx->attribute_coord3d);
    // Describe our vertices array to OpenGL (it can't guess its format automatically)
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, (vert_count * 3 * sizeof(GLfloat)),
                 verts, GL_STATIC_DRAW);
    glVertexAttribPointer(ctx->attribute_coord3d,        // attribute
                          3,        // number of elements per vertex, here (x,y,z)
                          GL_FLOAT,        // the type of each element
                          GL_FALSE,        // take our values as-is
                          0,        // no extra data between each position
                          0        // offset of first element
        );

    glEnableVertexAttribArray(ctx->attribute_colour);
    // Describe our vertices array to OpenGL (it can't guess its format automatically)
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo_colours);
    glBufferData(GL_ARRAY_BUFFER, (vert_count * 3 * sizeof(GLfloat)),
                 cols, GL_STATIC_DRAW);
    glVertexAttribPointer(ctx->attribute_colour,        // attribute
                          3,        // number of elements per vertex, here (x,y,z)
                          GL_FLOAT,        // the type of each element
                          GL_FALSE,        // take our values as-is
                          0,        // no extra data between each position
                          0        // offset of first element
        );

    return 0;
}


/* disableArray disables the vertices and normals arrays */
void disableArrays()
{
    glDisableVertexAttribArray(ctx->attribute_coord3d);
    glDisableVertexAttribArray(ctx->attribute_colour);
}


/* one time set up */
int init_resources()
{
    /* bind the buffers to the VBOs */
    glGenBuffers(1, &(ctx->vbo_vertices));
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, (ctx->vert_count * 3 * sizeof(GLfloat)),
                 ctx->vertices, GL_STATIC_DRAW);
    glGenBuffers(1, &(ctx->vbo_colours));
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo_colours);
    glBufferData(GL_ARRAY_BUFFER, (ctx->vert_count * 3 * sizeof(GLfloat)),
                 ctx->colours, GL_STATIC_DRAW);

    /* load, compile and link the shaders */
    GLint link_ok = GL_FALSE;

    GLuint vs, fs;
    vs = create_shader("trigraph.v.glsl", GL_VERTEX_SHADER);
    if (!vs) {
        FAIL_MSG("init_resources: create_shader() failed\n");
        return 1;
    }

    fs = create_shader("trigraph.f.glsl", GL_FRAGMENT_SHADER);
    if (!fs) {
        FAIL_MSG("init_resources: create_shader() failed\n");
        return 2;
    }


    ctx->program = glCreateProgram();
    if (!ctx->program) {
        FAIL_MSG("init_resources: glCreateProgram() failed\n");
        return 3;
    }

    glAttachShader(ctx->program, vs);
    glAttachShader(ctx->program, fs);
    glLinkProgram(ctx->program);
    glGetProgramiv(ctx->program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        fprintf(stderr, "glLinkProgram:");
        print_log(ctx->program);
        return 4;
    }

    /* get handles to the shader variables */
    ctx->attribute_coord3d = get_attrib(ctx->program, "coord3d");
    if (ctx->attribute_coord3d == -1) {
        FAIL_MSG("init_resources: get_attrib() failed\n");
        return 5;
    }

    ctx->attribute_colour = get_attrib(ctx->program, "v_color");
    if (ctx->attribute_colour == -1) {
        FAIL_MSG("init_resources: get_attrib() failed\n");
        return 6;
    }


    ctx->uniform_colset = get_uniform(ctx->program, "colset");
    if (ctx->uniform_colset == -1) {
        FAIL_MSG("init_resources: get_uniform() failed\n");
        return 7;
    }

    ctx->uniform_mvp = get_uniform(ctx->program, "mvp");
    if (ctx->uniform_mvp == -1) {
        FAIL_MSG("init_resources: get_uniform() failed\n");
        return 8;
    }


    /* set the projection view matrix */
    /* it's set one time here because we never move the camera in this program.
       If you need to move the camera, then you'll need to regenerate the
       projview as you do.
     */
    if ((ctx->type != BG_NORMAL) && (ctx->type != BG_DELAYED)) {
        if (set_projview() != 0) {
            FAIL_MSG("init_resources: set_projview() failed\n");
            return 9;
        }
    } else {
        if (set_projview2d() != 0) {
            FAIL_MSG("init_resources: set_projview2d() failed\n");
            return 9;
        }
    }



    return 0;
}


/* the onIdle function runs when there is time :)
   it's used here to update the rotation animation.
   The value of doing it here rather than in display is that
   here we only do it when there's time and we also calc the
   angle from the actual time rather than blindly advancing
   it on every draw cycle.
*/
void onIdle()
{
    /* running refers to the rotation animation.
       display refers to whether the display needs updating.
       If neither are true then there is nothing to do.
     */
    /* this massive 10ms delay is to allow onDisplay() to complete
       before we start throwing more frames at it.
     */
    if (ctx->quit) {
		cleanup();
        exit(0);
    }

    glFlush();

    if (!(ctx->display) && !(ctx->running)) {
        return;
    }

    /* initialise the start time */
    if (ctx->rot_start_time == 0) {
        ctx->rot_start_time = (int) (glfwGetTime() * 1000.0);
    }

    /* if the animation is running, update the angle */
    if (ctx->running) {
        ctx->angle = ctx->angle_start_delta + (((int) (glfwGetTime() * 1000.0) - ctx->rot_start_time) / 1000.0 * 45);        // 45° per second
    }

    /* if the display var was set, reset it */
    if (ctx->display) {
        ctx->display = 0;
    }

    if (ctx->reload) {
        if (tg_load_data() != 0) {
            FAIL_MSG("onIdle: tg_load_data() failed\n");
            return;
        }

        ctx->reload = 0;
        if (enable_usr1() != 0) {
            FAIL_MSG("onIdle: enable_usr1() failed\n");
            return;
        }

    }

    /* set the colset */
    glUseProgram(ctx->program);
    glUniform1i(ctx->uniform_colset, ctx->colset);

    /* create the anim matrix - basically just the rotation of the cube */
    if (setidentitym4(ctx->m_anim) != 0) {
        FAIL_MSG("onIdle: setidentitym4() failed\n");
        return;
    }

    if (roty(ctx->m_anim, ctx->angle) != 0) {
        FAIL_MSG("onIdle: roty() failed\n");
        return;
    }


    onDisplay();
}


/* drawPoints draws the current points using the current model matrix */
int drawPoints()
{
    GLfloat m_mvp[4][4];

    glUseProgram(ctx->program);

    /* make the VBOs available */
    if (enableArrays(ctx->vertices, ctx->colours, ctx->vert_count) != 0) {
        FAIL_MSG("drawPoints: enableArrays() failed\n");
        return 1;
    }

    /* mvp = projview * model */
    if (multm4(m_mvp, ctx->m_projview, ctx->m_model) != 0) {
        FAIL_MSG("drawPoints: multm4() failed\n");
        return 2;
    }


    /* set the mvp in the shader */
    glUniformMatrix4fv(ctx->uniform_mvp, 1, GL_TRUE, (GLfloat *) m_mvp);

    /* Push each element in buffer_vertices to the vertex shader */
    glDrawArrays(GL_POINTS, 0, ctx->vert_count);

    /* memory saving (allegedly) */
    disableArrays();

    return 0;
}


/* drawAxis draws the mini axis just outside the origin */
int drawAxis()
{
    GLfloat m_mvp[4][4];

    glUseProgram(ctx->program);

    /* make the VBOs available */
    enableArrays(axis_verts, axis_cols, axis_vert_count);

    /* mvp = projview * model */
    if (multm4(m_mvp, ctx->m_projview, ctx->m_model) != 0) {
        FAIL_MSG("drawAxis: multm4() failed\n");
        return 1;
    }


    /* set the mvp in the shader */
    glUniformMatrix4fv(ctx->uniform_mvp, 1, GL_TRUE, (GLfloat *) m_mvp);

    /* Push each element in buffer_vertices to the vertex shader */
    glDrawArrays(GL_LINES, 0, axis_vert_count);

    /* memory saving (allegedly) */
    disableArrays();

    return 0;
}


/* statusText draws the text onto the display */
int statusText()
{
    char msg[128];
    char msg2[128];
    GLfloat white[4] = {1, 1, 1, 1};
    GLfloat grey[4] = {0.5, 0.5, 0.5, 0.5};
    int i;
    float textoff;
    int fontsize = 24 * ctx->screen_width / 600;
    int fontgap = fontsize / 3;

    /* print the data sizes 1, 2, 4 and 8 with the current one highlighted */
    textoff = fontgap;
    for (i=1; i<=8; i *= 2) {
        snprintf(msg, 128, "%d", i);
        if (ctx->dsize == i) {
            textoff = display_text(msg, fontsize, textoff, ctx->screen_height - (fontsize + (fontgap * 2)), white);
        } else {
            textoff = display_text(msg, fontsize, textoff, ctx->screen_height - (fontsize + (fontgap * 2)), grey);
        }
        textoff += fontgap * 2;
    }

    textoff += fontgap * 2;

    strcpy(msg, "LITTLE");
    strcpy(msg2, "BIG");
    if (ctx->endian == TG_LITTLE_ENDIAN) {
        textoff = display_text(msg, fontsize, textoff, ctx->screen_height - (fontsize + (fontgap * 2)), white);
        textoff += fontgap * 2;
        textoff = display_text(msg2, fontsize, textoff, ctx->screen_height - (fontsize + (fontgap * 2)), grey);
    } else {
        textoff = display_text(msg, fontsize, textoff, ctx->screen_height - (fontsize + (fontgap * 2)), grey);
        textoff += fontgap * 2;
        textoff = display_text(msg2, fontsize, textoff, ctx->screen_height - (fontsize + (fontgap * 2)), white);
    }

    textoff += fontgap * 4;

    if (!ctx->connected) {
        strcpy(msg, "HOLD");
        textoff = display_text(msg, fontsize, textoff, ctx->screen_height - (fontsize + (fontgap * 2)), white);
    }


    /* print buffer address range */
    snprintf(msg, 128, "0x%016lx - 0x%016lx", ctx->offset & ~0x7, (ctx->offset + ctx->bufsize) & ~0x7);
    textoff = display_text(msg, fontsize, fontgap, ctx->screen_height - fontgap, white);

    return 0;
}


/* onDisplay runs when display needs updating */
void onDisplay()
{
    /* clear the screen to black */
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* set the model matrix */
    if (setidentitym4(ctx->m_model) != 0) {
        FAIL_MSG("onDisplay: setidentitym4() failed\n");
        return;
    }

    if (translatev3(ctx->m_model, 0.0, -2.0, -5.0) != 0) {
        FAIL_MSG("onDisplay: translatev3() failed\n");
        return;
    }


    /* model = model * anim */
    if (multm4(ctx->m_model, ctx->m_model, ctx->m_anim) != 0) {
        FAIL_MSG("onDisplay: multm4() failed\n");
        return;
    }


    /* send the points to the GPU */
    if (drawPoints() != 0) {
        FAIL_MSG("onDisplay: drawPoints() failed\n");
        return;
    }


    /* draw the axis */
    if ((ctx->type == TG_NORMAL) || (ctx->type == TG_DELAYED)) {
        if (drawAxis() != 0) {
            FAIL_MSG("onDisplay: drawAxis() failed\n");
            return;
        }
    }


    /* write text */
    /* do this last so that it can blend with the 3d model */
    if (statusText() != 0) {
        FAIL_MSG("onDisplay: statusText() failed\n");
        return;
    }

    /* update display */
    glfwSwapBuffers(ctx->window);
}


/* framebuffer_size_callback is called when window is resized. Constrain to square. */
void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
    ctx->screen_width = width;
    ctx->screen_height = height;
    if (width < height) {
        ctx->screen_height = width;
    } else {
        ctx->screen_width = height;
    }
    glViewport(0, 0, ctx->screen_width, ctx->screen_height);
    ctx->display = 1;
}


/* window_refresh_callback is alled when the window exposure changes */
void window_refresh_callback(GLFWwindow * window)
{
    ctx->display = 1;
}


/* key_callback is called when a key is pressed */
void key_callback(GLFWwindow * window, int key, int scancode,
                  int action, int mods)
{

    if (action != GLFW_PRESS) {
        return;
    }

    switch (key) {
    case 'Q':
    case GLFW_KEY_ESCAPE:
        /* quit */
		cleanup();
        exit(0);
        break;
    case 'P':
    case ' ':
        /* pause / unpause */
        if ((ctx->type == TG_NORMAL) || (ctx->type == TG_DELAYED)) {
            if (!(ctx->running)) {
                ctx->rot_start_time = (int) (glfwGetTime() * 1000.0);
                ctx->angle_start_delta = ctx->angle;
                ctx->running = 1;
            } else {
                ctx->running = 0;
            }
        }
        break;
    case 'M':
        /* rotate */
        if ((ctx->type == TG_NORMAL) || (ctx->type == TG_DELAYED)) {
            if (!(ctx->running)) {
                ctx->angle += 5.0;
                ctx->display = 1;
            }
        }
        break;
    case 'N':
        /* rotate */
        if ((ctx->type == TG_NORMAL) || (ctx->type == TG_DELAYED)) {
            if (!(ctx->running)) {
                ctx->angle -= 5.0;
                ctx->display = 1;
            }
        }
        break;
    case '1':
    case '2':
    case '4':
    case '8':
        /* change element size */
        ctx->dsize = key - '0';
        if (tg_load_data() != 0) {
            FAIL_MSG("onKeyboard: tg_load_data() failed\n");
            return;
        }

        ctx->display = 1;
        break;
    case 'L':
        /* switch to little endian */
        ctx->endian = TG_LITTLE_ENDIAN;
        if (tg_load_data() != 0) {
            FAIL_MSG("onKeyboard: tg_load_data() failed\n");
            return;
        }

        ctx->display = 1;
        break;
    case 'B':
        /* switch to big endian */
        ctx->endian = TG_BIG_ENDIAN;
        if (tg_load_data() != 0) {
            FAIL_MSG("onKeyboard: tg_load_data() failed\n");
            return;
        }

        ctx->display = 1;
        break;
    case 'C':
        /* cycle to next colour set.
           these are:
           * blue->pink - first points are blue, last are pink.
           * grey - first points are black, last are white.
           * blue - all points are blue.
         */
        (ctx->colset)++;
        if (ctx->colset > 2) {
            ctx->colset = 0;
        }
        ctx->display = 1;
        break;
    case 'H':
        /* hold / unhold.
         * determines whether display updates when RB changes or not */
        if (ctx->connected) {
            ctx->connected = 0;
            disable_usr1();
        } else {
            ctx->connected = 1;
            if (tg_load_data() != 0) {
                FAIL_MSG("onKeyboard: tg_load_data() failed\n");
                return;
            }
            enable_usr1();
        }
        ctx->display = 1;
        break;
    default:
        break;
    }
}


/* kill off our buffers */
void free_resources()
{
    glDeleteProgram(ctx->program);
    glDeleteBuffers(1, &(ctx->vbo_vertices));
    glDeleteBuffers(1, &(ctx->vbo_colours));
}


/* trigraph_display is the VIS_display function for trigraph */
int trigraph_display(void *unused_ctx)
{
    tg_display();

    return 0;
}


/* delayedtrigraph_display is the VIS_display function for delayedtrigraph */
int delayedtrigraph_display(void *unused_ctx)
{
    return tg_display();
}

static void error_callback(int error, const char *desc)
{
    fputs(desc, stderr);
}


/* tg_display is the display routine */
int tg_display()
{

    /* set up glfw error callback */
    glfwSetErrorCallback(error_callback);

    /* init glfw */
    if (!glfwInit()) {
        FAIL_MSG("tg_display: glfwInit() failed\n");
        return 1;
    }


    /* create window */
    switch (ctx->type) {
        case TG_NORMAL:
            ctx->window =
                glfwCreateWindow(ctx->screen_width, ctx->screen_height,
                                 "3D Trigraph Plot", NULL, NULL);
            break;
        case TG_DELAYED:
            ctx->window =
                glfwCreateWindow(ctx->screen_width, ctx->screen_height,
                                 "3D Delayed Trigraph Plot", NULL, NULL);
            break;
        case BG_NORMAL:
            ctx->window =
                glfwCreateWindow(ctx->screen_width, ctx->screen_height,
                                 "2D Bigraph Plot", NULL, NULL);
            break;
        case BG_DELAYED:
            ctx->window =
                glfwCreateWindow(ctx->screen_width, ctx->screen_height,
                                 "2D Delayed Bigraph Plot", NULL, NULL);
            break;
        default:
            ctx->type = TG_NORMAL;
            ctx->window =
                glfwCreateWindow(ctx->screen_width, ctx->screen_height,
                                 "3D Trigraph Plot", NULL, NULL);
    }

    if (!ctx->window) {
        fprintf(stderr, "tg_display: glfwCreateWindow() failed\n");
        glfwTerminate();
        return 2;
    }

    /* set up context */
    glfwMakeContextCurrent(ctx->window);
    glfwSwapInterval(1);

    /* keyboard callback */
    glfwSetKeyCallback(ctx->window, key_callback);

    /* resize callback */
    glfwSetFramebufferSizeCallback(ctx->window, framebuffer_size_callback);

    /* window refresh callback */
    glfwSetWindowRefreshCallback(ctx->window, window_refresh_callback);

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
        return 3;
    }

    if (!GLEW_VERSION_2_0) {
        FAIL_MSG("Error: your graphic card does not support OpenGL 2.0\n");
        return 4;
    }


    if (enable_usr1() != 0) {
        FAIL_MSG("tg_display: enable_usr1() failed\n");
        return 5;
    }


    if (signal(SIGHUP, sig_handler) == SIG_ERR) {
        FAIL_ERR("cannot catch SIGHUP\n");
        return 6;
    }


    if (init_resources() != 0) {
        FAIL_MSG("tg_display: init_resources() failed\n");
        return 7;
    }

    if (init_text_resources() == 0) {
        FAIL_MSG("tg_display: init_text_resources() failed\n");
        return 8;
    }

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LESS);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(ctx->window)) {
        onIdle();

        glfwPollEvents();
    }

    free_resources();

    glfwDestroyWindow(ctx->window);
    glfwTerminate();
	cleanup();
    exit(0);
    return 0;
}


/* cleanup closes the shared memory */
int cleanup()
{
	if (shm_destroy) {
		if (rb_shm_close(shm_ctx, shm) != 0) {
			FAIL_MSG("cleanup: rb_shm_close() failed\n");
			return 1;
		}
	}
	
	return 0;
}


int main(int argc, char *argv[])
{
    struct stat stattmp;
    char *ptr = NULL;

    if (argc <= 0) {
        printf
            ("Usage: trigraph shmpath|file\n\nVisualiser for Rubber Marbles\n");
        exit(1);
    } else if (argc != 2) {
        printf("Usage: %s shmpath|file\n\nVisualiser for Rubber Marbles\n",
               argv[0]);
        exit(1);
    }

    if (!strncmp(argv[1], "/rb_shm.", 8)) {

        /* argument is named shared memory buffer */
		shm_destroy = 0;
        if (shm_open_buffer(argv[1], &shm, &sem) != 0) {
            FAIL_MSG("main: shm_open_buffer() failed\n");
            return 2;
        }

    } else {

        /* argument is a file */
		shm_destroy = 1;
        if (rb_shm_init(&shm_ctx, &shm) != 0) {
            FAIL_MSG("main: rb_shm_init() failed\n");
            return 3;
        }

        if (stat(argv[1], &stattmp) != 0) {
            FAIL_ERR("main: stat() failed\n");
            return 4;
        }

        strncpy(shm->filename, argv[1], PATH_MAX - 1);
        shm->filename[PATH_MAX - 1] = 0x00;
        shm->offset = 0;
        shm->bufsize = stattmp.st_size;
        shm->buf_type = BUF_TYPE_FD;
        sem = shm_ctx->sem;

    }


    ptr = strrchr(argv[0], '/');
    if (!ptr) {
        ptr = argv[0];
    } else {
        ptr++;
    }

    ctx = trigraph_init(600, 600, 200, 200);
    if (!ctx) {
        FAIL_MSG("main: trigraph_init() failed\n");
        return 5;
    }


    /* open the file */
    ctx->fd = open(shm->filename, O_RDONLY);
    if (ctx->fd == -1) {
        FAIL_ERR("main: cannot open file\n");
        return 6;
    }


    /* stat it */
    if (fstat(ctx->fd, &(ctx->filestat)) != 0) {
        FAIL_ERR("main: cannot stat file\n");
        return 7;
    }


    if (strncmp(ptr, "trigraph", 9) == 0) {
        if (trigraph_filedesc(ctx, ctx->fd, &(ctx->filestat),
                              shm->offset, shm->bufsize,
                              1, TG_LITTLE_ENDIAN) != 0) {
            FAIL_MSG("main: trigraph_filedesc() failed\n");
            return 8;
        }

    } else if (strncmp(ptr, "delayedtrigraph", 16) == 0) {

        if (delayedtrigraph_filedesc(ctx, ctx->fd, &(ctx->filestat),
                                     shm->offset, shm->bufsize,
                                     1, TG_LITTLE_ENDIAN) != 0) {
            FAIL_MSG("main: delayedtrigraph_filedesc() failed\n");
            return 9;
        }

    } else if (strncmp(ptr, "bigraph", 8) == 0) {
        ctx->running = 0;
        ctx->display = 1;
        if (bigraph_filedesc(ctx, ctx->fd, &(ctx->filestat),
                              shm->offset, shm->bufsize,
                              1, TG_LITTLE_ENDIAN) != 0) {
            FAIL_MSG("main: bigraph_filedesc() failed\n");
            return 10;
        }

    } else if (strncmp(ptr, "delayedbigraph", 15) == 0) {
        ctx->running = 0;
        ctx->display = 1;
        if (delayedbigraph_filedesc(ctx, ctx->fd, &(ctx->filestat),
                                     shm->offset, shm->bufsize,
                                     1, TG_LITTLE_ENDIAN) != 0) {
            FAIL_MSG("main: delayedbigraph_filedesc() failed\n");
            return 11;
        }

    } else {

        FAIL_MSG("unknown program name\n");

        exit(1);
    }


    if (trigraph_display(ctx) != 0) {
        FAIL_MSG("main: trigraph_display() failed\n");
        return 12;
    }

    return 0;
}
