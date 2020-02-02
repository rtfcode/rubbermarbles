/*
 * Rubber Marbles - K Sheldrake
 * shader_utils.c
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 * 
 * Provides functions to load shaders.
 *
 * From the OpenGL Programming wikibook: http://en.wikibooks.org/wiki/OpenGL_Programming
 * This file is in the public domain.
 * Contributors: Sylvain Beucler
 *
 * Now with added error checking.
 *
 * Modified to use in-line shaders rather than external files.
 */


#include "shader_utils.h"

/* shader source */
const GLchar *v_glsl =
    "attribute vec3 coord3d;"
    "attribute vec3 v_color;"
    ""
    "uniform mat4 mvp;"
    "uniform mat4 model;"
    "uniform int colset;"
    ""
    "varying vec3 f_color;"
    ""
    "void main(void) {"
    ""
    "  gl_Position = mvp * vec4(coord3d, 1.0);"
    ""
    "  if (colset == 0) {"
    "    f_color = v_color;"
    "  } else if (colset == 1) {"
    "    f_color = vec3(v_color.r, v_color.r, v_color.r);"
    "  } else {" "    f_color = vec3(0.0, 0.0, 1.0);" "  }" "" "}";

const GLchar *f_glsl =
    "varying vec3 f_color;"
    ""
    ""
    "void main(void) {"
    ""
    "  gl_FragColor = vec4(f_color.x, f_color.y, f_color.z, 1.0);" "" "}";

const GLchar *text_v_glsl =
    "attribute vec4 coord;"
    "varying vec2 texpos;"
    ""
    "void main(void) {"
    "  gl_Position = vec4(coord.xy, 0, 1);"
    "  texpos = coord.zw;"
    "}"
    "";

const GLchar *text_f_glsl =
    "varying vec2 texpos;"
    "uniform sampler2D tex;"
    "uniform vec4 color;"
    ""
    "void main(void) {"
    "  gl_FragColor = vec4(1, 1, 1, texture2D(tex, texpos).a) * color;"
    ""
    "}";



/**
 * Store all the file's contents in memory, useful to pass shaders
 * source code to OpenGL
 */
char *file_read(const char *filename)
{
    if (!filename) {
        FAIL_MSG("file_read: invalid params\n");
        return NULL;
    }


    FILE *in = fopen(filename, "rb");
    if (!in) {
        FAIL_MSG("file_read: fopen() failed\n");
        return NULL;
    }


    int res_size = BUFSIZ;
    char *res = (char *) malloc(res_size);
    if (!res) {
        FAIL_MSG("file_read: malloc() failed\n");
        return NULL;
    }

    int nb_read_total = 0;

    while (!feof(in) && !ferror(in)) {
        if (nb_read_total + BUFSIZ > res_size) {
            if (res_size > 10 * 1024 * 1024)
                break;
            res_size = res_size * 2;
            res = (char *) realloc(res, res_size);
            if (!res) {
                FAIL_MSG("file_read: realloc() failed\n");
                return NULL;
            }

        }
        char *p_res = res + nb_read_total;
        nb_read_total += fread(p_res, 1, BUFSIZ, in);
    }

    fclose(in);
    res = (char *) realloc(res, nb_read_total + 1);
    if (!res) {
        FAIL_MSG("file_read: realloc() failed\n");
        return NULL;
    }

    res[nb_read_total] = '\0';
    return res;
}


/**
 * Display compilation errors from the OpenGL shader compiler
 */
void print_log(GLuint object)
{
    GLint log_length = 0;
    if (glIsShader(object))
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else if (glIsProgram(object))
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else {
        fprintf(stderr, "print_log: Not a shader or a program\n");
        return;
    }

    char *log = (char *) malloc(log_length);
    if (!log) {
        FAIL_MSG("print_log: malloc() failed\n");
        return;
    }


    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, log);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, log);

    fprintf(stderr, "%s", log);
    free(log);
}


/**
 * Compile the shader from file 'filename', with error handling
 */
GLuint create_shader(const char *filename, GLenum type)
{
    if (!filename) {
        FAIL_MSG("create_shader: invalid params\n");
        return 0;
    }

//    const GLchar *source = file_read(filename);
    const GLchar *source;

    if (!strncmp(filename, "trigraph.v.glsl", 16)) {
        source = v_glsl;
    } else if (!strncmp(filename, "trigraph.f.glsl", 16)) {
        source = f_glsl;
    } else if (!strncmp(filename, "text.v.glsl", 12)) {
        source = text_v_glsl;
    } else if (!strncmp(filename, "text.f.glsl", 12)) {
        source = text_f_glsl;
    } else {
        fprintf(stderr, "create_shader: invalid file\n");
        return 0;
    }

    if (!source) {
        FAIL_ERR("create_shader: file_read() failed\n");
        return 0;
    }

    GLuint res = glCreateShader(type);
    if (!res) {
        FAIL_MSG("create_shader: glCreateShader() failed\n");
        return 0;
    }

    const GLchar *sources[] = {
        // Define GLSL version
#ifdef GL_ES_VERSION_2_0
        "#version 100\n"        // OpenGL ES 2.0
#else
        "#version 120\n"        // OpenGL 2.1
#endif
            ,
        // GLES2 precision specifiers
#ifdef GL_ES_VERSION_2_0
        // Define default float precision for fragment shaders:
        (type == GL_FRAGMENT_SHADER) ?
            "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
            "precision highp float;           \n"
            "#else                            \n"
            "precision mediump float;         \n"
            "#endif                           \n" : ""
            // Note: OpenGL ES automatically defines this:
            // #define GL_ES
#else
        // Ignore GLES 2 precision specifiers:
        "#define lowp   \n" "#define mediump\n" "#define highp  \n"
#endif
            ,
        source
    };
    glShaderSource(res, 3, sources, NULL);
//    free((void *) source);

    glCompileShader(res);
    GLint compile_ok = GL_FALSE;
    glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
    if (compile_ok == GL_FALSE) {
        fprintf(stderr, "%s:", filename);
        print_log(res);
        glDeleteShader(res);
        return 0;
    }

    return res;
}


/* create program */
GLuint create_program(const char *vertexfile, const char *fragmentfile)
{
    if (!vertexfile || !fragmentfile) {
        FAIL_MSG("create_program: invalid params\n");
        return 0;
    }


    GLuint program = glCreateProgram();
    GLuint shader;

    if (!program) {
        FAIL_MSG("create_program: glCreateProgram() failed\n");
        return 0;
    }


    if (vertexfile) {
        shader = create_shader(vertexfile, GL_VERTEX_SHADER);
        if (!shader) {
            FAIL_MSG("create_program: create_shader() failed\n");
            return 0;
        }

        glAttachShader(program, shader);
    }

    if (fragmentfile) {
        shader = create_shader(fragmentfile, GL_FRAGMENT_SHADER);
        if (!shader) {
            FAIL_MSG("create_program: create_shader() failed\n");
            return 0;
        }

        glAttachShader(program, shader);
    }

    glLinkProgram(program);
    GLint link_ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        fprintf(stderr, "glLinkProgram:");
        print_log(program);
        glDeleteProgram(program);
        return 0;
    }

    return program;
}


/* geometry shader */
#ifdef GL_GEOMETRY_SHADER
GLuint
create_gs_program(const char *vertexfile, const char *geometryfile,
                  const char *fragmentfile, GLint input,
                  GLint output, GLint vertices)
{
    if (!vertexfile || !geometryfile || !fragmentfile) {
        FAIL_MSG("create_gs_program: invalid params\n");
        return 0;
    }


    GLuint program = glCreateProgram();
    GLuint shader;

    if (!program) {
        FAIL_MSG("create_gs_program: glCreateProgram() failed\n");
        return 0;
    }


    if (vertexfile) {
        shader = create_shader(vertexfile, GL_VERTEX_SHADER);
        if (!shader) {
            FAIL_MSG("create_gs_program: create_shader() failed\n");
            return 0;
        }

        glAttachShader(program, shader);
    }

    if (geometryfile) {
        shader = create_shader(geometryfile, GL_GEOMETRY_SHADER);
        if (!shader) {
            FAIL_MSG("create_gs_program: create_shader() failed\n");
            return 0;
        }

        glAttachShader(program, shader);

        glProgramParameteriEXT(program, GL_GEOMETRY_INPUT_TYPE_EXT, input);
        glProgramParameteriEXT(program, GL_GEOMETRY_OUTPUT_TYPE_EXT,
                               output);
        glProgramParameteriEXT(program, GL_GEOMETRY_VERTICES_OUT_EXT,
                               vertices);
    }

    if (fragmentfile) {
        shader = create_shader(fragmentfile, GL_FRAGMENT_SHADER);
        if (!shader) {
            FAIL_MSG("create_gs_program: create_shader() failed\n");
            return 0;
        }

        glAttachShader(program, shader);
    }

    glLinkProgram(program);
    GLint link_ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        fprintf(stderr, "glLinkProgram:");
        print_log(program);
        glDeleteProgram(program);
        return 0;
    }

    return program;
}
#else
GLuint
create_gs_program(const char *vertexfile, const char *geometryfile,
                  const char *fragmentfile, GLint input,
                  GLint output, GLint vertices)
{
    fprintf(stderr, "Missing support for geometry shaders.\n");
    return 0;
}
#endif


/* pointer to shader attribute */
GLint get_attrib(GLuint program, const char *name)
{
    if (!program || !name) {
        FAIL_MSG("get_attrib: invalid params\n");
        return -1;
    }


    GLint attribute = glGetAttribLocation(program, name);
    if (attribute == -1) {
        FAIL_MSG("get_attrib: glGetAttribLocation() failed\n");
        return -1;
    }

    return attribute;
}


/* pointer to uniform */
GLint get_uniform(GLuint program, const char *name)
{
    if (!program || !name) {
        FAIL_MSG("get_uniform: invalid params\n");
        return -1;
    }


    GLint uniform = glGetUniformLocation(program, name);
    if (uniform == -1) {
        FAIL_MSG("get_uniform: glGetUniformLocation() failed\n");
        return -1;
    }

    return uniform;
}
