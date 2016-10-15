#include "glwidget.h"
#include "render/pc/GL/glew.h"

namespace hexgame {
namespace gui {

GLWidget::GLWidget() {
	tb::g_renderer->AddListener(this);
}

GLWidget::~GLWidget() {
	tb::g_renderer->RemoveListener(this);
}

void GLWidget::OnPaint(const PaintProps &paint_props) {
	if (!init) {
		initGL();
		init = true;
	}
	GLint v[4];
	glGetIntegerv(GL_VIEWPORT, v);
	int dx, dy;
	tb::g_renderer->GetTranslate(&dx, &dy);
	tb::g_renderer->BeginNativeRender();
	tb::TBRect r = GetRect();
	tb::TBRect padr = GetPaddingRect();
	glViewport(dx+padr.x, (v[3]-(dy+r.h))+padr.y, padr.w, padr.h);
	render();
	tb::g_renderer->EndNativeRender();
}

}
}

