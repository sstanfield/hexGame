#pragma once

#include "glwidget.h"
#include "render/maprender.h"

#include <memory>

namespace hexgame {
namespace gui {

/**
 * Base class for widgets that render themselves using OpenGL.
 */
class MapWidget : public GLWidget {
public:
	TBOBJECT_SUBCLASS(MapWidget, GLWidget);

	MapWidget();
	~MapWidget() override;

	bool OnEvent(const tb::TBWidgetEvent &ev) override;
	void OnResized(int old_w, int old_h) override;
	void OnInflate(const tb::INFLATE_INFO &info) override;

protected:
	void initGL() override;
	void render() override;

private:
	Map *map;
	std::unique_ptr<render::MapRenderer> maprenderer;
};

}
}
