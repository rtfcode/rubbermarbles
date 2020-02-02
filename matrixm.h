/*
 * Rubber Marbles - K Sheldrake
 * matrixm.h
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

#ifndef _MATRIXM_H
#define _MATRIXM_H

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLEW for GLfloat
#include <GL/glew.h>

#include <math.h>
#include <string.h>
#include "macro.h"


int printmatrix4(GLfloat m[][4], const char *msg);
int printmatrix4t(GLfloat m[][4], const char *msg);
int printmatrix3(GLfloat m[][3], const char *msg);
int printmatrix3t(GLfloat m[][3], const char *msg);
int printvec3(GLfloat v[], const char *msg);
int setidentitym4(GLfloat m[][4]);
int setemptym4(GLfloat m[][4]);
int copymatrix4(GLfloat d[][4], GLfloat s[][4]);
int copyvec3(GLfloat d[], GLfloat s[]);
int copymatrix4to3(GLfloat d[][3], GLfloat s[][4]);
int subv3(GLfloat v[], GLfloat a[], GLfloat b[]);
int multv3v3(GLfloat d[], GLfloat a[], GLfloat b[]);
int multv3f(GLfloat v[], GLfloat s);
int multm4f(GLfloat m[][4], GLfloat s);
int inversem4(GLfloat d[][4], GLfloat s[][4]);
int inversem3(GLfloat d[][3], GLfloat s[][3]);
int transposem4(GLfloat d[][4], GLfloat s[][4]);
int transposem3(GLfloat d[][3], GLfloat s[][3]);
int crossv3(GLfloat v[], GLfloat a[], GLfloat b[]);
GLfloat dotv3(GLfloat a[], GLfloat b[]);
int multm4v4(GLfloat vout[], GLfloat m[][4], GLfloat v[]);
int multm4v3(GLfloat vout[], GLfloat m[][4], GLfloat v[]);
int multm4(GLfloat m[][4], GLfloat a[][4], GLfloat b[][4]);
GLfloat magnitudev3(GLfloat v[]);
int normalisev3(GLfloat vout[], GLfloat v[]);
int ortho(GLfloat m[4][4], GLfloat l, GLfloat r, GLfloat b, GLfloat t,
          GLfloat n, GLfloat f);
GLfloat radians(GLfloat d);
int perspective(GLfloat m[][4], GLfloat fovy, GLfloat aspect, GLfloat n,
                GLfloat f);
int frustum(GLfloat m[][4], GLfloat x0, GLfloat x1, GLfloat y0,
            GLfloat y1, GLfloat z0, GLfloat z1);
int translatev3(GLfloat m[][4], GLfloat x, GLfloat y, GLfloat z);
int scalev3(GLfloat m[][4], GLfloat v[3]);
int rotatev3(GLfloat m[][4], GLfloat a, GLfloat v[3]);
int rotx(GLfloat m[][4], GLfloat a);
int roty(GLfloat m[][4], GLfloat a);
int rotz(GLfloat m[][4], GLfloat a);
int lookat(GLfloat m[][4], GLfloat eye[], GLfloat target[], GLfloat up[]);
int viewport(GLfloat m[][4], GLfloat x, GLfloat y, GLfloat w, GLfloat h);

#endif
