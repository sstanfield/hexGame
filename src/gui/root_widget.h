#pragma once

#include "tb_widgets.h"

namespace hexgame {
namespace gui {

class RootWidget : public tb::TBWidget {
public:
	TBOBJECT_SUBCLASS(RootWidget, tb::TBWidget);

	RootWidget();
	virtual ~RootWidget();

	virtual bool OnEvent(const tb::TBWidgetEvent &ev);

	virtual void OnChildAdded(TBWidget *child);
	virtual void OnChildRemove(TBWidget *child);

	bool isDimmed() { return dimmerCnt>0; }

private:
	int dimmerCnt = 0;
};

}
}

