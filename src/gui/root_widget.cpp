#include "root_widget.h"
#include "Application.h"

#include "tb_widgets_common.h"

namespace hexgame {
namespace gui {

RootWidget::RootWidget()
{
	SetID(tb::TBID("RootWindow"));
}

bool RootWidget::OnEvent(const tb::TBWidgetEvent &ev) {
	bool ret = false;
	Application *app = Application::instance();
	if (ev.type == tb::EVENT_TYPE_CUSTOM && ev.ref_id == tb::TBID("gamepad")) {
	}
	if (ev.type == tb::EVENT_TYPE_CONTEXT_MENU) {
	}
	if (ev.type == tb::EVENT_TYPE_KEY_DOWN) {
		switch(ev.special_key) {
			case tb::TB_KEY_F11:
				app->toggleFullscreen();
				ret = true;
				break;
			default: break;
		}
		switch(ev.key) {
			case 'f':
			case 'F':
				if (ev.modifierkeys & tb::TB_ALT)
					if (app->isFullscreen()) app->nextFullscreen();
				ret = true;
				break;
			case 'q':
			case 'Q':
				if (ev.modifierkeys & tb::TB_CTRL) app->quit();
				ret = true;
				break;
		}
	}
	return ret;
}

void RootWidget::OnChildAdded(tb::TBWidget *child) {
	if (child->IsOfType<tb::TBDimmer>()) dimmerCnt++;
}

void RootWidget::OnChildRemove(tb::TBWidget *child) {
	if (child->IsOfType<tb::TBDimmer>() && dimmerCnt>0) dimmerCnt--;
}

}
}

