#include "mapwidget.h"
#include "settings.h"

namespace hexgame {
namespace gui {

MapWidget::MapWidget() {
}

MapWidget::~MapWidget() {
	if (maprenderer) freeMapRenderer(maprenderer);
}

void MapWidget::OnResized(int old_w, int old_h) {
	tb::TBRect r = GetRect();
	if (maprenderer) resizeMap(maprenderer, r.w, r.h);
	GLWidget::OnResized(old_w, old_h);
}

void MapWidget::initGL() {
	if (maprenderer) freeMapRenderer(maprenderer);
	tb::TBRect r = GetRect();
	maprenderer = initMapRender(map, r.w, r.h, Settings::i()->getAssetDir()->c_str());
}

void MapWidget::render() {
	renderMap(maprenderer, FALSE);  // Main map.
}

}
}

