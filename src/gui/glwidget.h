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
	virtual void OnPaint(const PaintProps &paint_props);

	virtual void OnContextLost() {}
	virtual void OnContextRestored() { init = false; }

protected:
	virtual void initGL() {}
	virtual void render() = 0;

private:
	bool init = false;
};

}
}
