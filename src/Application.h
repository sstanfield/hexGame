#pragma once

#include "tb_widgets.h"
#include "gui/root_widget.h"
#include "state/map.h"

#include <functional>
#include <memory>

namespace hexgame {

class ServicesInterface {
public:
	virtual bool isFullscreen() = 0;
	virtual void toggleFullscreen() = 0;
	virtual int  numScreens() = 0;
	virtual void nextFullscreen() = 0;
	virtual void quit() = 0;
};

class Application {
public:
	Application(std::unique_ptr<ServicesInterface> services, int width, int height);
	virtual ~Application();

	void lostContext();
	void restoreContext(int width, int height);
	void processFrame();
	void setWindowSize(int width, int height);
	void setScaleFactors(double w, double h) { scaleWidth = w; scaleHeight = h; }

	bool InvokeKey(unsigned int key, tb::SPECIAL_KEY special_key,
                   tb::MODIFIER_KEYS modifierkeys, bool down);
	void buttonPress(unsigned long button,
	                 tb::MODIFIER_KEYS modifierkeys);
	void buttonRelease(unsigned long button,
	                   tb::MODIFIER_KEYS modifierkeys);
	void cursorPosition(double x, double y, tb::MODIFIER_KEYS modifierkeys);
	void scroll(double x, double y, tb::MODIFIER_KEYS modifierkeys);

	bool procGamePad();

	bool isFullscreen() { return services->isFullscreen(); }
	void toggleFullscreen() { services->toggleFullscreen(); }
	int  numScreens() { return services->numScreens(); }
	void nextFullscreen() { services->nextFullscreen(); }
	void quit() { services->quit(); }
	std::shared_ptr<Map> getMap() { return map; }

	tb::TBWidget *getRootWidget() { return root.get(); }

	static Application *instance();

private:
	std::unique_ptr<gui::RootWidget> root;
	std::unique_ptr<tb::TBRenderer> renderer;
	std::unique_ptr<ServicesInterface> services;
	std::shared_ptr<Map> map;
	tb::TBRect mainRect;
	double mouseX = 0;
	double mouseY = 0;
	double scaleWidth = 1;
	double scaleHeight = 1;
	bool glClose = false;
	bool simEnter = false;

	int  toupr_ascii(int ascii);
	bool InvokeShortcut(int key, tb::SPECIAL_KEY special_key,
                        tb::MODIFIER_KEYS modifierkeys, bool down);
};

}  // hexgame

