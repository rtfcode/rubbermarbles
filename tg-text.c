/*
** From the OpenGL Programming / Modern OpenGL Tutorial Text Rendering 01:
** https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01
*/

#include "tg-text.h"

GLuint textprogram;
GLint attribute_coord;
GLint uniform_tex;
GLint uniform_color;
GLuint vbo;

FT_Library ft;
FT_Face face;
extern struct tg_ctx *ctx;

char fontfilename[PATH_MAX] = "";

int init_text_resources()
{
    char *homepath = NULL;

	homepath = getenv("HOME");
    if (!homepath) {
        FAIL_MSG("init_text_resources: HOME env var not set\n");
        return 0;
    }

	if (configuration(homepath, "glfont", fontfilename) != 0) {
		FAIL_MSG("init_text_resources: configuration() failed\n");
		return 0;
	}
	
	/* Initialize the FreeType2 library */
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "Could not init freetype library\n");
		return 0;
	}

	/* Load a font */
	if (FT_New_Face(ft, fontfilename, 0, &face)) {
		fprintf(stderr, "Could not open font %s\n", fontfilename);
		return 0;
	}

	textprogram = create_program("text.v.glsl", "text.f.glsl");
	if(textprogram == 0)
		return 0;

	attribute_coord = get_attrib(textprogram, "coord");
	uniform_tex = get_uniform(textprogram, "tex");
	uniform_color = get_uniform(textprogram, "color");

	if ((attribute_coord == -1) || (uniform_tex == -1) || (uniform_color == -1))
		return 0;

	// Create the vertex buffer object
	glGenBuffers(1, &vbo);

	return 1;
}

/**
 * Render text using the currently loaded font and currently set font size.
 * Rendering starts at coordinates (x, y), z is always 0.
 * The pixel coordinates that the FreeType2 library uses are scaled by (sx, sy).
 */
float render_text(const char *text, float x, float y, float sx, float sy) {
	const char *p;
	FT_GlyphSlot g = face->glyph;

	/* Create a texture that will be used to hold one "glyph" */
	GLuint tex;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(uniform_tex, 0);

	/* We require 1 byte alignment when uploading texture data */
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/* Clamping to edges is important to prevent artifacts when scaling */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Linear filtering usually looks best for text */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/* Set up the VBO for our vertex data */
	glEnableVertexAttribArray(attribute_coord);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/* Loop through all characters */
	for (p = text; *p; p++) {
		/* Try to load and render the character */
		if (FT_Load_Char(face, *p, FT_LOAD_RENDER))
			continue;

		/* Upload the "bitmap", which contains an 8-bit grayscale image, as an alpha texture */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, g->bitmap.width, g->bitmap.rows, 0, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		/* Calculate the vertex and texture coordinates */
		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		point box[4] = {
			{x2, -y2, 0, 0},
			{x2 + w, -y2, 1, 0},
			{x2, -y2 - h, 0, 1},
			{x2 + w, -y2 - h, 1, 1},
		};

		/* Draw the character on the screen */
		glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		/* Advance the cursor to the start of the next character */
		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}

	glDisableVertexAttribArray(attribute_coord);
	glDeleteTextures(1, &tex);

    return x;
}

/* display some text at a location in a certain colour.
   x and y are relative to top left corner in screen pixels.
   colour can be transparent.
   This function was the original onDisplay function with a
   few amends to make it fit with other programs.
*/
float display_text(const char *msg, unsigned int size, float x, float y, GLfloat colour[4])
{
	float sx = 2.0 / ctx->screen_width;
	float sy = 2.0 / ctx->screen_height;
    float xres;

	glUseProgram(textprogram);

	/* Enable blending, necessary for our alpha texture */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Set font size to 48 pixels, color to colour */
	FT_Set_Pixel_Sizes(face, 0, size);
	glUniform4fv(uniform_color, 1, colour);

	/* Effects of alignment */
	xres = render_text(msg, -1 + x * sx, 1 - y * sy, sx, sy);

    return (xres + 1) / sx;


}

void free_text_resources()
{
	glDeleteProgram(textprogram);
}


