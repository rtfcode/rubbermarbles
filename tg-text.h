/*
** From the OpenGL Programming / Modern OpenGL Tutorial Text Rendering 01:
** https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01
*/

#ifndef _TEXT_H
#define _TEXT_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "macro.h"
#include "rb-conf.h"
#include "shader_utils.h"
#include "trigraph.h"

typedef struct point {
	GLfloat x;
	GLfloat y;
	GLfloat s;
	GLfloat t;
} point;

#endif

int init_text_resources();
float render_text(const char *text, float x, float y, float sx, float sy);
float display_text(const char *msg, unsigned int size, float x, float y, GLfloat colour[4]);
void free_text_resources();

