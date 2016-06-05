/*
Copyright (c) 2015-2016 Steven Stanfield

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would
   be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not
   be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
   distribution.
*/
#include "rendertext.h"
#include "pc/GL/glew.h"
#include "../util/string.h"
#include "../state/gtypes.h"
#include "../render/shaders.h"
#include "distancemap.h"

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
  float ax; // advance.x
  float ay; // advance.y

  float bw; // bitmap.width;
  float bh; // bitmap.rows;

  float bl; // bitmap_left;
  float bt; // bitmap_top;

  float tx; // x offset of glyph in texture coordinates
  float ty; // y offset of glyph in texture coordinates
} charInfo;

typedef struct {
	int    atlas_width;
	int    atlas_height;
	int    maxWidth;
	int    maxHeight;
	int    bitmapXY;
	float  px;
	GLuint tex;

	charInfo c[128];
} _FontContext;


static Bool initted = FALSE;
static GLuint prog;
static GLuint square;
static GLuint squareuv;
static GLuint indices;
static int windowWidth;
static int windowHeight;

static GLfloat square_uv_buffer_data[] = {
    0.0f,  1.0f,    // left top
    1.0f,  1.0f,    // right top
    1.0f,  0.0f,    // right bottom
    0.0f,  0.0f,    // left bottom
};
static const short squareindices[] = { 2,1,0,
                                       3,2,0};
void setWindowWH(int width, int height) {
	windowWidth = width;
	windowHeight = height;
}

static int powTwoAbove(int t) {
	int i = 2;
	while (i < t) {
		i *= 2;
	}
	return i;
}

FontContext renderTextInit(char *fontFile, float pointSize, const char *assetDir) {
	_FontContext *_ctx = (_FontContext *)malloc(sizeof(_FontContext));
	if (!_ctx) return NULL;
	memset(_ctx, 0, sizeof(_FontContext));
	_ctx->px = pointSize;
	int error;
	FT_Library library;
	FT_Face    face;

	error = FT_Init_FreeType( &library );
	if ( error ) {
		printf("ERROR: Failed to open font.\n");
		free(_ctx);
		return NULL;
	}
	error = FT_New_Face( library,
	                     fontFile,
	                     0,
	                     &face );
	if ( error == FT_Err_Unknown_File_Format )
	{
		printf("ERROR: Unknown font file format.\n");
		FT_Done_FreeType(library);
		free(_ctx);
		return NULL;
		// the font file could be opened and read, but it appears
		// that its font format is unsupported
	} else if ( error ) {
		printf("ERROR: Unable to load font face.\n");
		FT_Done_FreeType(library);
		free(_ctx);
		return NULL;
		// another error code means that the font file could not
		// be opened or read, or that it is broken...
	}
	error = FT_Set_Pixel_Sizes(
	          face,   /* handle to face object */
	          0,      /* pixel_width           */
	          _ctx->px);   /* pixel_height          */
	if ( error ) {
		printf("ERROR: Failed to set font size.\n");
		FT_Done_Face(face);
		FT_Done_FreeType(library);
		free(_ctx);
		return NULL;
	}

	FT_GlyphSlot g = face->glyph;
	int w = 0;
	int maxW = 0;
	int maxAdvX = 0;
	int maxAdvY = 0;
	int h = 0;
	int i;
	for(i = 32; i < 128; i++) {
		if(FT_Load_Char(face, i, FT_LOAD_RENDER)) {
			fprintf(stderr, "Loading character %c failed!\n", i);
			continue;
		}
//		printf("XXX glyph: %c w: %d, h: %d\n", (char)i, g->bitmap.width, g->bitmap.rows);
		w += g->bitmap.width + 3;
		h = h>g->bitmap.rows?h:g->bitmap.rows;
		maxW = maxW>g->bitmap.width?maxW:g->bitmap.width;
		maxAdvX = maxAdvX>g->advance.x?maxAdvX:g->advance.x;
		maxAdvY = maxAdvY>g->advance.y?maxAdvY:g->advance.y;
	}

	_ctx->bitmapXY = powTwoAbove(maxW<h?h:maxW);
	printf("use %d x %d bitmap\n", _ctx->bitmapXY, _ctx->bitmapXY);

	_ctx->maxWidth = maxW;
	_ctx->maxHeight = h;
	glGenTextures(1, &_ctx->tex);
	glBindTexture(GL_TEXTURE_2D, _ctx->tex);

	// XXX TODO, save and restore this?
	// Need this to use a 1 byte texture format.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// XXX TODO- add atlas rows for more effiecient space use...
	_ctx->atlas_width = _ctx->bitmapXY * 8;
	_ctx->atlas_height = _ctx->bitmapXY * 12;
	int bitmapSize = _ctx->atlas_width * _ctx->atlas_height * sizeof(unsigned char);
	unsigned char *bytes = (unsigned char *)malloc(bitmapSize);
	memset(bytes, 0, bitmapSize);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, _ctx->atlas_width, _ctx->atlas_height,
	             0, GL_RED, GL_UNSIGNED_BYTE, bytes);
	free(bytes);
	printf("Atlas W: %d, Atlas H: %d\n", _ctx->atlas_width, _ctx->atlas_height);

	int x = 0;
	int y = 0;
	int glyphBitmapSize = _ctx->bitmapXY * _ctx->bitmapXY * sizeof(unsigned char);
	bytes = (unsigned char *)malloc(glyphBitmapSize);
	unsigned char *bytes2 = (unsigned char *)malloc(glyphBitmapSize);
	for(i = 32; i < 128; i++) {
		if(FT_Load_Char(face, i, FT_LOAD_RENDER))
			continue;

		memset(bytes, 0, glyphBitmapSize);
		memset(bytes2, 0, glyphBitmapSize);
		int ir, iw;
		int ir2 = (_ctx->bitmapXY - g->bitmap.rows) / 2;
		for (ir = g->bitmap.rows - 1; ir >= 0; ir--) {
			int targetRow = ir2 * _ctx->bitmapXY;
			for (iw = 0; iw < g->bitmap.width; iw++) {
				int curByte = targetRow + iw + ((_ctx->bitmapXY - g->bitmap.width) / 2);
				bytes[curByte] = g->bitmap.buffer[(ir * g->bitmap.width) + iw];
			}
			ir2++;
		}
		generateDistanceMap(bytes2, bytes, _ctx->bitmapXY, _ctx->bitmapXY);
		glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, _ctx->bitmapXY, _ctx->bitmapXY, GL_RED, GL_UNSIGNED_BYTE, bytes2);

		_ctx->c[i].ax = (float)g->advance.x / (float)maxAdvX;
//		_ctx->c[i].ax = (float)g->advance.x / (float)face->max_advance_width;
//		_ctx->c[i].ax = (float)g->metrics.horiAdvance / 64.0f / (float)(g->bitmap.width==0?_ctx->px:g->bitmap.width);//(float)(g->advance.x >> 6) / (float)(g->bitmap.width==0?px:g->bitmap.width);
//		_ctx->c[i].ax = (float)g->metrics.width / 64.0f / (float)(g->bitmap.width==0?px:g->bitmap.width);//(float)(g->advance.x >> 6) / (float)(g->bitmap.width==0?px:g->bitmap.width);
//		_ctx->c[i].ay = (float)(g->advance.y >> 6) / _ctx->px;
		_ctx->c[i].ay = (float)g->advance.y / (float)maxAdvY;
		_ctx->c[i].bw = (float)(g->bitmap.width);
		_ctx->c[i].bh = (float)(g->bitmap.rows);
		_ctx->c[i].bl = (float)(g->bitmap_left);
		_ctx->c[i].bt = (float)(g->bitmap_top);
		_ctx->c[i].tx = (float)x / _ctx->atlas_width;
		_ctx->c[i].ty = (float)y / _ctx->atlas_height;;
		x += _ctx->bitmapXY;
		if (x >= _ctx->bitmapXY * 8) {
			x = 0;
			y += _ctx->bitmapXY;
		}
	}
	free(bytes);
	free(bytes2);
	FT_Done_Face(face);
	FT_Done_FreeType(library);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);//GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);


	if (!initted) {
		char strVert[512];
		char strFrag[512];
		prog = LoadShadersFromFile(strAdd(strVert, 512, assetDir, "shaders/TextVertex.glsl"),
		                           strAdd(strFrag, 512, assetDir, "shaders/TextFragment.glsl"));
		static const GLfloat square_vertex_buffer_data[] = {
		                                                   /*-1.0f*/0.0f,  1.0f,  -0.1f,    // left top
		                                                   1.0f,  1.0f,  -0.1f,    // right top
		                                                   1.0f, /*-1.0f*/0.0f,  -0.1f,    // right bottom
		                                                   /*-1.0f*/0.0f, /*-1.0f*/0.0f,  -0.1f,    // left bottom
		};
		glGenBuffers(1, &square);
		glBindBuffer(GL_ARRAY_BUFFER, square);
		glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertex_buffer_data), square_vertex_buffer_data, GL_STATIC_DRAW);

		glGenBuffers(1, &squareuv);
		glBindBuffer(GL_ARRAY_BUFFER, squareuv);
		glBufferData(GL_ARRAY_BUFFER, sizeof(square_uv_buffer_data), square_uv_buffer_data, GL_DYNAMIC_DRAW);

		glGenBuffers(1, &indices);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareindices), squareindices, GL_STATIC_DRAW);
		initted = TRUE;
	}

//	printf("XXX w: %d, h: %d maxW: %d\n", _ctx->atlas_width, _ctx->atlas_height, maxW);
	return _ctx;
}

static void displayAtlas(_FontContext *_ctx) {
	float sx = 1, sy = 1;
//	glViewport(10, 10, windowWidth/*_ctx->atlas_width*/, _ctx->atlas_height * ((float)windowWidth/(float)_ctx->atlas_width));
//	glViewport(0, 0, windowWidth * ((float)windowHeight/(float)_ctx->atlas_height), windowHeight);
	glViewport(0, 0, windowWidth, windowHeight);
	glUseProgram(prog);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _ctx->tex);
	GLuint t0 = glGetUniformLocation(prog, "Texture0");
	glUniform1i(t0, 0);
	GLuint scalex = glGetUniformLocation(prog, "scalex");
	glUniform1f(scalex, sx);
	GLuint scaley = glGetUniformLocation(prog, "scaley");
	glUniform1f(scaley, sy);

	GLuint xpos = glGetUniformLocation(prog, "xpos");
	glUniform1f(xpos, -1);
	GLuint ypos = glGetUniformLocation(prog, "ypos");
	glUniform1f(ypos, -1);
	GLuint xoffset = glGetUniformLocation(prog, "xoffset");
	GLuint yoffset = glGetUniformLocation(prog, "yoffset");
	GLuint textColor = glGetUniformLocation(prog, "textColor");
	float black[] = {0, 0, 0, 1};
	glUniform4fv(textColor, 1, black);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, square);
	glVertexAttribPointer(0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	                      3,                  // size
	                      GL_FLOAT,           // type
	                      GL_FALSE,           // normalized?
	                      0,                  // stride
	                      (void*)0            // array buffer offset
	                      );
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, squareuv);
	glVertexAttribPointer(1,                  // attribute 1. No particular reason for 1, but must match the layout in the shader.
	                      2,                  // size
	                      GL_FLOAT,           // type
	                      GL_FALSE,           // normalized?
	                      0,                  // stride
	                      (void*)0            // array buffer offset
	                      );

	float xoff = 0;
	float yoff = 0;
	glUniform1f(xoffset, xoff);
	glUniform1f(yoffset, yoff);
	square_uv_buffer_data[0] = square_uv_buffer_data[6] = 0;
	square_uv_buffer_data[2] = square_uv_buffer_data[4] = 1;
	square_uv_buffer_data[5] = square_uv_buffer_data[7] = 0;
	square_uv_buffer_data[1] = square_uv_buffer_data[3] = 1;

	glBindBuffer(GL_ARRAY_BUFFER, squareuv);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square_uv_buffer_data), square_uv_buffer_data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glViewport(0, 0, windowWidth, windowHeight);
}

void renderTextSize(FontContext ctx, const char *text, float size, float *width, float *height)
{
	_FontContext *_ctx = (_FontContext *)ctx;
	float sx = size, sy = size;
	if (windowWidth < windowHeight) {
		sy *= (float)windowWidth/(float)windowHeight;
	} else {
		sx *= (float)windowHeight/(float)windowWidth;
	}

	const char *p;
	*width = 0;
	*height = sy;
	for(p = text; *p; p++) {
		*width += (_ctx->c[*p].ax * sx);
	}
}

void renderText(FontContext ctx, const char *text, float x, float y, float size, const float *color)
{
	_FontContext *_ctx = (_FontContext *)ctx;
	GLboolean depthTest;
	glGetBooleanv(GL_DEPTH_TEST, &depthTest);
	if (depthTest) glDisable(GL_DEPTH_TEST);
//	displayAtlas(_ctx);
	float sx = size, sy = size;
	if (windowWidth < windowHeight) {
		sy *= (float)windowWidth/(float)windowHeight;
	} else {
		sx *= (float)windowHeight/(float)windowWidth;
	}
	glUseProgram(prog);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _ctx->tex);
	GLuint t0 = glGetUniformLocation(prog, "Texture0");
	glUniform1i(t0, 0);
	GLuint scalex = glGetUniformLocation(prog, "scalex");
	GLuint scaley = glGetUniformLocation(prog, "scaley");
	glUniform1f(scalex, sx);
	glUniform1f(scaley, sy);


	GLuint xpos = glGetUniformLocation(prog, "xpos");
	glUniform1f(xpos, x);
	GLuint ypos = glGetUniformLocation(prog, "ypos");
	glUniform1f(ypos, y);
	GLuint xoffset = glGetUniformLocation(prog, "xoffset");
	GLuint yoffset = glGetUniformLocation(prog, "yoffset");
	GLuint zoffset = glGetUniformLocation(prog, "zoffset");
	GLuint textColor = glGetUniformLocation(prog, "textColor");
	glUniform4fv(textColor, 1, color);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, square);
	glVertexAttribPointer(0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	                      3,                  // size
	                      GL_FLOAT,           // type
	                      GL_FALSE,           // normalized?
	                      0,                  // stride
	                      (void*)0            // array buffer offset
	                      );
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, squareuv);
	glVertexAttribPointer(1,                  // attribute 1. No particular reason for 1, but must match the layout in the shader.
	                      2,                  // size
	                      GL_FLOAT,           // type
	                      GL_FALSE,           // normalized?
	                      0,                  // stride
	                      (void*)0            // array buffer offset
	                      );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);

	const char *p;
	float xoff = 0;
	float yoff = 0;
	float zoff = 0;
	for(p = text; *p; p++) {
		if (_ctx->c[*p].bw == 0 || _ctx->c[*p].bh == 0) {
			xoff += (_ctx->c[*p].ax * sx);
			continue;
		}

		float xslide = ((_ctx->c[*p].bl / (float)_ctx->bitmapXY) -
		               ((float)((_ctx->bitmapXY - _ctx->c[*p].bw) / 2) / (float)_ctx->bitmapXY))
		               * sx;
		xoff += xslide;
		glUniform1f(xoffset, xoff);
		xoff -= xslide;
		yoff = (((_ctx->c[*p].bt - _ctx->c[*p].bh) / (float)_ctx->bitmapXY) -
		        ((float)((_ctx->bitmapXY - _ctx->c[*p].bh) / 2) / (float)_ctx->bitmapXY))
		        * sy;
		glUniform1f(yoffset, yoff);
		glUniform1f(zoffset, zoff);
		xoff += (_ctx->c[*p].ax * sx);

		square_uv_buffer_data[0] = square_uv_buffer_data[6] = _ctx->c[*p].tx;
		square_uv_buffer_data[2] = square_uv_buffer_data[4] =
		        _ctx->c[*p].tx + (_ctx->bitmapXY / (float)_ctx->atlas_width);

		square_uv_buffer_data[5] = square_uv_buffer_data[7] = _ctx->c[*p].ty;
		square_uv_buffer_data[1] = square_uv_buffer_data[3] =
		        _ctx->c[*p].ty + (_ctx->bitmapXY / (float)_ctx->atlas_height);

		glBindBuffer(GL_ARRAY_BUFFER, squareuv);
		glBufferData(GL_ARRAY_BUFFER, sizeof(square_uv_buffer_data), square_uv_buffer_data, GL_DYNAMIC_DRAW);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
	}
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	if (depthTest) glEnable(GL_DEPTH_TEST);
}
