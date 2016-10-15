#pragma once

#include "tb_widgets.h"
#include "gui/root_widget.h"

#include <functional>

namespace hexgame {

class Application {
public:
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

	void setIsFullscreen(std::function<bool ()> callback) {
		m_isFullscreen = callback;
	}
	void setToggleFullscreen(std::function<void ()> callback) {
		m_toggleFullscreen = callback;
	}
	void setNumScreens(std::function<int ()> callback) {
		m_numScreens = callback;
	}
	void setNextFullscreen(std::function<void ()> callback) {
		m_nextFullscreen = callback;
	}
	void setQuit(std::function<void ()> callback) {
		m_quit = callback;
	}

	bool isFullscreen() { return m_isFullscreen?m_isFullscreen():false; }
	void toggleFullscreen() { if (m_toggleFullscreen) m_toggleFullscreen(); }
	int  numScreens() { return m_numScreens?m_numScreens():1; }
	void nextFullscreen() { if (m_nextFullscreen) m_nextFullscreen(); }
	void quit() { if (m_quit) m_quit(); }

	tb::TBWidget *getRootWidget() { return root; }

	static Application *instance();
	static Application *newInstance(int width, int height);

private:
	Application() { };

	gui::RootWidget *root;
	tb::TBWidget *mainWidget = nullptr;
	tb::TBRect mainRect;
	tb::TBRenderer *renderer = nullptr;
	double mouseX = 0;
	double mouseY = 0;
	double scaleWidth = 1;
	double scaleHeight = 1;
	bool glClose = false;
	bool simEnter = false;

	std::function<bool ()> m_isFullscreen = nullptr;
	std::function<void ()> m_toggleFullscreen = nullptr;
	std::function<int  ()> m_numScreens = nullptr;
	std::function<void ()> m_nextFullscreen = nullptr;
	std::function<void ()> m_quit = nullptr;

	int  toupr_ascii(int ascii);
	bool InvokeShortcut(int key, tb::SPECIAL_KEY special_key,
                        tb::MODIFIER_KEYS modifierkeys, bool down);
};

}  // hexgame

