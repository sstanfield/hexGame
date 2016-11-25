#pragma once

#include "tb/tb_widgets.h"

namespace hexgame {
namespace gui {

/**
 * Base class for widgets that render themselves using OpenGL.
 */
class GLWidget : public tb::TBWidget, public tb::TBRendererListener {
public:
	TBOBJECT_SUBCLASS(GLWidget, tb::TBWidget);

	GLWidget();
	virtual ~GLWidget();
	void OnPaint(const PaintProps &paint_props) override;

	void OnContextLost() override {}
	void OnContextRestored() override { init = false; }

protected:
	virtual void initGL() {}
	virtual void render() = 0;

private:
	bool init = false;
};

}
}
