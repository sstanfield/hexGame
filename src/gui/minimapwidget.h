#pragma once

#include "glwidget.h"
#include "render/minimaprender.h"

#include <memory>

namespace hexgame {
namespace gui {

/**
 * Display and manage the mini-map.
 */
class MiniMapWidget : public GLWidget {
public:
	TBOBJECT_SUBCLASS(MiniMapWidget, GLWidget);

	MiniMapWidget();
	~MiniMapWidget() override;

	bool OnEvent(const tb::TBWidgetEvent &ev) override;
	void OnResized(int old_w, int old_h) override;
	void OnInflate(const tb::INFLATE_INFO &info) override;

protected:
	void initGL() override;
	void render() override;

private:
	std::shared_ptr<Map> map;
	std::unique_ptr<render::MiniMapRenderer> minimaprenderer;
};

}
}
