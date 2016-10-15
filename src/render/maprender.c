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
#include "maprender.h"
#include "shaders.h"
#include "pc/GL/glew.h"
#include "imageutils.h"
#include "../util/string.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define HEX_HEIGHT_ADJUST .86330935

typedef struct {
	Map *map;

	int width;
	int height;
	int mapStartX;
	int mapStartY;
	int mapwidth;
	int mapheight;
	int miniStartX;
	int miniStartY;
	int miniwidth;
	int miniheight;
	int zoomLevel;
	int centerRow;
	int centerCol;

	float mapScale;
	float maphfactor;
	float mapwfactor;
	float minihfactor;
	float miniwfactor;

	Bool   miniMapDirty;
	byte  *miniMap;
	GLuint miniMapTexture;

	GLuint vertexArrayID;

	// Shaders
	GLuint hexprog;
	GLuint hexminiprog;
	GLuint minilocprog;
	GLuint hexoverlayprog;
	GLuint texpassprog;

	// Uniforms
	GLuint heightFactor;
	GLuint widthFactor;
	GLuint centerId;
	GLuint useBorderId;
	GLuint textureID0;
	GLuint borderColor;
	GLuint borderThickness;
	GLuint mini_heightFactor;
	GLuint mini_widthFactor;
	GLuint mini_centerId;
	GLuint mini_colorId;
	GLuint miniloc_heightFactor;
	GLuint miniloc_widthFactor;
	GLuint miniloc_centerhFactor;
	GLuint miniloc_centerwFactor;
	GLuint miniloc_centerId;
	GLuint miniloc_colorId;
	GLuint hexOverlayHeightFactor;
	GLuint hexOverlayWidthFactor;
	GLuint hexOverlayCenterId;
	GLuint texpass_textureID0;

	// Models for various hexes (or parts thereof).
	GLuint squarevertexbuffer;
	GLuint squareuvbuffer;
	GLuint hexvertexbuffer;
	GLuint hexbarrybuffer;
	GLuint hexuvbuffer;
	GLuint hexleftvertexbuffer;
	GLuint hexleftuvbuffer;
	GLuint hexrightvertexbuffer;
	GLuint hexrightuvbuffer;
	GLuint hextopvertexbuffer;
	GLuint hextopuvbuffer;
	GLuint hexbottomvertexbuffer;
	GLuint hexbottomuvbuffer;
	GLuint hextoprightvertexbuffer;
	GLuint hextoprightuvbuffer;
	GLuint hexbottomrightvertexbuffer;
	GLuint hexbottomrightuvbuffer;
	GLuint hexbottomleftvertexbuffer;
	GLuint hexbottomleftuvbuffer;

	// Model indices
	GLuint hexidxbuffer;
	GLuint squareidxbuffer;

	// Textures
	GLuint borderTex;
	GLuint waterTex;
	GLuint grassTex;
	GLuint hillTex;
	GLuint forestTex;
	GLuint mountainTex;
	GLuint swampTex;
	GLuint cityCenterTex;
	GLuint city1Tex;
	GLuint city2Tex;
	GLuint city3Tex;
	GLuint city4Tex;
	GLuint city5Tex;
	GLuint city6Tex;
	GLuint unitDefault;
} _MapRenderer;

static const float zoomLevels[] = { .2f, .1f, .075f, .05f, .03f };

static GLuint genBuffer(const GLfloat *buffer, int size) {
	GLuint bufferID;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &bufferID);

	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, bufferID);

	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
	return bufferID;
}

MapRenderer initMapRender(Map *map, int width, int height, const char *assetDir) {
	_MapRenderer *mr = (_MapRenderer *)malloc(sizeof(_MapRenderer));
	char strBuf[512];
	char strFrag[512];
	if (mr == NULL) return NULL;
	memset(mr, 0, sizeof(_MapRenderer));
	mr->zoomLevel = 1;
	mr->mapScale = zoomLevels[mr->zoomLevel];
	mr->map = map;
	mr->centerRow = map->row;
	mr->centerCol = map->col;
	mr->miniMapDirty = TRUE;
	mr->hexprog = LoadShadersFromFile(strAdd(strBuf, 512, assetDir, "shaders/HexVertex.glsl"),
	                                   strAdd(strFrag, 512, assetDir, "shaders/HexFragment.glsl"));
	mr->heightFactor = glGetUniformLocation(mr->hexprog, "heightFactor");
	mr->widthFactor = glGetUniformLocation(mr->hexprog, "widthFactor");
	mr->centerId = glGetUniformLocation(mr->hexprog, "center");
	mr->useBorderId = glGetUniformLocation(mr->hexprog, "useBorder");
	mr->textureID0 = glGetUniformLocation(mr->hexprog, "Texture0");
	mr->borderColor = glGetUniformLocation(mr->hexprog, "borderColor");
	mr->borderThickness = glGetUniformLocation(mr->hexprog, "borderThickness");

	mr->hexminiprog = LoadShadersFromFile(strAdd(strBuf, 512, assetDir, "shaders/HexMiniVertex.glsl"),
	                                       strAdd(strFrag, 512, assetDir, "shaders/HexMiniFragment.glsl"));
	mr->mini_heightFactor = glGetUniformLocation(mr->hexminiprog, "heightFactor");
	mr->mini_widthFactor = glGetUniformLocation(mr->hexminiprog, "widthFactor");
	mr->mini_centerId = glGetUniformLocation(mr->hexminiprog, "center");
	mr->mini_colorId = glGetUniformLocation(mr->hexminiprog, "tileColor");

	mr->minilocprog = LoadShadersFromFile(strAdd(strBuf, 512, assetDir, "shaders/MiniLocVertex.glsl"),
	                                       strAdd(strFrag, 512, assetDir, "shaders/MiniLocFragment.glsl"));
	mr->miniloc_heightFactor = glGetUniformLocation(mr->minilocprog, "heightFactor");
	mr->miniloc_widthFactor = glGetUniformLocation(mr->minilocprog, "widthFactor");
	mr->miniloc_centerhFactor = glGetUniformLocation(mr->minilocprog, "centerhFactor");
	mr->miniloc_centerwFactor = glGetUniformLocation(mr->minilocprog, "centerwFactor");
	mr->miniloc_centerId = glGetUniformLocation(mr->minilocprog, "center");
	mr->miniloc_colorId = glGetUniformLocation(mr->minilocprog, "tileColor");

	mr->hexoverlayprog = LoadShadersFromFile(strAdd(strBuf, 512, assetDir, "shaders/HexOverlayVertex.glsl"),
	                                         strAdd(strFrag, 512, assetDir, "shaders/HexOverlayFragment.glsl"));
	mr->hexOverlayHeightFactor = glGetUniformLocation(mr->hexoverlayprog, "heightFactor");
	mr->hexOverlayWidthFactor = glGetUniformLocation(mr->hexoverlayprog, "widthFactor");
	mr->hexOverlayCenterId = glGetUniformLocation(mr->hexoverlayprog, "center");
	mr->texpassprog = LoadShadersFromFile(strAdd(strBuf, 512, assetDir, "shaders/TexPassVertex.glsl"),
	                                      strAdd(strFrag, 512, assetDir, "shaders/TexPassFragment.glsl"));
	mr->texpass_textureID0 = glGetUniformLocation(mr->texpassprog, "Texture0");

	glGenVertexArrays(1, &mr->vertexArrayID);
	glBindVertexArray(mr->vertexArrayID);

	static const GLfloat square_vertex_buffer_data[] = {
	   -1.0f,  1.0f,  0.1f,    // left top
	    1.0f,  1.0f,  0.1f,    // right top
	    1.0f, -1.0f,  0.1f,    // right bottom
	   -1.0f, -1.0f,  0.1f,    // left bottom
	};
	mr->squarevertexbuffer = genBuffer(square_vertex_buffer_data, sizeof(square_vertex_buffer_data));
	static const GLfloat square_uv_buffer_data[] = {
	     0.0f,  1.0f,    // left top
	     1.0f,  1.0f,    // right top
	     1.0f,  0.0f,    // right bottom
	     0.0f,  0.0f,    // left bottom
	};
	mr->squareuvbuffer = genBuffer(square_uv_buffer_data, sizeof(square_uv_buffer_data));


	static const GLfloat hex_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	   -0.5f,  1.0f,  0.0f,    // left top
	    0.5f,  1.0f,  0.0f,    // right top
	    1.0f,  0.0f,  0.0f,    // right
	    0.5f, -1.0f,  0.0f,    // right bottom
	   -0.5f, -1.0f,  0.0f,    // left bottom
	   -1.0f,  0.0f,  0.0f,    // left
	};
	mr->hexvertexbuffer = genBuffer(hex_vertex_buffer_data, sizeof(hex_vertex_buffer_data));
	static const GLfloat hex_barry_buffer_data[] = {
	    1.0f,  0.0f,  0.0f,    // center
	    0.0f,  1.0f,  0.0f,    // left top
	    0.0f,  1.0f,  0.0f,    // right top
	    0.0f,  1.0f,  0.0f,    // right
	    0.0f,  1.0f,  0.0f,    // right bottom
	    0.0f,  1.0f,  0.0f,    // left bottom
	    0.0f,  1.0f,  0.0f,    // left
	};
	mr->hexbarrybuffer = genBuffer(hex_barry_buffer_data, sizeof(hex_barry_buffer_data));
	static const GLfloat hex_left_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	   -0.5f,  1.0f,  0.0f,    // left top
	    0.5f,  1.0f,  0.0f,    // right top
	    1.0f,  0.0f,  0.0f,    // right
	    0.5f, -1.0f,  0.0f,    // right bottom
	   -0.5f, -1.0f,  0.0f,    // left bottom
	   -0.5f,  0.0f,  0.0f,    // left
	};
	mr->hexleftvertexbuffer = genBuffer(hex_left_vertex_buffer_data, sizeof(hex_left_vertex_buffer_data));
	static const GLfloat hex_right_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	   -0.5f,  1.0f,  0.0f,    // left top
	    0.5f,  1.0f,  0.0f,    // right top
	    0.5f,  0.0f,  0.0f,    // right
	    0.5f, -1.0f,  0.0f,    // right bottom
	   -0.5f, -1.0f,  0.0f,    // left bottom
	   -1.0f,  0.0f,  0.0f,    // left
	};
	mr->hexrightvertexbuffer = genBuffer(hex_right_vertex_buffer_data, sizeof(hex_right_vertex_buffer_data));
	static const GLfloat hex_top_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	    0.0f,  0.0f,  0.0f,    // left top
	    0.0f,  0.0f,  0.0f,    // right top
	    1.0f,  0.0f,  0.0f,    // right
	    0.5f, -1.0f,  0.0f,    // right bottom
	   -0.5f, -1.0f,  0.0f,    // left bottom
	   -1.0f,  0.0f,  0.0f,    // left
	};
	mr->hextopvertexbuffer = genBuffer(hex_top_vertex_buffer_data, sizeof(hex_top_vertex_buffer_data));
	static const GLfloat hex_bottom_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	   -0.5f,  1.0f,  0.0f,    // left top
	    0.5f,  1.0f,  0.0f,    // right top
	    1.0f,  0.0f,  0.0f,    // right
	    0.0f,  0.0f,  0.0f,    // right bottom
	   -0.0f,  0.0f,  0.0f,    // left bottom
	   -1.0f,  0.0f,  0.0f,    // left
	};
	mr->hexbottomvertexbuffer = genBuffer(hex_bottom_vertex_buffer_data, sizeof(hex_bottom_vertex_buffer_data));
	static const GLfloat hex_top_right_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	    0.0f,  0.0f,  0.0f,    // left top
	    0.0f,  0.0f,  0.0f,    // right top
	    0.5f,  0.0f,  0.0f,    // right
	    0.5f, -1.0f,  0.0f,    // right bottom
	   -0.5f, -1.0f,  0.0f,    // left bottom
	   -1.0f,  0.0f,  0.0f,    // left
	};
	mr->hextoprightvertexbuffer = genBuffer(hex_top_right_vertex_buffer_data, sizeof(hex_top_right_vertex_buffer_data));
	static const GLfloat hex_bottom_left_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	   -0.5f,  1.0f,  0.0f,    // left top
	    0.5f,  1.0f,  0.0f,    // right top
	    1.0f,  0.0f,  0.0f,    // right
	    0.0f,  0.0f,  0.0f,    // right bottom
	   -0.0f,  0.0f,  0.0f,    // left bottom
	   -0.5f,  0.0f,  0.0f,    // left
	};
	mr->hexbottomleftvertexbuffer = genBuffer(hex_bottom_left_vertex_buffer_data, sizeof(hex_bottom_left_vertex_buffer_data));
	static const GLfloat hex_bottom_right_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	   -0.5f,  1.0f,  0.0f,    // left top
	    0.5f,  1.0f,  0.0f,    // right top
	    0.5f,  0.0f,  0.0f,    // right
	    0.0f,  0.0f,  0.0f,    // right bottom
	   -0.0f,  0.0f,  0.0f,    // left bottom
	   -1.0f,  0.0f,  0.0f,    // left
	};
	mr->hexbottomrightvertexbuffer = genBuffer(hex_bottom_right_vertex_buffer_data, sizeof(hex_bottom_right_vertex_buffer_data));

	static const GLfloat hex_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.25f, 1.0f,    // left top
	    0.75f, 1.0f,    // right top
	    1.0f,  0.5f,    // right
	    0.75f, 0.0f,    // right bottom
	    0.25f, 0.0f,    // left bottom
	    0.0f,  0.5f,    // left
	};
	mr->hexuvbuffer = genBuffer(hex_uv_buffer_data, sizeof(hex_uv_buffer_data));
	static const GLfloat hex_left_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.25f, 1.0f,    // left top
	    0.75f, 1.0f,    // right top
	    1.0f,  0.5f,    // right
	    0.75f, 0.0f,    // right bottom
	    0.25f, 0.0f,    // left bottom
	    0.25f,  0.5f,   // left
	};
	mr->hexleftuvbuffer = genBuffer(hex_left_uv_buffer_data, sizeof(hex_left_uv_buffer_data));
	static const GLfloat hex_right_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.25f, 1.0f,    // left top
	    0.75f, 1.0f,    // right top
	    0.75f, 0.5f,    // right
	    0.75f, 0.0f,    // right bottom
	    0.25f, 0.0f,    // left bottom
	    0.0f,  0.5f,    // left
	};
	mr->hexrightuvbuffer = genBuffer(hex_right_uv_buffer_data, sizeof(hex_right_uv_buffer_data));
	static const GLfloat hex_top_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.5f,  0.5f,    // left top
	    0.5f,  0.5f,    // right top
	    1.0f,  0.5f,    // right
	    0.75f, 0.0f,    // right bottom
	    0.25f, 0.0f,    // left bottom
	    0.0f,  0.5f,    // left
	};
	mr->hextopuvbuffer = genBuffer(hex_top_uv_buffer_data, sizeof(hex_top_uv_buffer_data));
	static const GLfloat hex_bottom_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.25f, 1.0f,    // left top
	    0.75f, 1.0f,    // right top
	    1.0f,  0.5f,    // right
	    0.75f, 0.5f,    // right bottom
	    0.25f, 0.5f,    // left bottom
	    0.0f,  0.5f,    // left
	};
	mr->hexbottomuvbuffer = genBuffer(hex_bottom_uv_buffer_data, sizeof(hex_bottom_uv_buffer_data));
	static const GLfloat hex_top_right_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.5f,  0.5f,    // left top
	    0.5f,  0.5f,    // right top
	    0.75f, 0.5f,    // right
	    0.75f, 0.0f,    // right bottom
	    0.25f, 0.0f,    // left bottom
	    0.0f,  0.5f,    // left
	};
	mr->hextoprightuvbuffer = genBuffer(hex_top_right_uv_buffer_data, sizeof(hex_top_right_uv_buffer_data));
	static const GLfloat hex_bottom_left_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.25f, 1.0f,    // left top
	    0.75f, 1.0f,    // right top
	    1.0f,  0.5f,    // right
	    0.75f, 0.5f,    // right bottom
	    0.25f, 0.5f,    // left bottom
	    0.25f, 0.5f,    // left
	};
	mr->hexbottomleftuvbuffer = genBuffer(hex_bottom_left_uv_buffer_data, sizeof(hex_bottom_left_uv_buffer_data));
	static const GLfloat hex_bottom_right_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.25f, 1.0f,    // left top
	    0.75f, 1.0f,    // right top
	    0.75f, 0.5f,    // right
	    0.75f, 0.5f,    // right bottom
	    0.25f, 0.5f,    // left bottom
	    0.0f,  0.5f,    // left
	};
	mr->hexbottomrightuvbuffer = genBuffer(hex_bottom_right_uv_buffer_data, sizeof(hex_bottom_right_uv_buffer_data));

	static const unsigned short hexindices[] = { 0, 2, 1,
	                                             0, 3, 2,
	                                             0, 4, 3,
	                                             0, 5, 4,
	                                             0, 6, 5,
	                                             0, 1, 6 };  // Triangles
	glGenBuffers(1, &mr->hexidxbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mr->hexidxbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(hexindices), hexindices, GL_STATIC_DRAW);

	static const unsigned short squareindices[] = { 2,1,0,
	                                       3,2,0};
	glGenBuffers(1, &mr->squareidxbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mr->squareidxbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareindices), squareindices, GL_STATIC_DRAW);

	mr->borderTex = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/border_tile.png"));
	mr->waterTex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/water_tile.png"));
	mr->grassTex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/grass_tile.png"));
	mr->hillTex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/hill_tile.png"));
	mr->forestTex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/forest_tile.png"));
	mr->mountainTex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/mountain_tile.png"));
	mr->swampTex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/swamp_tile.png"));
	mr->cityCenterTex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/citycenter_tile.png"));
	mr->city1Tex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/city1_tile.png"));
	mr->city2Tex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/city2_tile.png"));
	mr->city3Tex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/city3_tile.png"));
	mr->city4Tex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/city4_tile.png"));
	mr->city5Tex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/city5_tile.png"));
	mr->city6Tex  = read_png_file(strAdd(strBuf, 512, assetDir, "tiles/city6_tile.png"));
	mr->unitDefault  = read_png_file(strAdd(strBuf, 512, assetDir, "units/default_unit.png"));
	return mr;
}

void updateMapDimensions(_MapRenderer *_ctx, int width, int height) {
	Map *map = _ctx->map;
	_ctx->width = width;
	_ctx->height = height;

	// Calculate minimap coordinates.
	_ctx->miniwidth = width * .28;
	_ctx->miniheight = height * .5;
	_ctx->miniStartX = width - _ctx->miniwidth;
	_ctx->miniwidth -= (width * .01);
	_ctx->miniStartY = height - _ctx->miniheight;
	_ctx->miniheight -= (height * .01);
	// Calculate map coordinates.
	_ctx->mapwidth = width * .7;
	_ctx->mapheight = height * .85;
	_ctx->mapStartX = width * .01;
	_ctx->mapStartY = height - _ctx->mapheight - (height * .01);
	if (_ctx->mapStartY < 0) _ctx->mapStartY = 0;

	_ctx->maphfactor = _ctx->mapheight;
	_ctx->mapwfactor = _ctx->mapwidth;
	_ctx->minihfactor = _ctx->miniheight;
	_ctx->miniwfactor = _ctx->miniwidth;

	// Calculate the mini map scaling factors.
	if (_ctx->minihfactor < _ctx->miniwfactor) {
		_ctx->miniwfactor = (_ctx->minihfactor / _ctx->miniwfactor);
		_ctx->minihfactor = 1;
	} else {
		_ctx->minihfactor = (_ctx->miniwfactor / _ctx->minihfactor);
		_ctx->miniwfactor = 1;
	}
	_ctx->minihfactor *= HEX_HEIGHT_ADJUST;
	float s1 = 1.0f / ((float)map->numRows*_ctx->minihfactor);
	float s2 = 1.0f / ((float)(map->numCols/2)*1.5f*_ctx->miniwfactor);
	float miniScale = s1<s2?s1:s2;
	_ctx->minihfactor *= miniScale;
	_ctx->miniwfactor *= miniScale;

	// Calculate the main maps scaling factors.
	if (_ctx->maphfactor < _ctx->mapwfactor) {
		_ctx->mapwfactor = (_ctx->maphfactor / _ctx->mapwfactor) * _ctx->mapScale;
		_ctx->maphfactor = _ctx->mapScale;
	} else {
		_ctx->maphfactor = (_ctx->mapwfactor / _ctx->maphfactor) * _ctx->mapScale;
		_ctx->mapwfactor = _ctx->mapScale;
	}
	_ctx->maphfactor *= HEX_HEIGHT_ADJUST;

	if (_ctx->miniMap) {
		free(_ctx->miniMap);
		glDeleteTextures(1, &_ctx->miniMapTexture);
	}
	int miniMapMemSize = _ctx->miniheight * _ctx->miniwidth * sizeof(byte) * 4;
	_ctx->miniMap = (byte *)malloc(miniMapMemSize);
	memset(_ctx->miniMap, 255, miniMapMemSize);
	_ctx->miniMapDirty = TRUE;
	glGenTextures(1, &_ctx->miniMapTexture);

	// Store the mini map as a texture to avoid generating it needlessly (it is expensive).
	glBindTexture(GL_TEXTURE_2D, _ctx->miniMapTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _ctx->miniwidth, _ctx->miniheight,
	             0, GL_RGBA, GL_UNSIGNED_BYTE, _ctx->miniMap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glGenerateMipmap(GL_TEXTURE_2D);
}

void resizeMap(MapRenderer mapctx, int width, int height) {
	updateMapDimensions((_MapRenderer *)mapctx, width, height);
}

static GLuint getTileTexture(_MapRenderer *mr, TileType type) {
	GLuint ret;
	switch (type) {
	case Grass: ret = mr->grassTex; break;
	case Forest: ret = mr->forestTex; break;
	case Swamp: ret = mr->swampTex; break;
//	case Desert: ret = desertTex; break;
	case Hill: ret = mr->hillTex; break;
	case Mountain: ret = mr->mountainTex; break;
	case Water: ret = mr->waterTex; break;
//	case Temple: ret = templeTex; break;
//	case Ruin: ret = ruinTex; break;
	case CityCenter: ret = mr->cityCenterTex; break;
	case City1: ret = mr->city1Tex; break;
	case City2: ret = mr->city2Tex; break;
	case City3: ret = mr->city3Tex; break;
	case City4: ret = mr->city4Tex; break;
	case City5: ret = mr->city5Tex; break;
	case City6: ret = mr->city6Tex; break;
	}
	return ret;
}

static void getTileColor(TileType type, float *color) {
	switch (type) {
	case Grass: color[0] = 0.0; color[1]  = .76; color[2] = 0.0; break;
	case Forest: color[0] = 0.22; color[1] = .416; color[2] = 0.122; break;
	case Swamp: color[0] = 0.0; color[1]  = .76; color[2] = 0.522; break;
//	case Desert: ret = desertTex; break;
	case Hill: color[0] = 0.863; color[1] = .667; color[2] = 0.208; break;
	case Mountain: color[0] = 0.475; color[1] = .467; color[2] = 0.443; break;
	case Water: color[0] = 0.09; color[1] = .886; color[2] = 0.902; break;
//	case Temple: ret = templeTex; break;
//	case Ruin: ret = ruinTex; break;
	case CityCenter: color[0] = 0.0; color[1] = 0.0; color[2] = 0.0; break;
	case City1: color[0] = 0.0; color[1] = 0.0; color[2] = 0.0; break;
	case City2: color[0] = 0.0; color[1] = 0.0; color[2] = 0.0; break;
	case City3: color[0] = 0.0; color[1] = 0.0; color[2] = 0.0; break;
	case City4: color[0] = 0.0; color[1] = 0.0; color[2] = 0.0; break;
	case City5: color[0] = 0.0; color[1] = 0.0; color[2] = 0.0; break;
	case City6: color[0] = 0.0; color[1] = 0.0; color[2] = 0.0; break;
	}
}

static void drawHex(const float *center, Bool mini,
                    _MapRenderer *mr, int row, int col) {
	Map *map = mr->map;
	Bool isSelected = col==map->col&&row==map->row;
	TileType type = map->tiles[row][col].type;

	if (mini) {
		glUseProgram(mr->hexminiprog);
		glUniform1f(mr->mini_heightFactor, mr->minihfactor);
		glUniform1f(mr->mini_widthFactor, mr->miniwfactor);

		glUniform2fv(mr->mini_centerId, 1, center);

		float color[4] = {0.0, 0.0, 0.0, 1.0};
		getTileColor(type, color);
		glUniform4fv(mr->mini_colorId, 1, color);
	} else {
		glUseProgram(mr->hexprog);
		glUniform1f(mr->heightFactor, mr->maphfactor);
		glUniform1f(mr->widthFactor, mr->mapwfactor);

		glUniform2fv(mr->centerId, 1, center);
		if (isSelected) {
			float bcolor[] = {1.0, 0, 0, 1.0};
			glUniform4fv(mr->borderColor, 1, bcolor);
			glUniform1f(mr->borderThickness, .85 - ((float)mr->zoomLevel * .01));
			glUniform1i(mr->useBorderId, 1);
		} else {
			float bcolor[] = {0, 0, 0, 1.0};
			glUniform4fv(mr->borderColor, 1, bcolor);
			glUniform1f(mr->borderThickness, .95 - ((float)mr->zoomLevel * .01));
			glUniform1i(mr->useBorderId, 1);
		}

		GLuint tileTex = getTileTexture(mr, type);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tileTex);
		glUniform1i(mr->textureID0, 0);
	}

	GLuint vertexbuffer = mr->hexvertexbuffer;
	GLuint uvbuffer = mr->hexuvbuffer;
	if (col == 0) {
		vertexbuffer = mr->hexleftvertexbuffer;
		uvbuffer = mr->hexleftuvbuffer;
	}
	if (col == (map->numCols - 1)) {
		vertexbuffer = mr->hexrightvertexbuffer;
		uvbuffer = mr->hexrightuvbuffer;
	}
	if (row == 0 && (col % 2)) {
		if (col == (map->numCols - 1)) {
			vertexbuffer = mr->hextoprightvertexbuffer;
			uvbuffer = mr->hextoprightuvbuffer;
		} else {
			vertexbuffer = mr->hextopvertexbuffer;
			uvbuffer = mr->hextopuvbuffer;
		}
	}
	if (row == (map->numRows - 1) && !(col % 2)) {
		if (col == (map->numCols - 1)) {
			vertexbuffer = mr->hexbottomrightvertexbuffer;
			uvbuffer = mr->hexbottomrightuvbuffer;
		} else {
			vertexbuffer = mr->hexbottomvertexbuffer;
			uvbuffer = mr->hexbottomuvbuffer;
		}
	}
	if (row == (map->numRows - 1) && col == 0) {
		vertexbuffer = mr->hexbottomleftvertexbuffer;
		uvbuffer = mr->hexbottomleftuvbuffer;
	}

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	                      3,                  // size
	                      GL_FLOAT,            // type
	                      GL_FALSE,           // normalized?
	                      0,                  // stride
	                      (void*)0            // array buffer offset
	                      );
	if (!mini) {
		// 2nd attribute buffer : texture coords
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1,                  // attribute 1. No particular reason for 1, but must match the layout in the shader.
		                      2,                  // size
		                      GL_FLOAT,           // type
		                      GL_FALSE,           // normalized?
		                      0,                  // stride
		                      (void*)0            // array buffer offset
		                      );

		// 3nd attribute buffer : used for borders
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, mr->hexbarrybuffer);
		glVertexAttribPointer(2,                  // attribute 1. No particular reason for 1, but must match the layout in the shader.
		                      3,                  // size
		                      GL_FLOAT,           // type
		                      GL_FALSE,           // normalized?
		                      0,                  // stride
		                      (void*)0            // array buffer offset
		                      );
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mr->hexidxbuffer);
	glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, NULL);

	if (!mini) {
		GLboolean depthTest;
		glGetBooleanv(GL_DEPTH_TEST, &depthTest);
		if (depthTest) glDisable(GL_DEPTH_TEST);
		if (map->tiles[row][col].units) {
			glBindTexture(GL_TEXTURE_2D, mr->unitDefault);
//			printf("XXX Draw unit %d, %d\n", row, col);
			glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, NULL);
		}
		if (map->selectedUnit && map->tiles[row][col].currentMoveCost &&
		    map->tiles[row][col].currentMoveCost <= map->selectedUnit->movement) {
//			printf("XXX MARK MOVE %d, %d\n", row, col);
			glUseProgram(mr->hexoverlayprog);
			glUniform1f(mr->hexOverlayHeightFactor, mr->maphfactor);
			glUniform1f(mr->hexOverlayWidthFactor, mr->mapwfactor);
			glUniform2fv(mr->hexOverlayCenterId, 1, center);
			glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, NULL);
		}
		if (depthTest) glEnable(GL_DEPTH_TEST);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}
	glDisableVertexAttribArray(0);
}

static void saveMiniMap(_MapRenderer *_ctx) {
	if (!_ctx->miniMapDirty) return;
	// Save map so we can blit it directly until dirty.
	if (_ctx->miniMap) {
		glReadPixels(_ctx->miniStartX, _ctx->miniStartY, _ctx->miniwidth, _ctx->miniheight,
		             GL_RGBA, GL_UNSIGNED_BYTE, _ctx->miniMap);
		glBindTexture(GL_TEXTURE_2D, _ctx->miniMapTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _ctx->miniwidth, _ctx->miniheight,
		                GL_RGBA, GL_UNSIGNED_BYTE, _ctx->miniMap);
		_ctx->miniMapDirty = FALSE;
	}
}

static void miniMapLocMarker(_MapRenderer *_ctx) {
	// Mini Map location marker.
	int halfX = (_ctx->map->numCols / 2);
	int halfY = (_ctx->map->numRows / 2);
	glUseProgram(_ctx->minilocprog);
	float hfac = (1.0f / _ctx->maphfactor) * _ctx->minihfactor;// / (float)(map->numRows);
	float wfac = (1.0f / _ctx->mapwfactor) * _ctx->miniwfactor;// / ((float)map->numCols*.75f);
	glUniform1f(_ctx->miniloc_heightFactor, hfac);
	glUniform1f(_ctx->miniloc_widthFactor, wfac);
	glUniform1f(_ctx->miniloc_centerhFactor, _ctx->minihfactor);
	glUniform1f(_ctx->miniloc_centerwFactor, _ctx->miniwfactor);
	float c2[] = { 0, 0 };
	int mcol = -(halfY-_ctx->centerCol);
	int mrow = halfX-_ctx->centerRow;
	c2[0] = 1.5f * mcol;
	c2[1] = (2.0f * mrow)+(mcol%2?1.0f:0.0f);
	glUniform2fv(_ctx->miniloc_centerId, 1, c2);
	float markcolor[4] = {0.0, 0.0, 0.0, .3};
	glUniform4fv(_ctx->miniloc_colorId, 1, markcolor);
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, _ctx->squarevertexbuffer);
	glVertexAttribPointer(0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	                      3,                  // size
	                      GL_FLOAT,           // type
	                      GL_FALSE,           // normalized?
	                      0,                  // stride
	                      (void*)0            // array buffer offset
	                      );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ctx->squareidxbuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
	glDisableVertexAttribArray(0);
}

void renderMap(MapRenderer mapctx, Bool mini) {
	_MapRenderer *_ctx = (_MapRenderer *)mapctx;
	Map *map = _ctx->map;
	if (mini) {
		glViewport(_ctx->miniStartX,
		           _ctx->miniStartY,
		           _ctx->miniwidth,
		           _ctx->miniheight);
		if (!_ctx->miniMapDirty) {
			GLboolean depthTest;
			glGetBooleanv(GL_DEPTH_TEST, &depthTest);
			if (depthTest) glDisable(GL_DEPTH_TEST);
			glUseProgram(_ctx->texpassprog);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, _ctx->miniMapTexture);
			glUniform1i(_ctx->texpass_textureID0, 0);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, _ctx->squarevertexbuffer);
			glVertexAttribPointer(0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			                      3,                  // size
			                      GL_FLOAT,           // type
			                      GL_FALSE,           // normalized?
			                      0,                  // stride
			                      (void*)0            // array buffer offset
			                      );
			glEnableVertexAttribArray(1);
			glBindBuffer(GL_ARRAY_BUFFER, _ctx->squareuvbuffer);
			glVertexAttribPointer(1,                  // attribute 1. No particular reason for 1, but must match the layout in the shader.
			                      2,                  // size
			                      GL_FLOAT,           // type
			                      GL_FALSE,           // normalized?
			                      0,                  // stride
			                      (void*)0            // array buffer offset
			                      );
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ctx->squareidxbuffer);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			if (depthTest) glEnable(GL_DEPTH_TEST);
			miniMapLocMarker(_ctx);
			glViewport(0,
			           0,
			           _ctx->width,
			           _ctx->height);
			return;
		}
	} else {
		glViewport(_ctx->mapStartX,
		           _ctx->mapStartY,
		           _ctx->mapwidth,
		           _ctx->mapheight);
	}
	int numX;
	int numY;
	if (mini) {
		numX = map->numCols;
		numY = map->numRows;
	} else {
		numX = (int)(1.0f / _ctx->mapwfactor) + 1;
		numX += (numX / 4) + 6;  //  These are interleaved so need some padding.
		numY = (int)(1.0f / _ctx->maphfactor) + 2;
	}

	float c[] = { 0.0f, 0.0f };
	int row, col;
	int halfX = (numX / 2);
	int halfY = (numY / 2);

	// Map backing color.
	glUseProgram(_ctx->hexminiprog);
	glUniform1f(_ctx->mini_heightFactor, 1);
	glUniform1f(_ctx->mini_widthFactor, 1);
	float c2[] = { 0, 0 };
	glUniform2fv(_ctx->mini_centerId, 1, c2);
	float backcolor[4] = {0.0, 0.0, 0.0, 1.0};
	glUniform4fv(_ctx->mini_colorId, 1, backcolor);
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, _ctx->squarevertexbuffer);
	glVertexAttribPointer(0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	                      3,                  // size
	                      GL_FLOAT,           // type
	                      GL_FALSE,           // normalized?
	                      0,                  // stride
	                      (void*)0            // array buffer offset
	                      );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ctx->squareidxbuffer);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
	glDisableVertexAttribArray(0);

	for (col = -halfX; col <= halfX; col++) {
		c[0] = 1.5f * col;
		int mcol = mini?halfY+col:_ctx->centerCol+col;
		for (row = -halfY; row <= halfY; row++) {
			c[1] = (2.0f * row)+(mcol%2?1.0f:0.0f);
			int mrow = mini?halfX-row:_ctx->centerRow-row;
			if (mrow >= 0 && mrow < map->numRows && mcol >=0 && mcol < map->numCols) {
				drawHex(c , mini, _ctx, mrow, mcol);
			}
		}
	}

	if (mini) {
		saveMiniMap(_ctx);
		miniMapLocMarker(_ctx);
	}

	glViewport(0,
	           0,
	           _ctx->width,
	           _ctx->height);
}

void miniMapDirty(MapRenderer mapctx) {
	_MapRenderer *_ctx = (_MapRenderer *)mapctx;
	_ctx->miniMapDirty = TRUE;
}

void getMapDisplayCenter(MapRenderer mapctx, int *centerRow, int *centerCol,
                         int *rowsDisplayed, int *colsDisplayed) {
	_MapRenderer *mrend = (_MapRenderer *)mapctx;
	*centerRow = mrend->centerRow;
	*centerCol = mrend->centerCol;
	*rowsDisplayed = (int)(1.0f / mrend->maphfactor);
	*colsDisplayed = (int)((1.0f / mrend->mapwfactor) * 1.25);
}

void setMapDisplayCenter(MapRenderer mapctx, int displayCenterRow,
                                int displayCenterCol) {
	_MapRenderer *mrend = (_MapRenderer *)mapctx;
	mrend->centerRow = displayCenterRow;
	mrend->centerCol = displayCenterCol;
}

void zoomInMap(MapRenderer mapctx) {
	_MapRenderer *mrend = (_MapRenderer *)mapctx;
	if (mrend->zoomLevel > 0) {
		mrend->zoomLevel--;
		mrend->mapScale = zoomLevels[mrend->zoomLevel];
		updateMapDimensions(mrend, mrend->width, mrend->height);
	}
}

void zoomOutMap(MapRenderer mapctx) {
	_MapRenderer *mrend = (_MapRenderer *)mapctx;
	int maxLevel = sizeof(zoomLevels) / sizeof(float);
	if (mrend->zoomLevel < (maxLevel - 1)) {
		mrend->zoomLevel++;
		mrend->mapScale = zoomLevels[mrend->zoomLevel];
		updateMapDimensions(mrend, mrend->width, mrend->height);
	}
}


void freeMapRenderer(MapRenderer mapctx) {
	_MapRenderer *mrend = (_MapRenderer *)mapctx;
	if (!mrend) return;
	glDeleteBuffers(1, &mrend->squarevertexbuffer);
	glDeleteBuffers(1, &mrend->squareuvbuffer);
	glDeleteBuffers(1, &mrend->hexvertexbuffer);
	glDeleteBuffers(1, &mrend->hexbarrybuffer);
	glDeleteBuffers(1, &mrend->hexuvbuffer);
	glDeleteBuffers(1, &mrend->hexleftvertexbuffer);
	glDeleteBuffers(1, &mrend->hexleftuvbuffer);
	glDeleteBuffers(1, &mrend->hexrightvertexbuffer);
	glDeleteBuffers(1, &mrend->hexrightuvbuffer);
	glDeleteBuffers(1, &mrend->hextopvertexbuffer);
	glDeleteBuffers(1, &mrend->hextopuvbuffer);
	glDeleteBuffers(1, &mrend->hexbottomvertexbuffer);
	glDeleteBuffers(1, &mrend->hexbottomuvbuffer);
	glDeleteBuffers(1, &mrend->hextoprightvertexbuffer);
	glDeleteBuffers(1, &mrend->hextoprightuvbuffer);
	glDeleteBuffers(1, &mrend->hexbottomrightvertexbuffer);
	glDeleteBuffers(1, &mrend->hexbottomrightuvbuffer);
	glDeleteBuffers(1, &mrend->hexbottomleftvertexbuffer);
	glDeleteBuffers(1, &mrend->hexbottomleftuvbuffer);

	glDeleteProgram(mrend->hexprog);
	glDeleteProgram(mrend->hexminiprog);
	glDeleteProgram(mrend->minilocprog);
	glDeleteProgram(mrend->hexoverlayprog);
	glDeleteProgram(mrend->texpassprog);

	glDeleteTextures(1, &mrend->borderTex);
	glDeleteTextures(1, &mrend->waterTex);
	glDeleteTextures(1, &mrend->grassTex);
	glDeleteTextures(1, &mrend->hillTex);
	glDeleteTextures(1, &mrend->forestTex);
	glDeleteTextures(1, &mrend->mountainTex);
	glDeleteTextures(1, &mrend->swampTex);
	glDeleteTextures(1, &mrend->cityCenterTex);
	glDeleteTextures(1, &mrend->city1Tex);
	glDeleteTextures(1, &mrend->city2Tex);
	glDeleteTextures(1, &mrend->city3Tex);
	glDeleteTextures(1, &mrend->city4Tex);
	glDeleteTextures(1, &mrend->city5Tex);
	glDeleteTextures(1, &mrend->city6Tex);

	glDeleteVertexArrays(1, &mrend->vertexArrayID);

	if (mrend->miniMap) {
		free(mrend->miniMap);
		glDeleteTextures(1, &mrend->miniMapTexture);
	}

	free(mrend);
}

void getMapPostion(MapRenderer ctx, int *x, int *y,
                   int *width, int *height) {
	_MapRenderer *_ctx = (_MapRenderer *)ctx;
	*x = _ctx->mapStartX;
	*y = _ctx->height - (_ctx->mapStartY + _ctx->mapheight);
	*width = _ctx->mapwidth;
	*height = _ctx->mapheight;
}

void getMiniMapPostion(MapRenderer ctx, int *x, int *y,
                       int *width, int *height) {
	_MapRenderer *_ctx = (_MapRenderer *)ctx;
	*x = _ctx->miniStartX;
	*y = _ctx->height - (_ctx->miniStartY + _ctx->miniheight);
	*width = _ctx->miniwidth;
	*height = _ctx->miniheight;
}

Bool setCenterMiniMap(MapRenderer ctx, float x, float y, int *hexcol, int *hexrow) {
	_MapRenderer *_ctx = (_MapRenderer *)ctx;
	Bool ret = FALSE;
	int col = (x / (1.5f * _ctx->miniwfactor)) + (_ctx->map->numCols / 2);
	int row = -(y / (2.0f * _ctx->minihfactor) - (col%2?1.0f:0.0f))
	                 + (_ctx->map->numRows / 2);
	if (col >=0 && col < _ctx->map->numCols && row >= 0 && row < _ctx->map->numRows) {
		_ctx->map->col = col;
		_ctx->map->row = row;
		setMapDisplayCenter(ctx, _ctx->map->row, _ctx->map->col);
		if (hexcol) *hexcol = col;
		if (hexrow) *hexrow = row;
		ret = TRUE;
	}
	return ret;
}

Bool setCenterMap(MapRenderer ctx, float x, float y, int *hexcol, int *hexrow) {
	_MapRenderer *_ctx = (_MapRenderer *)ctx;
	Bool ret = FALSE;
	int col = _ctx->centerCol + (int)(x / (1.5f * _ctx->mapwfactor) +
	                                  (x<0?-.5f:.5f));
	int row = _ctx->centerRow - (int)((y / (2.0f * _ctx->maphfactor) +
	                                   (y<0?-.5f:.5f)) - (col%2?.5f:0.0f));
	if (col >=0 && col < _ctx->map->numCols && row >= 0 && row < _ctx->map->numRows) {
		_ctx->map->col = col;
		_ctx->map->row = row;
		if (hexcol) *hexcol = col;
		if (hexrow) *hexrow = row;
		ret = TRUE;
	}
	return ret;
}
