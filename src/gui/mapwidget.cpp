#include "Application.h"
#include "mapwidget.h"
#include "settings.h"
#include "tb_widgets_reader.h"

namespace hexgame {
namespace gui {

MapWidget::MapWidget() {
	map = Application::instance()->getMap();
	tb::TBRect r = GetRect();
	maprenderer = std::make_unique<render::MapRenderer>(map, r.w, r.h, Settings::i()->getAssetDir());
}

MapWidget::~MapWidget() {
}

bool MapWidget::OnEvent(const tb::TBWidgetEvent &ev)
{
	if (ev.type == tb::EVENT_TYPE_CUSTOM && ev.ref_id == tb::TBID("gamepad")) {
		//return procGamePad();
	}
	if (ev.type == tb::EVENT_TYPE_POINTER_MOVE) {
		int mx, my, mw, mh;
		maprenderer->getMapPostion(&mx, &my, &mw, &mh);
		int hw = mw / 2;
		int hh = mh / 2;
		float relX = (float)((ev.target_x-mx)-hw)/(float)hw;
		float relY = -(float)((ev.target_y-my)-hh)/(float)hh;
		int hexcol, hexrow;
		if (maprenderer->setCenterMap(relX, relY, &hexcol, &hexrow)) {
			//			selectHex(hexcol, hexrow);
		}
	} else if (ev.type == tb::EVENT_TYPE_WHEEL) {
	} else if (ev.type == tb::EVENT_TYPE_KEY_DOWN) {
		unsigned int centerRow, centerCol, rowsDisplayed, colsDisplayed;
		switch(ev.special_key) {
			case tb::TB_KEY_LEFT:
				if (map->col > 0) map->col--;
				maprenderer->getMapDisplayCenter(centerRow, centerCol,
						rowsDisplayed, colsDisplayed);
				if (map->col < (centerCol - (colsDisplayed / 2))) {
					maprenderer->setMapDisplayCenter(centerRow, map->col);
				}
				break;
			case tb::TB_KEY_UP:
				maprenderer->moveUp();
				break;
			case tb::TB_KEY_RIGHT:
				if (map->col < (map->numCols()-1)) map->col++;
				maprenderer->getMapDisplayCenter(centerRow, centerCol,
						rowsDisplayed, colsDisplayed);
				if (map->col > (centerCol + (colsDisplayed / 2))) {
					maprenderer->setMapDisplayCenter(centerRow, map->col);
				}
				break;
			case tb::TB_KEY_DOWN:
				maprenderer->moveDown();
				break;
			case tb::TB_KEY_ESC:
				break;
			case tb::TB_KEY_ENTER:
				maprenderer->selectHex(map->col, map->row);
				break;
			case tb::TB_KEY_UNDEFINED: {
				switch(ev.key) {
					case '1':
						maprenderer->zoomOutMap();
						maprenderer->setMapDisplayCenter(map->row, map->col);
						break;
					case '2':
						maprenderer->zoomInMap();
						maprenderer->setMapDisplayCenter(map->row, map->col);
						break;
					default: return false;
				}
			}
			default: return false;
		}
		return true;
	} else if (ev.type == tb::EVENT_TYPE_KEY_UP) {
		// Consume the UP events for keys we used on down.
		switch(ev.special_key) {
			case tb::TB_KEY_LEFT:
			case tb::TB_KEY_UP:
			case tb::TB_KEY_RIGHT:
			case tb::TB_KEY_DOWN:
			case tb::TB_KEY_ESC:
			case tb::TB_KEY_ENTER:
				return true;
			default: break;
		}
	} else if (ev.type == tb::EVENT_TYPE_CLICK) {
		int mx, my, mw, mh;
		maprenderer->getMapPostion(&mx, &my, &mw, &mh);
		int hw = mw / 2;
		int hh = mh / 2;
		float relX = (float)((ev.target_x-mx)-hw)/(float)hw;
		float relY = -(float)((ev.target_y-my)-hh)/(float)hh;
		int hexcol, hexrow;
		if (maprenderer->setCenterMap(relX, relY, &hexcol, &hexrow)) {
			maprenderer->selectHex(hexcol, hexrow);
		}
		return true;
	}
	return false;
}

void MapWidget::OnResized(int old_w, int old_h) {
	tb::TBRect r = GetRect();
	maprenderer->resizeMap(r.w, r.h);
	GLWidget::OnResized(old_w, old_h);
}

void MapWidget::initGL() {
	//tb::TBRect r = GetRect();
	//maprenderer = std::make_unique<render::MapRenderer>(map, r.w, r.h, Settings::i()->getAssetDir());
}

void MapWidget::render() {
	maprenderer->renderMap(false);  // Main map.
}

void MapWidget::OnInflate(const tb::INFLATE_INFO &info) {
	GLWidget::OnInflate(info);
}

TB_WIDGET_FACTORY(MapWidget, tb::TBValue::TYPE_NULL, tb::WIDGET_Z_TOP) {}

}
}

