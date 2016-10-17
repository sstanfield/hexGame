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
#include "minimaprender.h"
#include "shaders.h"
#include "pc/GL/glew.h"
#include "gl_util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define HEX_HEIGHT_ADJUST .86330935

namespace hexgame { namespace render {

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

struct MiniMapRenderer::CTX {
	std::shared_ptr<Map> map;

	int width;
	int height;
	int centerRow;
	int centerCol;

	float mapScale;
	float maphfactor;
	float mapwfactor;

	bool   miniMapDirty;
	byte  *miniMap;
	GLuint miniMapTexture;

	GLuint vertexArrayID;

	// Shaders
	Shader::s_ptr hexminiprog;
	Shader::s_ptr minilocprog;
	Shader::s_ptr texpassprog;

	// Uniforms
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
	GLuint texpass_textureID0;

	// Models for various hexes (or parts thereof).
	GLuint squarevertexbuffer;
	GLuint squareuvbuffer;
	GLuint hexvertexbuffer;
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

	void drawHex(const float *center, unsigned int row, unsigned int col) {
		printf("XXX hex %d, %d\n", row, col);
		TileType type = map->tiles[row][col].type;

		glUseProgram(hexminiprog->id());
		glUniform1f(mini_heightFactor, maphfactor);
		glUniform1f(mini_widthFactor, mapwfactor);

		glUniform2fv(mini_centerId, 1, center);

		float color[4] = {0.0, 0.0, 0.0, 1.0};
		getTileColor(type, color);
		glUniform4fv(mini_colorId, 1, color);

		GLuint vertexbuffer = hexvertexbuffer;
		GLuint uvbuffer = hexuvbuffer;
		if (col == 0) {
			vertexbuffer = hexleftvertexbuffer;
			uvbuffer = hexleftuvbuffer;
		}
		if (col == (map->numCols - 1)) {
			vertexbuffer = hexrightvertexbuffer;
			uvbuffer = hexrightuvbuffer;
		}
		if (row == 0 && (col % 2)) {
			if (col == (map->numCols - 1)) {
				vertexbuffer = hextoprightvertexbuffer;
				uvbuffer = hextoprightuvbuffer;
			} else {
				vertexbuffer = hextopvertexbuffer;
				uvbuffer = hextopuvbuffer;
			}
		}
		if (row == (map->numRows - 1) && !(col % 2)) {
			if (col == (map->numCols - 1)) {
				vertexbuffer = hexbottomrightvertexbuffer;
				uvbuffer = hexbottomrightuvbuffer;
			} else {
				vertexbuffer = hexbottomvertexbuffer;
				uvbuffer = hexbottomuvbuffer;
			}
		}
		if (row == (map->numRows - 1) && col == 0) {
			vertexbuffer = hexbottomleftvertexbuffer;
			uvbuffer = hexbottomleftuvbuffer;
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

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hexidxbuffer);
		glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, NULL);

		glDisableVertexAttribArray(0);
	}

	void saveMiniMap() {
		if (!miniMapDirty) return;
		// Save map so we can blit it directly until dirty.
		if (miniMap) {
			glReadPixels(0, 0, width, height,
					GL_RGBA, GL_UNSIGNED_BYTE, miniMap);
			glBindTexture(GL_TEXTURE_2D, miniMapTexture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
					GL_RGBA, GL_UNSIGNED_BYTE, miniMap);
			miniMapDirty = false;
		}
	}

	void miniMapLocMarker() {
		// Mini Map location marker.
		int halfX = (map->numCols / 2);
		int halfY = (map->numRows / 2);
		glUseProgram(minilocprog->id());
		float hfac = (1.0f / maphfactor) * maphfactor;// / (float)(map->numRows);
		float wfac = (1.0f / mapwfactor) * mapwfactor;// / ((float)map->numCols*.75f);
		glUniform1f(miniloc_heightFactor, hfac);
		glUniform1f(miniloc_widthFactor, wfac);
		glUniform1f(miniloc_centerhFactor, maphfactor);
		glUniform1f(miniloc_centerwFactor, mapwfactor);
		float c2[] = { 0, 0 };
		int mcol = -(halfY-centerCol);
		int mrow = halfX-centerRow;
		c2[0] = 1.5f * mcol;
		c2[1] = (2.0f * mrow)+(mcol%2?1.0f:0.0f);
		glUniform2fv(miniloc_centerId, 1, c2);
		float markcolor[4] = {0.0, 0.0, 0.0, .3};
		glUniform4fv(miniloc_colorId, 1, markcolor);
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, squarevertexbuffer);
		glVertexAttribPointer(0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
	            );
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareidxbuffer);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
		glDisableVertexAttribArray(0);
	}

};

MiniMapRenderer::MiniMapRenderer(std::shared_ptr<Map> map, int width, int height, std::string assetDir) {
	_ctx = std::make_unique<CTX>();
	_ctx->map = map;
	_ctx->centerRow = map->row;
	_ctx->centerCol = map->col;
	_ctx->miniMapDirty = true;

	_ctx->hexminiprog = LoadShadersFromFile(assetDir + "shaders/HexMiniVertex.glsl",
	                                      assetDir + "shaders/HexMiniFragment.glsl");
	_ctx->mini_heightFactor = glGetUniformLocation(_ctx->hexminiprog->id(), "heightFactor");
	_ctx->mini_widthFactor = glGetUniformLocation(_ctx->hexminiprog->id(), "widthFactor");
	_ctx->mini_centerId = glGetUniformLocation(_ctx->hexminiprog->id(), "center");
	_ctx->mini_colorId = glGetUniformLocation(_ctx->hexminiprog->id(), "tileColor");

	_ctx->minilocprog = LoadShadersFromFile(assetDir + "shaders/MiniLocVertex.glsl",
	                                      assetDir + "shaders/MiniLocFragment.glsl");
	_ctx->miniloc_heightFactor = glGetUniformLocation(_ctx->minilocprog->id(), "heightFactor");
	_ctx->miniloc_widthFactor = glGetUniformLocation(_ctx->minilocprog->id(), "widthFactor");
	_ctx->miniloc_centerhFactor = glGetUniformLocation(_ctx->minilocprog->id(), "centerhFactor");
	_ctx->miniloc_centerwFactor = glGetUniformLocation(_ctx->minilocprog->id(), "centerwFactor");
	_ctx->miniloc_centerId = glGetUniformLocation(_ctx->minilocprog->id(), "center");
	_ctx->miniloc_colorId = glGetUniformLocation(_ctx->minilocprog->id(), "tileColor");

	_ctx->texpassprog = LoadShadersFromFile(assetDir + "shaders/TexPassVertex.glsl",
	                                      assetDir + "shaders/TexPassFragment.glsl");
	_ctx->texpass_textureID0 = glGetUniformLocation(_ctx->texpassprog->id(), "Texture0");

	glGenVertexArrays(1, &_ctx->vertexArrayID);
	glBindVertexArray(_ctx->vertexArrayID);

	static const GLfloat square_vertex_buffer_data[] = {
	   -1.0f,  1.0f,  0.1f,    // left top
	    1.0f,  1.0f,  0.1f,    // right top
	    1.0f, -1.0f,  0.1f,    // right bottom
	   -1.0f, -1.0f,  0.1f,    // left bottom
	};
	_ctx->squarevertexbuffer = genBuffer(square_vertex_buffer_data, sizeof(square_vertex_buffer_data));
	static const GLfloat square_uv_buffer_data[] = {
	     0.0f,  1.0f,    // left top
	     1.0f,  1.0f,    // right top
	     1.0f,  0.0f,    // right bottom
	     0.0f,  0.0f,    // left bottom
	};
	_ctx->squareuvbuffer = genBuffer(square_uv_buffer_data, sizeof(square_uv_buffer_data));


	static const GLfloat hex_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	   -0.5f,  1.0f,  0.0f,    // left top
	    0.5f,  1.0f,  0.0f,    // right top
	    1.0f,  0.0f,  0.0f,    // right
	    0.5f, -1.0f,  0.0f,    // right bottom
	   -0.5f, -1.0f,  0.0f,    // left bottom
	   -1.0f,  0.0f,  0.0f,    // left
	};
	_ctx->hexvertexbuffer = genBuffer(hex_vertex_buffer_data, sizeof(hex_vertex_buffer_data));
	static const GLfloat hex_left_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	   -0.5f,  1.0f,  0.0f,    // left top
	    0.5f,  1.0f,  0.0f,    // right top
	    1.0f,  0.0f,  0.0f,    // right
	    0.5f, -1.0f,  0.0f,    // right bottom
	   -0.5f, -1.0f,  0.0f,    // left bottom
	   -0.5f,  0.0f,  0.0f,    // left
	};
	_ctx->hexleftvertexbuffer = genBuffer(hex_left_vertex_buffer_data, sizeof(hex_left_vertex_buffer_data));
	static const GLfloat hex_right_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	   -0.5f,  1.0f,  0.0f,    // left top
	    0.5f,  1.0f,  0.0f,    // right top
	    0.5f,  0.0f,  0.0f,    // right
	    0.5f, -1.0f,  0.0f,    // right bottom
	   -0.5f, -1.0f,  0.0f,    // left bottom
	   -1.0f,  0.0f,  0.0f,    // left
	};
	_ctx->hexrightvertexbuffer = genBuffer(hex_right_vertex_buffer_data, sizeof(hex_right_vertex_buffer_data));
	static const GLfloat hex_top_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	    0.0f,  0.0f,  0.0f,    // left top
	    0.0f,  0.0f,  0.0f,    // right top
	    1.0f,  0.0f,  0.0f,    // right
	    0.5f, -1.0f,  0.0f,    // right bottom
	   -0.5f, -1.0f,  0.0f,    // left bottom
	   -1.0f,  0.0f,  0.0f,    // left
	};
	_ctx->hextopvertexbuffer = genBuffer(hex_top_vertex_buffer_data, sizeof(hex_top_vertex_buffer_data));
	static const GLfloat hex_bottom_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	   -0.5f,  1.0f,  0.0f,    // left top
	    0.5f,  1.0f,  0.0f,    // right top
	    1.0f,  0.0f,  0.0f,    // right
	    0.0f,  0.0f,  0.0f,    // right bottom
	   -0.0f,  0.0f,  0.0f,    // left bottom
	   -1.0f,  0.0f,  0.0f,    // left
	};
	_ctx->hexbottomvertexbuffer = genBuffer(hex_bottom_vertex_buffer_data, sizeof(hex_bottom_vertex_buffer_data));
	static const GLfloat hex_top_right_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	    0.0f,  0.0f,  0.0f,    // left top
	    0.0f,  0.0f,  0.0f,    // right top
	    0.5f,  0.0f,  0.0f,    // right
	    0.5f, -1.0f,  0.0f,    // right bottom
	   -0.5f, -1.0f,  0.0f,    // left bottom
	   -1.0f,  0.0f,  0.0f,    // left
	};
	_ctx->hextoprightvertexbuffer = genBuffer(hex_top_right_vertex_buffer_data, sizeof(hex_top_right_vertex_buffer_data));
	static const GLfloat hex_bottom_left_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	   -0.5f,  1.0f,  0.0f,    // left top
	    0.5f,  1.0f,  0.0f,    // right top
	    1.0f,  0.0f,  0.0f,    // right
	    0.0f,  0.0f,  0.0f,    // right bottom
	   -0.0f,  0.0f,  0.0f,    // left bottom
	   -0.5f,  0.0f,  0.0f,    // left
	};
	_ctx->hexbottomleftvertexbuffer = genBuffer(hex_bottom_left_vertex_buffer_data, sizeof(hex_bottom_left_vertex_buffer_data));
	static const GLfloat hex_bottom_right_vertex_buffer_data[] = {
	    0.0f,  0.0f,  0.0f,    // center
	   -0.5f,  1.0f,  0.0f,    // left top
	    0.5f,  1.0f,  0.0f,    // right top
	    0.5f,  0.0f,  0.0f,    // right
	    0.0f,  0.0f,  0.0f,    // right bottom
	   -0.0f,  0.0f,  0.0f,    // left bottom
	   -1.0f,  0.0f,  0.0f,    // left
	};
	_ctx->hexbottomrightvertexbuffer = genBuffer(hex_bottom_right_vertex_buffer_data, sizeof(hex_bottom_right_vertex_buffer_data));

	static const GLfloat hex_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.25f, 1.0f,    // left top
	    0.75f, 1.0f,    // right top
	    1.0f,  0.5f,    // right
	    0.75f, 0.0f,    // right bottom
	    0.25f, 0.0f,    // left bottom
	    0.0f,  0.5f,    // left
	};
	_ctx->hexuvbuffer = genBuffer(hex_uv_buffer_data, sizeof(hex_uv_buffer_data));
	static const GLfloat hex_left_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.25f, 1.0f,    // left top
	    0.75f, 1.0f,    // right top
	    1.0f,  0.5f,    // right
	    0.75f, 0.0f,    // right bottom
	    0.25f, 0.0f,    // left bottom
	    0.25f,  0.5f,   // left
	};
	_ctx->hexleftuvbuffer = genBuffer(hex_left_uv_buffer_data, sizeof(hex_left_uv_buffer_data));
	static const GLfloat hex_right_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.25f, 1.0f,    // left top
	    0.75f, 1.0f,    // right top
	    0.75f, 0.5f,    // right
	    0.75f, 0.0f,    // right bottom
	    0.25f, 0.0f,    // left bottom
	    0.0f,  0.5f,    // left
	};
	_ctx->hexrightuvbuffer = genBuffer(hex_right_uv_buffer_data, sizeof(hex_right_uv_buffer_data));
	static const GLfloat hex_top_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.5f,  0.5f,    // left top
	    0.5f,  0.5f,    // right top
	    1.0f,  0.5f,    // right
	    0.75f, 0.0f,    // right bottom
	    0.25f, 0.0f,    // left bottom
	    0.0f,  0.5f,    // left
	};
	_ctx->hextopuvbuffer = genBuffer(hex_top_uv_buffer_data, sizeof(hex_top_uv_buffer_data));
	static const GLfloat hex_bottom_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.25f, 1.0f,    // left top
	    0.75f, 1.0f,    // right top
	    1.0f,  0.5f,    // right
	    0.75f, 0.5f,    // right bottom
	    0.25f, 0.5f,    // left bottom
	    0.0f,  0.5f,    // left
	};
	_ctx->hexbottomuvbuffer = genBuffer(hex_bottom_uv_buffer_data, sizeof(hex_bottom_uv_buffer_data));
	static const GLfloat hex_top_right_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.5f,  0.5f,    // left top
	    0.5f,  0.5f,    // right top
	    0.75f, 0.5f,    // right
	    0.75f, 0.0f,    // right bottom
	    0.25f, 0.0f,    // left bottom
	    0.0f,  0.5f,    // left
	};
	_ctx->hextoprightuvbuffer = genBuffer(hex_top_right_uv_buffer_data, sizeof(hex_top_right_uv_buffer_data));
	static const GLfloat hex_bottom_left_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.25f, 1.0f,    // left top
	    0.75f, 1.0f,    // right top
	    1.0f,  0.5f,    // right
	    0.75f, 0.5f,    // right bottom
	    0.25f, 0.5f,    // left bottom
	    0.25f, 0.5f,    // left
	};
	_ctx->hexbottomleftuvbuffer = genBuffer(hex_bottom_left_uv_buffer_data, sizeof(hex_bottom_left_uv_buffer_data));
	static const GLfloat hex_bottom_right_uv_buffer_data[] = {
	    0.5f,  0.5f,    // center
	    0.25f, 1.0f,    // left top
	    0.75f, 1.0f,    // right top
	    0.75f, 0.5f,    // right
	    0.75f, 0.5f,    // right bottom
	    0.25f, 0.5f,    // left bottom
	    0.0f,  0.5f,    // left
	};
	_ctx->hexbottomrightuvbuffer = genBuffer(hex_bottom_right_uv_buffer_data, sizeof(hex_bottom_right_uv_buffer_data));

	static const unsigned short hexindices[] = { 0, 2, 1,
	                                             0, 3, 2,
	                                             0, 4, 3,
	                                             0, 5, 4,
	                                             0, 6, 5,
	                                             0, 1, 6 };  // Triangles
	glGenBuffers(1, &_ctx->hexidxbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ctx->hexidxbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(hexindices), hexindices, GL_STATIC_DRAW);

	static const unsigned short squareindices[] = { 2,1,0,
	                                       3,2,0};
	glGenBuffers(1, &_ctx->squareidxbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ctx->squareidxbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareindices), squareindices, GL_STATIC_DRAW);

	doGLError("MiniMapRenderer::MiniMapRenderer");
}

MiniMapRenderer::~MiniMapRenderer() {
	glDeleteBuffers(1, &_ctx->squarevertexbuffer);
	glDeleteBuffers(1, &_ctx->squareuvbuffer);
	glDeleteBuffers(1, &_ctx->hexvertexbuffer);
	glDeleteBuffers(1, &_ctx->hexuvbuffer);
	glDeleteBuffers(1, &_ctx->hexleftvertexbuffer);
	glDeleteBuffers(1, &_ctx->hexleftuvbuffer);
	glDeleteBuffers(1, &_ctx->hexrightvertexbuffer);
	glDeleteBuffers(1, &_ctx->hexrightuvbuffer);
	glDeleteBuffers(1, &_ctx->hextopvertexbuffer);
	glDeleteBuffers(1, &_ctx->hextopuvbuffer);
	glDeleteBuffers(1, &_ctx->hexbottomvertexbuffer);
	glDeleteBuffers(1, &_ctx->hexbottomuvbuffer);
	glDeleteBuffers(1, &_ctx->hextoprightvertexbuffer);
	glDeleteBuffers(1, &_ctx->hextoprightuvbuffer);
	glDeleteBuffers(1, &_ctx->hexbottomrightvertexbuffer);
	glDeleteBuffers(1, &_ctx->hexbottomrightuvbuffer);
	glDeleteBuffers(1, &_ctx->hexbottomleftvertexbuffer);
	glDeleteBuffers(1, &_ctx->hexbottomleftuvbuffer);

	glDeleteVertexArrays(1, &_ctx->vertexArrayID);

	if (_ctx->miniMap) {
		free(_ctx->miniMap);
		glDeleteTextures(1, &_ctx->miniMapTexture);
	}
}

void MiniMapRenderer::updateMapDimensions(int width, int height) {
	Map *map = _ctx->map.get();
	_ctx->width = width;
	_ctx->height = height;

	_ctx->width = width;
	_ctx->height = height;

	_ctx->maphfactor = _ctx->height;
	_ctx->mapwfactor = _ctx->width;

	// Calculate the mini map scaling factors.
	if (_ctx->maphfactor < _ctx->mapwfactor) {
		_ctx->mapwfactor = (_ctx->maphfactor / _ctx->mapwfactor);
		_ctx->maphfactor = 1;
	} else {
		_ctx->maphfactor = (_ctx->mapwfactor / _ctx->maphfactor);
		_ctx->mapwfactor = 1;
	}
	_ctx->maphfactor *= HEX_HEIGHT_ADJUST;
	float s1 = 1.0f / ((float)map->numRows*_ctx->maphfactor);
	float s2 = 1.0f / ((float)(map->numCols/2)*1.5f*_ctx->mapwfactor);
	float miniScale = s1<s2?s1:s2;
	_ctx->maphfactor *= miniScale;
	_ctx->mapwfactor *= miniScale;

	free(_ctx->miniMap);
	glDeleteTextures(1, &_ctx->miniMapTexture);
	int miniMapMemSize = _ctx->height * _ctx->width * sizeof(byte) * 4;
	_ctx->miniMap = (byte *)malloc(miniMapMemSize);
	memset(_ctx->miniMap, 255, miniMapMemSize);
	_ctx->miniMapDirty = true;
	glGenTextures(1, &_ctx->miniMapTexture);

	// Store the mini map as a texture to avoid generating it needlessly (it is expensive).
	glBindTexture(GL_TEXTURE_2D, _ctx->miniMapTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _ctx->width, _ctx->height,
	             0, GL_RGBA, GL_UNSIGNED_BYTE, _ctx->miniMap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//	glGenerateMipmap(GL_TEXTURE_2D);
}

void MiniMapRenderer::resizeMap(int width, int height) {
	updateMapDimensions(width, height);
}

void MiniMapRenderer::renderMap() {
	Map *map = _ctx->map.get();
	if (!_ctx->miniMapDirty) {
		GLboolean depthTest;
		glGetBooleanv(GL_DEPTH_TEST, &depthTest);
		if (depthTest) glDisable(GL_DEPTH_TEST);
		glUseProgram(_ctx->texpassprog->id());
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
		_ctx->miniMapLocMarker();
		return;
	}
	int numX;
	int numY;
	numX = map->numCols;
	numY = map->numRows;

	float c[] = { 0.0f, 0.0f };
	int row, col;
	int halfX = (numX / 2);
	int halfY = (numY / 2);

	// Map backing color.
	glUseProgram(_ctx->hexminiprog->id());
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
		int mcol = halfY+col;
		for (row = -halfY; row <= halfY; row++) {
			c[1] = (2.0f * row)+(mcol%2?1.0f:0.0f);
			int mrow = halfX-row;
			if (mrow >= 0 && mrow < map->numRows && mcol >=0 && mcol < map->numCols) {
				_ctx->drawHex(c , mrow, mcol);
			}
		}
	}

	//_ctx->saveMiniMap();
	_ctx->miniMapLocMarker();
}

void MiniMapRenderer::miniMapDirty() {
	_ctx->miniMapDirty = true;
}

void MiniMapRenderer::getMapDisplayCenter(unsigned int& centerRow, unsigned int& centerCol,
                         unsigned int& rowsDisplayed, unsigned int& colsDisplayed) {
	centerRow = _ctx->centerRow;
	centerCol = _ctx->centerCol;
	rowsDisplayed = static_cast<unsigned int>(1.0f / _ctx->maphfactor);
	colsDisplayed = static_cast<unsigned int>((1.0f / _ctx->mapwfactor) * 1.25);
}

void MiniMapRenderer::setMapDisplayCenter(int displayCenterRow,
                                int displayCenterCol) {
	_ctx->centerRow = displayCenterRow;
	_ctx->centerCol = displayCenterCol;
}

void MiniMapRenderer::getMiniMapPostion(int *x, int *y,
                       int *width, int *height) {
	*x = 0;
	*y = 0;
	*width = _ctx->width;
	*height = _ctx->height;
}

bool MiniMapRenderer::setCenterMiniMap(float x, float y, int *hexcol, int *hexrow) {
	Bool ret = FALSE;
	int col = (x / (1.5f * _ctx->mapwfactor)) + (_ctx->map->numCols / 2);
	int row = -(y / (2.0f * _ctx->maphfactor) - (col%2?1.0f:0.0f))
	                   + (_ctx->map->numRows / 2);
	if (col >=0 && col < _ctx->map->numCols && row >= 0 && row < _ctx->map->numRows) {
		_ctx->map->col = col;
		_ctx->map->row = row;
		setMapDisplayCenter(_ctx->map->row, _ctx->map->col);
		if (hexcol) *hexcol = col;
		if (hexrow) *hexrow = row;
		ret = TRUE;
	}
	return ret;
}

} }

