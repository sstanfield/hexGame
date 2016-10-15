// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#pragma once

#include "tb_types.h"

#include "renderers/tb_renderer_batcher.h"

namespace hexgame { namespace render {

class TBRenderer;

class TBBitmap : public tb::TBBitmap
{
public:
	TBBitmap(TBRenderer *renderer);
	~TBBitmap();
	bool Init(int width, int height, tb::uint32 *data);
	virtual int Width() { return m_w; }
	virtual int Height() { return m_h; }
	virtual void SetData(tb::uint32 *data);
public:
	TBRenderer *m_renderer;
	int m_w, m_h;
	unsigned int m_texture;
};

class TBRenderer : public tb::TBRendererBatcher, public tb::TBRendererListener
{
public:
	TBRenderer();
	void initGL();

	// == TBRenderer ====================================================================

	virtual void BeginPaint(int render_target_w, int render_target_h);
	virtual void EndPaint();

	virtual tb::TBBitmap *CreateBitmap(int width, int height, tb::uint32 *data);

	// == TBRendererBatcher ===============================================================

	virtual void RenderBatch(Batch *batch);
	virtual void SetClipRect(const tb::TBRect &rect);


	virtual void OnContextLost();
	virtual void OnContextRestored();

	virtual void BeginNativeRender();
	virtual void EndNativeRender();
private:
	unsigned int mvp_handle;
	unsigned int vertexbuffer;
	unsigned int shaders;
	int m_render_target_w;
	int m_render_target_h;
	void Setup(int render_target_w, int render_target_h);
};

} }

