#pragma once

#include "tb_widgets.h"

namespace hexgame {
namespace gui {

class RootWidget : public tb::TBWidget {
public:
	TBOBJECT_SUBCLASS(RootWidget, tb::TBWidget);

	RootWidget();
	~RootWidget() override = default;

	bool OnEvent(const tb::TBWidgetEvent &ev) override;

	void OnChildAdded(TBWidget *child) override;
	void OnChildRemove(TBWidget *child) override;

	bool isDimmed() { return dimmerCnt>0; }

private:
	int dimmerCnt = 0;
};

}
}

