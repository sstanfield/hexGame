#pragma once

#include "glwidget.h"
#include "render/maprender.h"

namespace hexgame {
namespace gui {

/**
 * Base class for widgets that render themselves using OpenGL.
 */
class MapWidget : public GLWidget {
public:
	TBOBJECT_SUBCLASS(MapWidget, GLWidget);

	MapWidget();
	virtual ~MapWidget();

	virtual void OnResized(int old_w, int old_h);

protected:
	virtual void initGL();
	virtual void render();

private:
	Map *map;
	MapRenderer maprenderer;
};

}
}
