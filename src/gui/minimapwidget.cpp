#include "Application.h"
#include "minimapwidget.h"
#include "settings.h"
#include "tb_widgets_reader.h"

namespace hexgame {
namespace gui {

MiniMapWidget::MiniMapWidget() {
	map = Application::instance()->getMap();
	tb::TBRect r = GetRect();
	minimaprenderer = std::make_unique<render::MiniMapRenderer>(map, r.w, r.h, Settings::i()->getShaderDir());
}

MiniMapWidget::~MiniMapWidget() {
}

bool MiniMapWidget::OnEvent(const tb::TBWidgetEvent &ev)
{
	if (ev.type == tb::EVENT_TYPE_CUSTOM && ev.ref_id == tb::TBID("gamepad")) {
		//return procGamePad();
	}
	if (ev.type == tb::EVENT_TYPE_POINTER_MOVE) {
	} else if (ev.type == tb::EVENT_TYPE_WHEEL) {
	} else if (ev.type == tb::EVENT_TYPE_KEY_DOWN) {
		switch(ev.special_key) {
			case tb::TB_KEY_LEFT:
				break;
			case tb::TB_KEY_UP:
				break;
			case tb::TB_KEY_RIGHT:
				break;
			case tb::TB_KEY_DOWN:
				break;
			case tb::TB_KEY_ESC:
				break;
			case tb::TB_KEY_ENTER:
				break;
			case tb::TB_KEY_UNDEFINED: {
				switch(ev.key) {
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
		return true;
	}
	return false;
}

void MiniMapWidget::OnResized(int old_w, int old_h) {
	tb::TBRect r = GetRect();
	minimaprenderer->resizeMap(r.w, r.h);
	GLWidget::OnResized(old_w, old_h);
}

void MiniMapWidget::initGL() {
	//tb::TBRect r = GetRect();
	//maprenderer = std::make_unique<render::MapRenderer>(map, r.w, r.h, Settings::i()->getAssetDir());
}

void MiniMapWidget::render() {
	minimaprenderer->renderMap();  // Main map.
}

void MiniMapWidget::OnInflate(const tb::INFLATE_INFO &info) {
	GLWidget::OnInflate(info);
}

TB_WIDGET_FACTORY(MiniMapWidget, tb::TBValue::TYPE_NULL, tb::WIDGET_Z_TOP) {}

}
}

