// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tbrenderer.h"

#include "render/pc/GL/glew.h"
#include "render/glcapabilities.h"

#include "tb_bitmap_fragment.h"
#include "tb_system.h"

#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "settings.h"
#include "render/shaders.h"

#include <assert.h>
#include <stdio.h>

namespace hexgame { namespace render {

// == Batching ====================================================================================

GLuint g_current_texture = (GLuint)-1;

void BindBitmap(tb::TBBitmap *bitmap)
{
	GLuint texture = bitmap ? static_cast<TBBitmap*>(bitmap)->m_texture : 0;
	if (texture != g_current_texture)
	{
		g_current_texture = texture;
		glBindTexture(GL_TEXTURE_2D, g_current_texture);
	}
}

// == TBBitmapMURS ==================================================================================

TBBitmap::TBBitmap(TBRenderer *renderer)
	: m_renderer(renderer), m_w(0), m_h(0), m_texture(0)
{
}

TBBitmap::~TBBitmap()
{
	// Must flush and unbind before we delete the texture
	m_renderer->FlushBitmap(this);
	if (m_texture == g_current_texture)
		BindBitmap(nullptr);

	glDeleteTextures(1, &m_texture);
}

bool TBBitmap::Init(int width, int height, tb::uint32 *data)
{
	assert(width == tb::TBGetNearestPowerOfTwo(width));
	assert(height == tb::TBGetNearestPowerOfTwo(height));

	m_w = width;
	m_h = height;

	glGenTextures(1, &m_texture);
	BindBitmap(this);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	SetData(data);

	return true;
}

void TBBitmap::SetData(tb::uint32 *data)
{
	m_renderer->FlushBitmap(this);
	BindBitmap(this);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_w, m_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

// == TBRenderer ================================================================================

TBRenderer::TBRenderer()
{
	initGL();
	AddListener(this);
}

void TBRenderer::initGL() {
	shaders = LoadShadersFromFile((*Settings::i()->getAssetDir() + "shaders/tbVert.glsl").c_str(),
	                              (*Settings::i()->getAssetDir() + "shaders/tbFrag.glsl").c_str());
	mvp_handle = glGetUniformLocation(shaders, "MVP");
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*VERTEX_BATCH_SIZE, NULL, GL_DYNAMIC_DRAW);
}

void TBRenderer::BeginPaint(int render_target_w, int render_target_h)
{
	tb::TBRendererBatcher::BeginPaint(render_target_w, render_target_h);
	m_render_target_w = render_target_w;
	m_render_target_h = render_target_h;
	Setup(render_target_w, render_target_h);
}

void TBRenderer::EndPaint()
{
	tb::TBRendererBatcher::EndPaint();
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

tb::TBBitmap *TBRenderer::CreateBitmap(int width, int height, tb::uint32 *data)
{
	TBBitmap *bitmap = new TBBitmap(this);
	if (!bitmap || !bitmap->Init(width, height, data))
	{
		delete bitmap;
		return nullptr;
	}
	return bitmap;
}

void TBRenderer::RenderBatch(Batch *batch)
{
	BindBitmap(batch->bitmap);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*batch->vertex_count, &batch->vertex[0], GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, batch->vertex_count);
}

void TBRenderer::SetClipRect(const tb::TBRect &rect)
{
	glScissor(m_clip_rect.x, m_screen_rect.h - (m_clip_rect.y + m_clip_rect.h), m_clip_rect.w, m_clip_rect.h);
}

void TBRenderer::OnContextLost() {
}

void TBRenderer::OnContextRestored() {
	initGL();
}

void TBRenderer::BeginNativeRender() {
	FlushAllInternal();  // Do this so native gl widgets render "over" the background.
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void TBRenderer::EndNativeRender() {
	Setup(m_render_target_w, m_render_target_h);
}

void TBRenderer::Setup(int render_target_w, int render_target_h)
{
	g_current_texture = (GLuint)-1;

	glm::mat4 mvp;
	mvp = glm::ortho(0.f, (float)render_target_w, (float)render_target_h, 0.f);
	glViewport(0, 0, render_target_w, render_target_h);
	glScissor(0, 0, render_target_w, render_target_h);

	GLCapabilities::i()->enable(GL_BLEND);
	GLCapabilities::i()->disable(GL_DEPTH_TEST);
	GLCapabilities::i()->enable(GL_SCISSOR_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
	      0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	      2,                  // size
	      GL_FLOAT,           // type
	      GL_FALSE,           // normalized?
	      sizeof(Vertex),     // stride
	      (void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
	      1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	      2,                  // size
	      GL_FLOAT,           // type
	      GL_FALSE,           // normalized?
	      sizeof(Vertex),     // stride
	      (void*)8            // array buffer offset
	);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
	      2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	      4,                  // size
	      GL_UNSIGNED_BYTE,   // type
	      GL_TRUE,            // normalized?
	      sizeof(Vertex),     // stride
	      (void*)16           // array buffer offset
	);
	glUseProgram(shaders);
	glUniformMatrix4fv(mvp_handle, 1, GL_FALSE, &mvp[0][0]);
}

} }

