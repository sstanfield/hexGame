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
#include "imageutils.h"
#include "gl_util.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define HEX_HEIGHT_ADJUST .86330935

namespace hexgame { namespace render {

static constexpr float zoomLevels[] = { .2f, .1f, .075f, .05f, .03f };

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

static void getTileColor(state::TileType type, float *color) {
	switch (type) {
		case state::TileType::Grass: color[0] = 0.0; color[1]  = .76; color[2] = 0.0; break;
		case state::TileType::Forest: color[0] = 0.22; color[1] = .416; color[2] = 0.122; break;
		case state::TileType::Swamp: color[0] = 0.0; color[1]  = .76; color[2] = 0.522; break;

		// XXX Fixme...
		case state::TileType::Desert: color[0] = 0.0; color[1] = 0.0; color[2] = 0.0; break;
		case state::TileType::Hill: color[0] = 0.863; color[1] = .667; color[2] = 0.208; break;
		case state::TileType::Mountain: color[0] = 0.475; color[1] = .467; color[2] = 0.443; break;
		case state::TileType::Water: color[0] = 0.09; color[1] = .886; color[2] = 0.902; break;

		// XXX Fixme...
		case state::TileType::Temple: color[0] = 0.5; color[1] = 0.5; color[2] = 0.5; break;

		// XXX Fixme...
		case state::TileType::Ruin: color[0] = 0.6; color[1] = 0.6; color[2] = 0.6; break;
		case state::TileType::City: color[0] = 0.0; color[1] = 0.0; color[2] = 0.0; break;
	}
}

struct MapRenderer::CTX {
	state::Map::s_ptr map;

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

	bool   miniMapDirty;
	byte  *miniMap;
	GLuint miniMapTexture;

	GLuint vertexArrayID;

	// Shaders
	Shader::s_ptr hexprog;
	Shader::s_ptr hexminiprog;
	Shader::s_ptr minilocprog;
	Shader::s_ptr hexoverlayprog;
	Shader::s_ptr texpassprog;

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
	GLuint cityTex;
	GLuint unitDefault;


	GLuint getTileTexture(state::TileType type) {
		GLuint ret;
		switch (type) {
			case state::TileType::Grass: ret = grassTex; break;
			case state::TileType::Forest: ret = forestTex; break;
			case state::TileType::Swamp: ret = swampTex; break;
			            //	case state::TileType::Desert: ret = desertTex; break;
			case state::TileType::Hill: ret = hillTex; break;
			case state::TileType::Mountain: ret = mountainTex; break;
			case state::TileType::Water: ret = waterTex; break;
			            //	case state::TileType::Temple: ret = templeTex; break;
			            //	case state::TileType::Ruin: ret = ruinTex; break;
			case state::TileType::City: ret = cityTex; break;
		}
		return ret;
	}

	void drawHex(const float *center, bool mini,
			uint row, uint col) {
		bool isSelected = col==map->col&&row==map->row;
		state::TileType& type = map->tile(row, col).type;

		if (mini) {
			glUseProgram(hexminiprog->id());
			glUniform1f(mini_heightFactor, minihfactor);
			glUniform1f(mini_widthFactor, miniwfactor);

			glUniform2fv(mini_centerId, 1, center);

			float color[4] = {0.0, 0.0, 0.0, 1.0};
			getTileColor(type, color);
			glUniform4fv(mini_colorId, 1, color);
		} else {
			glUseProgram(hexprog->id());
			glUniform1f(heightFactor, maphfactor);
			glUniform1f(widthFactor, mapwfactor);

			glUniform2fv(centerId, 1, center);
			if (isSelected) {
				float bcolor[] = {1.0, 0, 0, 1.0};
				glUniform4fv(borderColor, 1, bcolor);
				glUniform1f(borderThickness, .85 - ((float)zoomLevel * .01));
				glUniform1i(useBorderId, 1);
			} else {
				float bcolor[] = {0, 0, 0, 1.0};
				glUniform4fv(borderColor, 1, bcolor);
				glUniform1f(borderThickness, .95 - ((float)zoomLevel * .01));
				glUniform1i(useBorderId, 1);
			}

			GLuint tileTex = getTileTexture(type);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tileTex);
			glUniform1i(textureID0, 0);
		}

		GLuint vertexbuffer = hexvertexbuffer;
		GLuint uvbuffer = hexuvbuffer;
		if (col == 0) {
			vertexbuffer = hexleftvertexbuffer;
			uvbuffer = hexleftuvbuffer;
		}
		if (col == (map->numCols() - 1)) {
			vertexbuffer = hexrightvertexbuffer;
			uvbuffer = hexrightuvbuffer;
		}
		if (row == 0 && (col % 2)) {
			if (col == (map->numCols() - 1)) {
				vertexbuffer = hextoprightvertexbuffer;
				uvbuffer = hextoprightuvbuffer;
			} else {
				vertexbuffer = hextopvertexbuffer;
				uvbuffer = hextopuvbuffer;
			}
		}
		if (row == (map->numRows() - 1) && !(col % 2)) {
			if (col == (map->numCols() - 1)) {
				vertexbuffer = hexbottomrightvertexbuffer;
				uvbuffer = hexbottomrightuvbuffer;
			} else {
				vertexbuffer = hexbottomvertexbuffer;
				uvbuffer = hexbottomuvbuffer;
			}
		}
		if (row == (map->numRows() - 1) && col == 0) {
			vertexbuffer = hexbottomleftvertexbuffer;
			uvbuffer = hexbottomleftuvbuffer;
		}

		int aPos = 0, aTex = 1, aBarry = 2;
#ifdef __EMSCRIPTEN__
		aPos = glGetAttribLocation (hexprog->id(), "Position" );
		if (!mini) {
			aTex = glGetAttribLocation(hexprog->id(), "TexCoord" );
			aBarry = glGetAttribLocation(hexprog->id(), "BarryCoord" );
		}
#endif
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(aPos);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(aPos,
				3,                  // size
				GL_FLOAT,            // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
	            );
		if (!mini) {
			// 2nd attribute buffer : texture coords
			glEnableVertexAttribArray(aTex);
			glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
			glVertexAttribPointer(aTex,
					2,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
		            );

			// 3nd attribute buffer : used for borders
			glEnableVertexAttribArray(aBarry);
			glBindBuffer(GL_ARRAY_BUFFER, hexbarrybuffer);
			glVertexAttribPointer(aBarry,                  // attribute 1. No particular reason for 1, but must match the layout in the shader.
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
		            );
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hexidxbuffer);
		glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, NULL);

		if (!mini) {
			GLboolean depthTest;
			glGetBooleanv(GL_DEPTH_TEST, &depthTest);
			if (depthTest) glDisable(GL_DEPTH_TEST);
			if (map->tile(row, col).numUnits > 0) {
				glBindTexture(GL_TEXTURE_2D, unitDefault);
				//			printf("XXX Draw unit %d, %d\n", row, col);
				glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, NULL);
			}
			if (map->selectedUnit && map->tile(row, col).currentMoveCost &&
					map->tile(row, col).currentMoveCost <= map->selectedUnit->movement) {
				//			printf("XXX MARK MOVE %d, %d\n", row, col);
				glUseProgram(hexoverlayprog->id());
				glUniform1f(hexOverlayHeightFactor, maphfactor);
				glUniform1f(hexOverlayWidthFactor, mapwfactor);
				glUniform2fv(hexOverlayCenterId, 1, center);
				glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_SHORT, NULL);
			}
			if (depthTest) glEnable(GL_DEPTH_TEST);
			glDisableVertexAttribArray(aTex);
			glDisableVertexAttribArray(aBarry);
		}
		glDisableVertexAttribArray(aPos);
	}

	void saveMiniMap() {
		if (!miniMapDirty) return;
		// Save map so we can blit it directly until dirty.
		if (miniMap) {
			glReadPixels(miniStartX, miniStartY, miniwidth, miniheight,
					GL_RGBA, GL_UNSIGNED_BYTE, miniMap);
			glBindTexture(GL_TEXTURE_2D, miniMapTexture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, miniwidth, miniheight,
					GL_RGBA, GL_UNSIGNED_BYTE, miniMap);
			miniMapDirty = false;
		}
	}

	void miniMapLocMarker() {
		// Mini Map location marker.
		int halfX = (map->numCols() / 2);
		int halfY = (map->numRows() / 2);
		glUseProgram(minilocprog->id());
		float hfac = (1.0f / maphfactor) * minihfactor;// / (float)(map->numRows);
		float wfac = (1.0f / mapwfactor) * miniwfactor;// / ((float)map->numCols*.75f);
		glUniform1f(miniloc_heightFactor, hfac);
		glUniform1f(miniloc_widthFactor, wfac);
		glUniform1f(miniloc_centerhFactor, minihfactor);
		glUniform1f(miniloc_centerwFactor, miniwfactor);
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

MapRenderer::MapRenderer(state::Map::s_ptr map, int width, int height,
                         std::string assetDir, std::string shaderDir) {
	_ctx = std::make_unique<CTX>();
	_ctx->zoomLevel = 1;
	_ctx->mapScale = zoomLevels[_ctx->zoomLevel];
	_ctx->map = map;
	_ctx->centerRow = map->row;
	_ctx->centerCol = map->col;
	_ctx->miniMapDirty = true;
	_ctx->hexprog = LoadShadersFromFile(shaderDir + "HexVertex.glsl",
	                                  shaderDir + "HexFragment.glsl");
	_ctx->heightFactor = glGetUniformLocation(_ctx->hexprog->id(), "heightFactor");
	_ctx->widthFactor = glGetUniformLocation(_ctx->hexprog->id(), "widthFactor");
	_ctx->centerId = glGetUniformLocation(_ctx->hexprog->id(), "center");
	_ctx->useBorderId = glGetUniformLocation(_ctx->hexprog->id(), "useBorder");
	_ctx->textureID0 = glGetUniformLocation(_ctx->hexprog->id(), "Texture0");
	_ctx->borderColor = glGetUniformLocation(_ctx->hexprog->id(), "borderColor");
	_ctx->borderThickness = glGetUniformLocation(_ctx->hexprog->id(), "borderThickness");

	_ctx->hexminiprog = LoadShadersFromFile(shaderDir + "HexMiniVertex.glsl",
	                                      shaderDir + "HexMiniFragment.glsl");
	_ctx->mini_heightFactor = glGetUniformLocation(_ctx->hexminiprog->id(), "heightFactor");
	_ctx->mini_widthFactor = glGetUniformLocation(_ctx->hexminiprog->id(), "widthFactor");
	_ctx->mini_centerId = glGetUniformLocation(_ctx->hexminiprog->id(), "center");
	_ctx->mini_colorId = glGetUniformLocation(_ctx->hexminiprog->id(), "tileColor");

	_ctx->minilocprog = LoadShadersFromFile(shaderDir + "MiniLocVertex.glsl",
	                                      shaderDir + "MiniLocFragment.glsl");
	_ctx->miniloc_heightFactor = glGetUniformLocation(_ctx->minilocprog->id(), "heightFactor");
	_ctx->miniloc_widthFactor = glGetUniformLocation(_ctx->minilocprog->id(), "widthFactor");
	_ctx->miniloc_centerhFactor = glGetUniformLocation(_ctx->minilocprog->id(), "centerhFactor");
	_ctx->miniloc_centerwFactor = glGetUniformLocation(_ctx->minilocprog->id(), "centerwFactor");
	_ctx->miniloc_centerId = glGetUniformLocation(_ctx->minilocprog->id(), "center");
	_ctx->miniloc_colorId = glGetUniformLocation(_ctx->minilocprog->id(), "tileColor");

	_ctx->hexoverlayprog = LoadShadersFromFile(shaderDir + "HexOverlayVertex.glsl",
	                                         shaderDir + "HexOverlayFragment.glsl");
	_ctx->hexOverlayHeightFactor = glGetUniformLocation(_ctx->hexoverlayprog->id(), "heightFactor");
	_ctx->hexOverlayWidthFactor = glGetUniformLocation(_ctx->hexoverlayprog->id(), "widthFactor");
	_ctx->hexOverlayCenterId = glGetUniformLocation(_ctx->hexoverlayprog->id(), "center");
	_ctx->texpassprog = LoadShadersFromFile(shaderDir + "TexPassVertex.glsl",
	                                      shaderDir + "TexPassFragment.glsl");
	_ctx->texpass_textureID0 = glGetUniformLocation(_ctx->texpassprog->id(), "Texture0");

#ifndef __EMSCRIPTEN__
	glGenVertexArrays(1, &_ctx->vertexArrayID);
	glBindVertexArray(_ctx->vertexArrayID);
#endif

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
	static const GLfloat hex_barry_buffer_data[] = {
	    1.0f,  0.0f,  0.0f,    // center
	    0.0f,  1.0f,  0.0f,    // left top
	    0.0f,  1.0f,  0.0f,    // right top
	    0.0f,  1.0f,  0.0f,    // right
	    0.0f,  1.0f,  0.0f,    // right bottom
	    0.0f,  1.0f,  0.0f,    // left bottom
	    0.0f,  1.0f,  0.0f,    // left
	};
	_ctx->hexbarrybuffer = genBuffer(hex_barry_buffer_data, sizeof(hex_barry_buffer_data));
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

	_ctx->borderTex = read_png_file((assetDir + "tiles/border_tile.png").c_str());
	_ctx->waterTex  = read_png_file((assetDir + "tiles/water_tile.png").c_str());
	_ctx->grassTex  = read_png_file((assetDir + "tiles/grass_tile.png").c_str());
	_ctx->hillTex  = read_png_file((assetDir + "tiles/hill_tile.png").c_str());
	_ctx->forestTex  = read_png_file((assetDir + "tiles/forest_tile.png").c_str());
	_ctx->mountainTex  = read_png_file((assetDir + "tiles/mountain_tile.png").c_str());
	_ctx->swampTex  = read_png_file((assetDir + "tiles/swamp_tile.png").c_str());
	_ctx->cityTex  = read_png_file((assetDir + "tiles/citycenter_tile.png").c_str());
	_ctx->unitDefault  = read_png_file((assetDir + "units/default_unit.png").c_str());
	doGLError("MapRenderer::MapRenderer");
}

MapRenderer::~MapRenderer() {
	glDeleteBuffers(1, &_ctx->squarevertexbuffer);
	glDeleteBuffers(1, &_ctx->squareuvbuffer);
	glDeleteBuffers(1, &_ctx->hexvertexbuffer);
	glDeleteBuffers(1, &_ctx->hexbarrybuffer);
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

	glDeleteTextures(1, &_ctx->borderTex);
	glDeleteTextures(1, &_ctx->waterTex);
	glDeleteTextures(1, &_ctx->grassTex);
	glDeleteTextures(1, &_ctx->hillTex);
	glDeleteTextures(1, &_ctx->forestTex);
	glDeleteTextures(1, &_ctx->mountainTex);
	glDeleteTextures(1, &_ctx->swampTex);
	glDeleteTextures(1, &_ctx->cityTex);

#ifndef __EMSCRIPTEN__
	glDeleteVertexArrays(1, &_ctx->vertexArrayID);
#endif

	if (_ctx->miniMap) {
		free(_ctx->miniMap);
		glDeleteTextures(1, &_ctx->miniMapTexture);
	}
}

void MapRenderer::updateMapDimensions(int width, int height) {
	state::Map *map = _ctx->map.get();
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
	_ctx->mapwidth = width;// * .7;
	_ctx->mapheight = height;// * .85;
	_ctx->mapStartX = 0;//width * .01;
	_ctx->mapStartY = 0;//height - _ctx->mapheight - (height * .01);
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
	float s1 = 1.0f / ((float)map->numRows()*_ctx->minihfactor);
	float s2 = 1.0f / ((float)(map->numCols()/2)*1.5f*_ctx->miniwfactor);
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
	_ctx->miniMapDirty = true;
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

void MapRenderer::resizeMap(int width, int height) {
	updateMapDimensions(width, height);
}

void MapRenderer::renderMap(bool mini) {
	state::Map *map = _ctx->map.get();
	if (mini) {
		/*glViewport(_ctx->miniStartX,
		           _ctx->miniStartY,
		           _ctx->miniwidth,
		           _ctx->miniheight);*/
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
			/*glViewport(0,
			           0,
			           _ctx->width,
			           _ctx->height);*/
			return;
		}
	/*} else {
		glViewport(_ctx->mapStartX,
		           _ctx->mapStartY,
		           _ctx->mapwidth,
		           _ctx->mapheight);*/
	}
	int numX;
	int numY;
	if (mini) {
		numX = map->numCols();
		numY = map->numRows();
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
		int mcol = mini?halfY+col:_ctx->centerCol+col;
		for (row = -halfY; row <= halfY; row++) {
			c[1] = (2.0f * row)+(mcol%2?1.0f:0.0f);
			int mrow = mini?halfX-row:_ctx->centerRow-row;
			if (mrow >= 0 && mrow < (int)map->numRows() && mcol >=0 && mcol < (int)map->numCols()) {
				_ctx->drawHex(c , mini, mrow, mcol);
			}
		}
	}

	if (mini) {
		_ctx->saveMiniMap();
		_ctx->miniMapLocMarker();
	}

	/*glViewport(0,
	           0,
	           _ctx->width,
	           _ctx->height);*/
}

void MapRenderer::miniMapDirty() {
	_ctx->miniMapDirty = true;
}

void MapRenderer::getMapDisplayCenter(unsigned int& centerRow, unsigned int& centerCol,
                         unsigned int& rowsDisplayed, unsigned int& colsDisplayed) {
	centerRow = _ctx->centerRow;
	centerCol = _ctx->centerCol;
	rowsDisplayed = static_cast<unsigned int>(1.0f / _ctx->maphfactor);
	colsDisplayed = static_cast<unsigned int>((1.0f / _ctx->mapwfactor) * 1.25);
}

void MapRenderer::setMapDisplayCenter(int displayCenterRow,
                                int displayCenterCol) {
	_ctx->centerRow = displayCenterRow;
	_ctx->centerCol = displayCenterCol;
}

void MapRenderer::zoomInMap() {
	if (_ctx->zoomLevel > 0) {
		_ctx->zoomLevel--;
		_ctx->mapScale = zoomLevels[_ctx->zoomLevel];
		updateMapDimensions(_ctx->width, _ctx->height);
	}
}

void MapRenderer::zoomOutMap() {
	int maxLevel = sizeof(zoomLevels) / sizeof(float);
	if (_ctx->zoomLevel < (maxLevel - 1)) {
		_ctx->zoomLevel++;
		_ctx->mapScale = zoomLevels[_ctx->zoomLevel];
		updateMapDimensions(_ctx->width, _ctx->height);
	}
}


void MapRenderer::getMapPostion(int *x, int *y,
                   int *width, int *height) {
	*x = _ctx->mapStartX;
	*y = _ctx->height - (_ctx->mapStartY + _ctx->mapheight);
	*width = _ctx->mapwidth;
	*height = _ctx->mapheight;
}

void MapRenderer::getMiniMapPostion(int *x, int *y,
                       int *width, int *height) {
	*x = _ctx->miniStartX;
	*y = _ctx->height - (_ctx->miniStartY + _ctx->miniheight);
	*width = _ctx->miniwidth;
	*height = _ctx->miniheight;
}

bool MapRenderer::setCenterMiniMap(float x, float y, int *hexcol, int *hexrow) {
	bool ret = false;
	int col = (x / (1.5f * _ctx->miniwfactor)) + (_ctx->map->numCols() / 2);
	int row = -(y / (2.0f * _ctx->minihfactor) - (col%2?1.0f:0.0f))
	                 + (_ctx->map->numRows() / 2);
	if (col >=0 && col < (int)_ctx->map->numCols() && row >= 0 && row < (int)_ctx->map->numRows()) {
		_ctx->map->col = col;
		_ctx->map->row = row;
		setMapDisplayCenter(_ctx->map->row, _ctx->map->col);
		if (hexcol) *hexcol = col;
		if (hexrow) *hexrow = row;
		ret = true;
	}
	return ret;
}

bool MapRenderer::setCenterMap(float x, float y, int *hexcol, int *hexrow) {
	bool ret = false;
	int col = _ctx->centerCol + (int)(x / (1.5f * _ctx->mapwfactor) +
	                                  (x<0?-.5f:.5f));
	int row = _ctx->centerRow - (int)((y / (2.0f * _ctx->maphfactor) +
	                                   (y<0?-.5f:.5f)) - (col%2?.5f:0.0f));
	if (col >=0 && col < (int)_ctx->map->numCols() && row >= 0 && row < (int)_ctx->map->numRows()) {
		_ctx->map->col = col;
		_ctx->map->row = row;
		if (hexcol) *hexcol = col;
		if (hexrow) *hexrow = row;
		ret = true;
	}
	return ret;
}

void MapRenderer::selectHex(unsigned int col, unsigned int row) {
	state::Map *map = _ctx->map.get();
	if (map->selectedUnit) {
		if (map->selectedUnit->row == row && map->selectedUnit->col == col) {
			// Select again so deselect.
			map->deSelectUnit();
		} else {  // Move here if it can.
			if (map->tile(row, col).currentMoveCost <= map->selectedUnit->movement) {
				map->selectedUnit->movement -= map->tile(row, col).currentMoveCost;
				map->moveUnit(map->selectedUnit, row, col);
//				selectUnit(map, map->selectedUnit);
				map->deSelectUnit();
			}
		}
	} else {  // If there is a unit to select then select it.
		if (map->tile(row, col).numUnits) {
			map->selectUnit(map->tile(row, col).units[0]);
		}
	}
}

void MapRenderer::moveUp() {
	state::Map *map = _ctx->map.get();
	unsigned int centerRow, centerCol, rowsDisplayed, colsDisplayed;
	if (map->row > 0) map->row--;
	getMapDisplayCenter(centerRow, centerCol,
	                    rowsDisplayed, colsDisplayed);
	if (map->row < (centerRow - (rowsDisplayed / 2))) {
		setMapDisplayCenter(map->row, centerCol);
	}
}

void MapRenderer::moveDown() {
	state::Map *map = _ctx->map.get();
	unsigned int centerRow, centerCol, rowsDisplayed, colsDisplayed;
	if (map->row < (map->numRows()-1)) map->row++;
	getMapDisplayCenter(centerRow, centerCol,
	                    rowsDisplayed, colsDisplayed);
	if (map->row > (centerRow + (rowsDisplayed / 2))) {
		setMapDisplayCenter(map->row, centerCol);
	}
}

} }

