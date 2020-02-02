/*
 * Rubber Marbles - K Sheldrake
 * shader_utils.h
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

#ifndef _SHADER_UTILS_H
#define _SHADER_UTILS_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "macro.h"

char *file_read(const char *filename);
void print_log(GLuint object);
GLuint create_shader(const char *filename, GLenum type);
GLuint create_program(const char *vertexfile, const char *fragmentfile);
GLuint create_gs_program(const char *vertexfile, const char *geometryfile,
                         const char *fragmentfile, GLint input,
                         GLint output, GLint vertices);
GLint get_attrib(GLuint program, const char *name);
GLint get_uniform(GLuint program, const char *name);
#endif
