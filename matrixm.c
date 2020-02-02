/*
 * Rubber Marbles - K Sheldrake
 * matrixm.c
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 * 
 * Provides matrix maths functions - basically a reimplementation of
 * glm.
 */


#include "matrixm.h"

GLfloat identity4[4][4] = { {1.0, 0.0, 0.0, 0.0}
, {0.0, 1.0, 0.0, 0.0}
, {0.0, 0.0, 1.0, 0.0}
, {0.0, 0.0, 0.0, 1.0}
};
GLfloat empty4[4][4] = { {0.0, 0.0, 0.0, 0.0}
, {0.0, 0.0, 0.0, 0.0}
, {0.0, 0.0, 0.0, 0.0}
, {0.0, 0.0, 0.0, 0.0}
};


int printmatrix4(GLfloat m[][4], const char *msg)
{
    int i, j;

    if (!m) {
        FAIL_MSG("printmatrix4: invalid params\n");
        return 1;
    }


    if (msg) {
        printf("%s:\n", msg);
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            printf("%f ", m[i][j]);
        }
        printf("\n");
    }

    return 0;
}

int printmatrix4t(GLfloat m[][4], const char *msg)
{
    int i, j;

    if (!m) {
        FAIL_MSG("printmatrix4t: invalid params\n");
        return 1;
    }


    if (msg) {
        printf("%s:\n", msg);
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            printf("%f ", m[j][i]);
        }
        printf("\n");
    }

    return 0;
}

int printmatrix3(GLfloat m[][3], const char *msg)
{
    int i, j;

    if (!m) {
        FAIL_MSG("printmatrix3: invalid params\n");
        return 1;
    }


    if (msg) {
        printf("%s:\n", msg);
    }

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            printf("%f ", m[i][j]);
        }
        printf("\n");
    }

    return 0;
}

int printmatrix3t(GLfloat m[][3], const char *msg)
{
    int i, j;

    if (!m) {
        FAIL_MSG("printmatrix3t: invalid params\n");
        return 1;
    }


    if (msg) {
        printf("%s:\n", msg);
    }

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            printf("%f ", m[j][i]);
        }
        printf("\n");
    }

    return 0;
}

int printvec3(GLfloat v[], const char *msg)
{
    if (!v) {
        FAIL_MSG("printvec3: invalid params\n");
        return 1;
    }


    if (msg) {
        printf("%s:\n", msg);
    }

    printf("%f %f %f\n", v[0], v[1], v[2]);

    return 0;
}

int setidentitym4(GLfloat m[][4])
{
    if (!m) {
        FAIL_MSG("setidentitym4: invalid params\n");
        return 1;
    }


    if (copymatrix4(m, identity4) != 0) {
        FAIL_MSG("setidentifym4: copymatrix4() failed\n");
        return 2;
    }


    return 0;
}

int setemptym4(GLfloat m[][4])
{
    if (!m) {
        FAIL_MSG("setemptym4: invalid params\n");
        return 1;
    }


    if (copymatrix4(m, empty4) != 0) {
        FAIL_MSG("setemptym4: copymatrix4() failed\n");
        return 2;
    }


    return 0;
}


int copymatrix4(GLfloat d[][4], GLfloat s[][4])
{
    if (!d || !s) {
        FAIL_MSG("copymatrix4: invalid params\n");
        return 1;
    }


    memcpy(d, s, sizeof(GLfloat) * 16);

    return 0;
}

int copymatrix3(GLfloat d[][3], GLfloat s[][3])
{
    if (!d || !s) {
        FAIL_MSG("copymatrix3: invalid params\n");
        return 1;
    }


    memcpy(d, s, sizeof(GLfloat) * 9);

    return 0;
}

int copyvec3(GLfloat d[], GLfloat s[])
{
    if (!d || !s) {
        FAIL_MSG("copyvec3: invalid params\n");
        return 1;
    }


    memcpy(d, s, sizeof(GLfloat) * 3);

    return 0;
}

int copymatrix4to3(GLfloat d[][3], GLfloat s[][4])
{
    int i, j;

    if (!d || !s) {
        FAIL_MSG("copymatrix4to3: invalid params\n");
        return 1;
    }


    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            d[i][j] = s[i][j];
        }
    }

    return 0;
}

int subv3(GLfloat v[], GLfloat a[], GLfloat b[])
{
    int i;

    if (!v || !a || !b) {
        FAIL_MSG("subv3: invalid params\n");
        return 1;
    }


    for (i = 0; i < 3; i++) {
        v[i] = a[i] - b[i];
    }

    return 0;
}

int multv3v3(GLfloat d[], GLfloat a[], GLfloat b[])
{
    int i;

    if (!d || !a || !b) {
        FAIL_MSG("multv3v3: invalid params\n");
        return 1;
    }


    for (i = 0; i < 3; i++) {
        d[i] = a[i] * b[i];
    }

    return 0;
}

int multv3f(GLfloat v[], GLfloat s)
{
    int i;

    if (!v) {
        FAIL_MSG("multv3f: invalid params\n");
        return 1;
    }


    for (i = 0; i < 3; i++) {
        v[i] = v[i] * s;
    }

    return 0;
}

int multm4f(GLfloat m[][4], GLfloat s)
{
    int i;

    if (!m) {
        FAIL_MSG("multm4f: invalid params\n");
        return 1;
    }


    GLfloat *d = (GLfloat *) m;

    for (i = 0; i < 16; i++) {
        d[i] = d[i] * s;
    }

    return 0;
}

int inversem4(GLfloat d[][4], GLfloat s[][4])
{
    GLfloat t[4][4];

    if (!d || !s) {
        FAIL_MSG("inversem4: invalid params\n");
        return 1;
    }


    GLfloat SubFactor00 = s[2][2] * s[3][3] - s[3][2] * s[2][3];
    GLfloat SubFactor01 = s[2][1] * s[3][3] - s[3][1] * s[2][3];
    GLfloat SubFactor02 = s[2][1] * s[3][2] - s[3][1] * s[2][2];
    GLfloat SubFactor03 = s[2][0] * s[3][3] - s[3][0] * s[2][3];
    GLfloat SubFactor04 = s[2][0] * s[3][2] - s[3][0] * s[2][2];
    GLfloat SubFactor05 = s[2][0] * s[3][1] - s[3][0] * s[2][1];
    GLfloat SubFactor06 = s[1][2] * s[3][3] - s[3][2] * s[1][3];
    GLfloat SubFactor07 = s[1][1] * s[3][3] - s[3][1] * s[1][3];
    GLfloat SubFactor08 = s[1][1] * s[3][2] - s[3][1] * s[1][2];
    GLfloat SubFactor09 = s[1][0] * s[3][3] - s[3][0] * s[1][3];
    GLfloat SubFactor10 = s[1][0] * s[3][2] - s[3][0] * s[1][2];
    GLfloat SubFactor11 = s[1][1] * s[3][3] - s[3][1] * s[1][3];
    GLfloat SubFactor12 = s[1][0] * s[3][1] - s[3][0] * s[1][1];
    GLfloat SubFactor13 = s[1][2] * s[2][3] - s[2][2] * s[1][3];
    GLfloat SubFactor14 = s[1][1] * s[2][3] - s[2][1] * s[1][3];
    GLfloat SubFactor15 = s[1][1] * s[2][2] - s[2][1] * s[1][2];
    GLfloat SubFactor16 = s[1][0] * s[2][3] - s[2][0] * s[1][3];
    GLfloat SubFactor17 = s[1][0] * s[2][2] - s[2][0] * s[1][2];
    GLfloat SubFactor18 = s[1][0] * s[2][1] - s[2][0] * s[1][1];

    t[0][0] =
        +(s[1][1] * SubFactor00 - s[1][2] * SubFactor01 +
          s[1][3] * SubFactor02);
    t[1][0] =
        -(s[1][0] * SubFactor00 - s[1][2] * SubFactor03 +
          s[1][3] * SubFactor04);
    t[2][0] =
        +(s[1][0] * SubFactor01 - s[1][1] * SubFactor03 +
          s[1][3] * SubFactor05);
    t[3][0] =
        -(s[1][0] * SubFactor02 - s[1][1] * SubFactor04 +
          s[1][2] * SubFactor05);

    t[0][1] =
        -(s[0][1] * SubFactor00 - s[0][2] * SubFactor01 +
          s[0][3] * SubFactor02);
    t[1][1] =
        +(s[0][0] * SubFactor00 - s[0][2] * SubFactor03 +
          s[0][3] * SubFactor04);
    t[2][1] =
        -(s[0][0] * SubFactor01 - s[0][1] * SubFactor03 +
          s[0][3] * SubFactor05);
    t[3][1] =
        +(s[0][0] * SubFactor02 - s[0][1] * SubFactor04 +
          s[0][2] * SubFactor05);

    t[0][2] =
        +(s[0][1] * SubFactor06 - s[0][2] * SubFactor07 +
          s[0][3] * SubFactor08);
    t[1][2] =
        -(s[0][0] * SubFactor06 - s[0][2] * SubFactor09 +
          s[0][3] * SubFactor10);
    t[2][2] =
        +(s[0][0] * SubFactor11 - s[0][1] * SubFactor09 +
          s[0][3] * SubFactor12);
    t[3][2] =
        -(s[0][0] * SubFactor08 - s[0][1] * SubFactor10 +
          s[0][2] * SubFactor12);

    t[0][3] =
        -(s[0][1] * SubFactor13 - s[0][2] * SubFactor14 +
          s[0][3] * SubFactor15);
    t[1][3] =
        +(s[0][0] * SubFactor13 - s[0][2] * SubFactor16 +
          s[0][3] * SubFactor17);
    t[2][3] =
        -(s[0][0] * SubFactor14 - s[0][1] * SubFactor16 +
          s[0][3] * SubFactor18);
    t[3][3] =
        +(s[0][0] * SubFactor15 - s[0][1] * SubFactor17 +
          s[0][2] * SubFactor18);

    GLfloat Determinant = +s[0][0] * t[0][0]
        + s[0][1] * t[1][0] + s[0][2] * t[2][0] + s[0][3] * t[3][0];

    if (Determinant == 0.0) {
        FAIL_MSG("inversem4: Determinant is 0.0\n");
        return 2;
    }


    if (multm4f(t, 1 / Determinant) != 0) {
        FAIL_MSG("inversem4: multm4f() failed\n");
        return 3;
    }


    if (copymatrix4(d, t) != 0) {
        FAIL_MSG("inversem4: copymatrix4() failed\n");
        return 4;
    }


    return 0;
}

int inversem3(GLfloat d[][3], GLfloat s[][3])
{
    GLfloat t[3][3];

    if (!d || !s) {
        FAIL_MSG("inversem3: invalid params\n");
        return 1;
    }


    GLfloat D = +s[0][0] * (s[1][1] * s[2][2] - s[2][1] * s[1][2])
        - s[1][0] * (s[0][1] * s[2][2] - s[2][1] * s[0][2])
        + s[2][0] * (s[0][1] * s[1][2] - s[1][1] * s[0][2]);

    if (D == 0.0) {
        FAIL_MSG("inversem3: D is 0.0\n");
        return 2;
    }


    t[0][0] = +(s[1][1] * s[2][2] - s[1][2] * s[2][1]) / D;
    t[0][1] = -(s[0][1] * s[2][2] - s[0][2] * s[2][1]) / D;
    t[0][2] = +(s[0][1] * s[1][2] - s[0][2] * s[1][1]) / D;
    t[1][0] = -(s[1][0] * s[2][2] - s[1][2] * s[2][0]) / D;
    t[1][1] = +(s[0][0] * s[2][2] - s[0][2] * s[2][0]) / D;
    t[1][2] = -(s[0][0] * s[1][2] - s[0][2] * s[1][0]) / D;
    t[2][0] = +(s[1][0] * s[2][1] - s[1][1] * s[2][0]) / D;
    t[2][1] = -(s[0][0] * s[2][1] - s[0][1] * s[2][0]) / D;
    t[2][2] = +(s[0][0] * s[1][1] - s[0][1] * s[1][0]) / D;

    if (copymatrix3(d, t) != 0) {
        FAIL_MSG("inversem3: copymatrix3() failed\n");
        return 2;
    }


    return 0;
}

int transposem4(GLfloat d[][4], GLfloat s[][4])
{
    int i, j;
    GLfloat t[4][4];

    if (!d || !s) {
        FAIL_MSG("transposem4: invalid params\n");
        return 1;
    }


    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            t[i][j] = s[j][i];
        }
    }

    if (copymatrix4(d, t) != 0) {
        FAIL_MSG("transposem4: copymatrix() failed\n");
        return 2;
    }


    return 0;
}

int transposem3(GLfloat d[][3], GLfloat s[][3])
{
    int i, j;
    GLfloat t[3][3];

    if (!d || !s) {
        FAIL_MSG("transposem3: invalid params\n");
        return 1;
    }


    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            t[i][j] = s[j][i];
        }
    }

    if (copymatrix3(d, t) != 0) {
        FAIL_MSG("transposem3: copymatrix3() failed\n");
        return 2;
    }


    return 0;
}

int crossv3(GLfloat v[], GLfloat a[], GLfloat b[])
{
    GLfloat t[3];

    if (!v || !a || !b) {
        FAIL_MSG("crossv3: invalid params\n");
        return 1;
    }


    t[0] = (a[1] * b[2]) - (a[2] * b[1]);
    t[1] = (a[2] * b[0]) - (a[0] * b[2]);
    t[2] = (a[0] * b[1]) - (a[1] * b[0]);

    if (copyvec3(v, t) != 0) {
        FAIL_MSG("crossv3: copyvec3() failed\n");
        return 2;
    }


    return 0;
}

GLfloat dotv3(GLfloat a[], GLfloat b[])
{
    if (!a || !b) {
        FAIL_MSG("dotv3: invalid params\n");
        return 0.0;
    }


    return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

int multm4v4(GLfloat vout[], GLfloat m[][4], GLfloat v[])
{
    int i;
    GLfloat t[4];

    if (!vout || !m || !v) {
        FAIL_MSG("multm4v4: invalid params\n");
        return 1;
    }


    for (i = 0; i < 4; i++) {
        t[i] =
            (m[i][0] * v[0]) + (m[i][1] * v[1]) + (m[i][2] * v[2]) +
            (m[i][3] * v[3]);
    }

    if (copyvec3(vout, t) != 0) {
        FAIL_MSG("multm4v4: copyvec3() failed\n");
        return 2;
    }


    return 0;
}

int multm4v3(GLfloat vout[], GLfloat m[][4], GLfloat v[])
{
    int i;
    GLfloat t[3];

    if (!vout || !m || !v) {
        FAIL_MSG("multm4v3: invalid params\n");
        return 1;
    }


    for (i = 0; i < 3; i++) {
        t[i] =
            (m[i][0] * v[0]) + (m[i][1] * v[1]) + (m[i][2] * v[2]) +
            m[i][3];
    }

    if (copyvec3(vout, t) != 0) {
        FAIL_MSG("multm4v3: copyvec3() failed\n");
        return 2;
    }


    return 0;
}

int multm4(GLfloat m[][4], GLfloat a[][4], GLfloat b[][4])
{
    int i, j, k;
    GLfloat t[4][4];

    if (!m || !a || !b) {
        FAIL_MSG("multm4: invalid params\n");
        return 1;
    }


    if (copymatrix4(t, empty4) != 0) {
        FAIL_MSG("multm4: copymatrix4() failed\n");
        return 2;
    }


    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 4; k++) {
                t[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    if (copymatrix4(m, t) != 0) {
        FAIL_MSG("multm4: copymatrix4() failed\n");
        return 3;
    }


    return 0;
}

GLfloat magnitudev3(GLfloat v[])
{
    if (!v) {
        FAIL_MSG("magnitudev3: invalid params\n");
        return 0.0;
    }


    return sqrt((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
}

int normalisev3(GLfloat vout[], GLfloat v[])
{
    int i;

    if (!vout || !v) {
        FAIL_MSG("normalisev3: invalid params\n");
        return 1;
    }


    GLfloat m = magnitudev3(v);

    if (m == 0.0) {
        FAIL_MSG("normalisev3: magnitudev3() failed\n");
        return 2;
    }


    for (i = 0; i < 3; i++) {
        vout[i] = v[i] / m;
    }

    return 0;
}

int
ortho(GLfloat m[4][4], GLfloat l, GLfloat r, GLfloat b, GLfloat t,
      GLfloat n, GLfloat f)
{
    GLfloat dx = r - l;
    GLfloat dy = t - b;
    GLfloat dz = f - n;

    if (!m || (dx == 0.0) || (dy == 0.0) || (dz == 0.0)) {
        FAIL_MSG("ortho: invalid params\n");
        return 1;
    }


    GLfloat rx = -(r + l) / dx;
    GLfloat ry = -(t + b) / dy;
    GLfloat rz = -(f + n) / dz;

    if (copymatrix4(m, empty4) != 0) {
        FAIL_MSG("ortho: copymatrix4() failed\n");
        return 2;
    }


    m[0][0] = 2.0 / dx;
    m[0][3] = rx;
    m[1][1] = 2.0 / dy;
    m[1][3] = ry;
    m[2][2] = -2.0 / dz;
    m[2][3] = rz;
    m[3][3] = 1.0;

    return 0;
}

GLfloat radians(GLfloat d)
{
    return (d * M_PI / 180.0);
}

int
perspective(GLfloat m[][4], GLfloat fovy, GLfloat aspect, GLfloat n,
            GLfloat f)
{
    GLfloat s = 1.0 / tan(fovy / 2.0);
    GLfloat sx = s / aspect;
    GLfloat sy = s;
    GLfloat zz = (f + n) / (n - f);
    GLfloat zw = 2.0 * f * n / (n - f);

    if (!m) {
        FAIL_MSG("perspective: invalid params\n");
        return 1;
    }


    if (copymatrix4(m, empty4) != 0) {
        FAIL_MSG("perspective: copymatrix4() failed\n");
        return 2;
    }


    m[0][0] = sx;
    m[1][1] = sy;
    m[2][2] = zz;
    m[2][3] = zw;
    m[3][2] = -1.0;

    return 0;
}

int
frustum(GLfloat m[][4], GLfloat x0, GLfloat x1, GLfloat y0,
        GLfloat y1, GLfloat z0, GLfloat z1)
{
    GLfloat a = (x1 + x0) / (x1 - x0);
    GLfloat b = (y1 + y0) / (y1 - y0);
    GLfloat c = -(z1 + z0) / (z1 - z0);
    GLfloat d = -2.0 * z1 * z0 / (z1 - z0);
    GLfloat sx = 2.0 * z0 / (x1 - x0);
    GLfloat sy = 2.0 * z0 / (y1 - y0);

    if (!m) {
        FAIL_MSG("frustum: invalid params\n");
        return 1;
    }


    if (copymatrix4(m, empty4) != 0) {
        FAIL_MSG("frustum: copymatrix4() failed\n");
        return 2;
    }


    m[0][0] = sx;
    m[0][2] = a;
    m[1][1] = sy;
    m[1][2] = b;
    m[2][2] = c;
    m[2][3] = d;
    m[3][2] = -1.0;

    return 0;
}

int translatev3(GLfloat m[][4], GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat t[4][4];
    GLfloat t2[4][4];

    if (!m) {
        FAIL_MSG("translatev3: invalid params\n");
        return 1;
    }


    if (copymatrix4(t, identity4) != 0) {
        FAIL_MSG("translatev3: copymatrix4() failed\n");
        return 2;
    }


    t[0][3] = x;
    t[1][3] = y;
    t[2][3] = z;

    if (copymatrix4(t2, m) != 0) {
        FAIL_MSG("translatev3: copymatrix4() failed\n");
        return 3;
    }


    if (multm4(m, t2, t) != 0) {
        FAIL_MSG("translatev3: multm4() failed\n");
        return 4;
    }


    return 0;
}

int scalev3(GLfloat m[][4], GLfloat v[3])
{
    GLfloat t[4][4];
    GLfloat t2[4][4];

    if (!m || !v) {
        FAIL_MSG("scalev3: invalid params\n");
        return 1;
    }


    if (copymatrix4(t, identity4) != 0) {
        FAIL_MSG("scalev3: copymatrix4() failed\n");
        return 2;
    }


    t[0][0] = v[0];
    t[1][1] = v[1];
    t[2][2] = v[2];

    if (copymatrix4(t2, m) != 0) {
        FAIL_MSG("scalev3: copymatrix4() failed\n");
        return 2;
    }


    if (multm4(m, t2, t) != 0) {
        FAIL_MSG("scalev3: multm4() failed\n");
        return 3;
    }


    return 0;
}

int rotatev3(GLfloat m[][4], GLfloat a, GLfloat v[3])
{
    GLfloat vnorm[3];
    GLfloat t[4][4];
    GLfloat t2[4][4];

    if (!m || !v) {
        FAIL_MSG("rotatev3: invalid params\n");
        return 1;
    }


    if (normalisev3(vnorm, v) != 0) {
        FAIL_MSG("rotatev3: normalisev3() failed\n");
        return 2;
    }


    GLfloat x = vnorm[0];
    GLfloat y = vnorm[1];
    GLfloat z = vnorm[2];
    GLfloat s = sin(radians(a));
    GLfloat c = cos(radians(a));
    GLfloat nc = 1.0 - c;

    if (copymatrix4(t, identity4) != 0) {
        FAIL_MSG("rotatev3: copymatrix4() failed\n");
        return 3;
    }


    t[0][0] = (x * x * nc) + c;
    t[0][1] = (x * y * nc) - (z * s);
    t[0][2] = (x * z * nc) + (y * s);
    t[1][0] = (y * x * nc) + (z * s);
    t[1][1] = (y * y * nc) + c;
    t[1][2] = (y * z * nc) - (x * s);
    t[2][0] = (x * z * nc) - (y * s);
    t[2][1] = (y * z * nc) + (x * s);
    t[2][2] = (z * z * nc) + c;

    if (copymatrix4(t2, m) != 0) {
        FAIL_MSG("rotatev3: copymatrix4() failed\n");
        return 4;
    }


    if (multm4(m, t2, t) != 0) {
        FAIL_MSG("rotatev3: multm4() failed\n");
        return 5;
    }


    return 0;
}

int rotx(GLfloat m[][4], GLfloat a)
{
    GLfloat t[4][4];
    GLfloat t2[4][4];
    GLfloat s = sin(radians(a));
    GLfloat c = cos(radians(a));

    if (!m) {
        FAIL_MSG("rotx: invalid params\n");
        return 1;
    }


    if (copymatrix4(t, identity4) != 0) {
        FAIL_MSG("rotx: copymatrix4() failed\n");
        return 2;
    }


    t[1][1] = c;
    t[1][2] = -s;
    t[2][1] = s;
    t[2][2] = c;

    if (copymatrix4(t2, m) != 0) {
        FAIL_MSG("rotx: copymatrix4() failed\n");
        return 3;
    }


    if (multm4(m, t2, t) != 0) {
        FAIL_MSG("rotx: multm4() failed\n");
        return 4;
    }


    return 0;
}


int roty(GLfloat m[][4], GLfloat a)
{
    GLfloat t[4][4];
    GLfloat t2[4][4];
    GLfloat s = sin(radians(a));
    GLfloat c = cos(radians(a));

    if (!m) {
        FAIL_MSG("roty: invalid params\n");
        return 1;
    }


    if (copymatrix4(t, identity4) != 0) {
        FAIL_MSG("roty: copymatrix4() failed\n");
        return 2;
    }


    t[0][0] = c;
    t[0][2] = s;
    t[2][0] = -s;
    t[2][2] = c;

    if (copymatrix4(t2, m) != 0) {
        FAIL_MSG("roty: copymatrix4() failed\n");
        return 3;
    }


    if (multm4(m, t2, t) != 0) {
        FAIL_MSG("roty: multm4() failed\n");
        return 4;
    }


    return 0;
}


int rotz(GLfloat m[][4], GLfloat a)
{
    GLfloat t[4][4];
    GLfloat t2[4][4];
    GLfloat s = sin(radians(a));
    GLfloat c = cos(radians(a));

    if (!m) {
        FAIL_MSG("rotz: invalid params\n");
        return 1;
    }


    if (copymatrix4(t, identity4) != 0) {
        FAIL_MSG("rotz: copymatrix4() failed\n");
        return 2;
    }


    t[0][0] = c;
    t[0][1] = -s;
    t[1][0] = s;
    t[1][1] = c;

    if (copymatrix4(t2, m) != 0) {
        FAIL_MSG("rotz: copymatrix4() failed\n");
        return 3;
    }


    if (multm4(m, t2, t) != 0) {
        FAIL_MSG("rotz: multm4() failed\n");
        return 4;
    }


    return 0;
}

int lookat(GLfloat m[][4], GLfloat eye[], GLfloat target[], GLfloat up[])
{
    int i;

    GLfloat F[3];
    GLfloat f[3];
    GLfloat s[3];
    GLfloat S[3];
    GLfloat u[3];

    if (!m || !eye || !target || !up) {
        FAIL_MSG("lookat: invalid params\n");
        return 1;
    }


    if (subv3(F, target, eye) != 0) {
        FAIL_MSG("lookat: subv3() failed\n");
        return 2;
    }


    if (normalisev3(f, F) != 0) {
        FAIL_MSG("lookat: normalisev3() failed\n");
        return 3;
    }


    if (crossv3(S, f, up) != 0) {
        FAIL_MSG("lookat: crossv3() failed\n");
        return 4;
    }


    if (normalisev3(s, S) != 0) {
        FAIL_MSG("lookat: normalisev3() failed\n");
        return 5;
    }


    if (crossv3(u, s, f) != 0) {
        FAIL_MSG("lookat: crossv3() failed\n");
        return 6;
    }


    if (copymatrix4(m, identity4) != 0) {
        FAIL_MSG("lookat: copymatrix4() failed\n");
        return 7;
    }


    for (i = 0; i < 3; i++) {
        m[0][i] = s[i];
        m[1][i] = u[i];
        m[2][i] = -f[i];
    }

    m[0][3] = -dotv3(s, eye);
    m[1][3] = -dotv3(u, eye);
    m[2][3] = dotv3(f, eye);

    return 0;
}

int viewport(GLfloat m[][4], GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
    if (!m) {
        FAIL_MSG("viewport: invalid params\n");
        return 1;
    }


    if (copymatrix4(m, identity4) != 0) {
        FAIL_MSG("viewport: copymatrix4()\n");
        return 2;
    }


    m[0][0] = w / 2.0;
    m[0][3] = x + (w / 2.0);
    m[1][1] = h / 2.0;
    m[1][3] = y + (h / 2.0);

    return 0;
}
