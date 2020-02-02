/*
 * Rubber Marbles - K Sheldrake
 * trigraph.h
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
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <semaphore.h>
#include <signal.h>
/* Use glew.h instead of gl.h to get all the GL prototypes declared */
#include <GL/glew.h>
/* GLFW for window management foo */
#include <GLFW/glfw3.h>

#include "vis-shm.h"
#include "rb-shm.h"
#include "shader_utils.h"
#include "matrixm.h"
#include "tg-text.h"
#include "macro.h"

#ifndef _TRIGRAPH_H
#define _TRIGRAPH_H

#define TG_LITTLE_ENDIAN 0
#define TG_BIG_ENDIAN 1

#define TG_NORMAL 0
#define TG_DELAYED 1
#define BG_NORMAL 2
#define BG_DELAYED 3


struct tg_ctx {
    /* program vars */
    int quit;
    int screen_width;
    int screen_height;
    int screen_xpos;
    int screen_ypos;
    int running;
    int display;
    int reload;
    int colset;
    float angle;
    int rot_start_time;
    float angle_start_delta;
    int connected;

    /* glfw vars */
    GLFWwindow *window;

    /* opengl matrices */
    GLfloat m_projview[4][4];
    GLfloat m_model[4][4];
    GLfloat m_anim[4][4];

    /* data buffer */
	int fd;
	struct stat filestat;
    uint8_t *buf;
    unsigned long bufsize;
    unsigned long offset;
    unsigned int type;
    unsigned int buftype;
    unsigned int dsize;
    unsigned int endian;
	char shmname[256];

    /* mmap section */
    unsigned long mmap_offset;
    unsigned long mmap_size;
    uint8_t *mmap_ptr;

    /* opengl vars */
    GLuint vbo_vertices;
    GLuint vbo_colours;
    GLuint program;
    GLint attribute_coord3d;
    GLint attribute_colour;
    GLint uniform_colset;
    GLint uniform_mvp;

    /* the vertices */
    GLfloat *vertices;
    GLfloat *colours;
    long vert_count;
};


void sig_handler(int signo);
int trigraph_free(void *ctx);
int delayedtrigraph_free(void *ctx);
void *trigraph_init(unsigned int xsize, unsigned int ysize, unsigned int x,
                    unsigned int y);
void *delayedtrigraph_init(unsigned int xsize, unsigned int ysize,
                           unsigned int x, unsigned int y);
int tg_fd_initialised();
int tg_buf_initialised();
unsigned long tg_get_value(unsigned long loc);
int tg_load_data();
int tg_load_fd();
int tg_load_buffer();
int trigraph_buffer(void *ctx, uint8_t * buf, unsigned long size,
                    unsigned int type, unsigned int dsize, int endian);
int trigraph_filedesc(void *ctx, int fd, struct stat *filestat,
                      unsigned long offset, unsigned long size,
                      unsigned int dsize, int endian);
int delayedtrigraph_filedesc(void *ctx, int fd, struct stat *filestat,
                             unsigned long offset, unsigned long size,
                             unsigned int dsize, int endian);
int tg_filedesc(int fd, struct stat *filestat, unsigned long offset,
                unsigned long size, unsigned int type, unsigned int dsize,
                int endian);
int set_projview();
int enableArrays(GLfloat *verts, GLfloat *cols, unsigned int vert_count);
void disableArrays();
int init_resources();
void onIdle();
int drawPoints();
void onDisplay();
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode,
						 int action, int mods);
void free_resources();
int trigraph_display(void *ctx);
int delayedtrigraph_display(void *ctx);
int tg_display();
int cleanup();

#endif
