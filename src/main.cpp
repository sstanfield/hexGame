/*
Copyright (c) 2015-2016 Steven Stanfield

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would
   be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not
   be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
   distribution.
*/
#include "Application.h"
#include "render/shaders.h"
#include "render/gl_util.h"
#include "render/imageutils.h"
#include "render/maprender.h"
#include "util/string.h"
#ifdef __EMSCRIPTEN__
#define GLFW_INCLUDE_ES2
#endif
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#endif
#include "GLFW/glfw3.h"
#include "tb_widgets.h"
#include "settings.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <csignal>
#include <thread>
#include <chrono>

static bool running = true;

//static unsigned int turn = 1;

using namespace hexgame;

Application *app = nullptr;

class Services : public ServicesInterface {
public:
	bool isFullscreen() override
	{
		return false;
	}

	void toggleFullscreen() override
	{
	}

	int  numScreens() override
	{
		return 1;
	}

	void nextFullscreen() override
	{
	}

	void quit() override
	{
		running = false;
	}
};

static void getScaleFactors(GLFWwindow *window, double *scaleWidth, double *scaleHeight) {
	int fwidth = 1, fheight = 1, swidth = 1, sheight = 1;
	glfwGetFramebufferSize(window, &fwidth, &fheight);
	glfwGetWindowSize(window, &swidth, &sheight);
	*scaleWidth = (double)fwidth / (double)swidth;
	*scaleHeight = (double)fheight / (double)sheight;
}

static void init(GLFWwindow *window) {
	printf("Initted, vender %s, version %s\n", glGetString(GL_VENDOR), glGetString(GL_VERSION));

//	glClearColor(.0, .0, .4, 1.0);
	glClearColor(.5, .5, .5, 1.0);
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static double lastFpsTime = -1;
static void idle() {
	if (!running) {
#ifdef __EMSCRIPTEN__
		emscripten_cancel_main_loop();
#endif
		return;
	}
	double curtime = glfwGetTime();
	if (lastFpsTime == -1) lastFpsTime = curtime;
	//double renderStart = glfwGetTime();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	app->processFrame();
	//double renderEnd = glfwGetTime();
	//float renderTime = (float)( renderEnd - renderStart);

	//float rFPS = 1.0f / renderTime;
	/*if (rFPS > 30) {
		float sleep = (1.0f / 30.0f) - renderTime;
		//std::this_thread::sleep_for(std::chrono::nanoseconds((long)(sleep * 1e9)));
	}*/

	//float secondsPerFrame = (float)(curtime - lastFpsTime);

	/*if (secondsPerFrame > 0) {
		float FPS = 1.0f / secondsPerFrame;
		//printf("FPS: %f\n", FPS);
	}*/
	lastFpsTime           = curtime;

	/* Poll for and process events */
	glfwPollEvents();

}

static void shutdown() {
	printf("Shutting down\n");
}

static bool key_alt = false;
static bool key_ctrl = false;
static bool key_shift = false;
static bool key_super = false;

static tb::MODIFIER_KEYS GetModifierKeys() {
	tb::MODIFIER_KEYS code = tb::TB_MODIFIER_NONE;
	if (key_alt)	code |= tb::TB_ALT;
	if (key_ctrl)	code |= tb::TB_CTRL;
	if (key_shift)	code |= tb::TB_SHIFT;
	if (key_super)	code |= tb::TB_SUPER;
	return code;
}

static tb::MODIFIER_KEYS GetModifierKeys(int glfwmod) {
	tb::MODIFIER_KEYS code = tb::TB_MODIFIER_NONE;
	// Re-sync key flags, going fullscreen can cause these to be missed.
	key_alt = key_ctrl = key_shift = key_super = false;
	if (glfwmod & GLFW_MOD_ALT)		{ code |= tb::TB_ALT; key_alt = true; }
	if (glfwmod & GLFW_MOD_CONTROL)	{ code |= tb::TB_CTRL; key_ctrl = true; }
	if (glfwmod & GLFW_MOD_SHIFT)	{ code |= tb::TB_SHIFT; key_shift = true; }
	if (glfwmod & GLFW_MOD_SUPER)	{ code |= tb::TB_SUPER; key_super = true; }
	return code;
}

static bool InvokeKey(unsigned int key, tb::SPECIAL_KEY special_key,
                   tb::MODIFIER_KEYS modifierkeys, bool down) {
	return app->InvokeKey(key, special_key, modifierkeys, down);
}

static bool doKey(int key, unsigned long metaKeys, bool down) {
	bool ret = false;
	tb::MODIFIER_KEYS modifier = GetModifierKeys(metaKeys);
	switch (key)
	{
		case GLFW_KEY_F1:		ret = InvokeKey(0, tb::TB_KEY_F1, modifier, down); break;
		case GLFW_KEY_F2:		ret = InvokeKey(0, tb::TB_KEY_F2, modifier, down); break;
		case GLFW_KEY_F3:		ret = InvokeKey(0, tb::TB_KEY_F3, modifier, down); break;
		case GLFW_KEY_F4:		ret = InvokeKey(0, tb::TB_KEY_F4, modifier, down); break;
		case GLFW_KEY_F5:		ret = InvokeKey(0, tb::TB_KEY_F5, modifier, down); break;
		case GLFW_KEY_F6:		ret = InvokeKey(0, tb::TB_KEY_F6, modifier, down); break;
		case GLFW_KEY_F7:		ret = InvokeKey(0, tb::TB_KEY_F7, modifier, down); break;
		case GLFW_KEY_F8:		ret = InvokeKey(0, tb::TB_KEY_F8, modifier, down); break;
		case GLFW_KEY_F9:		ret = InvokeKey(0, tb::TB_KEY_F9, modifier, down); break;
		case GLFW_KEY_F10:		ret = InvokeKey(0, tb::TB_KEY_F10, modifier, down); break;
		case GLFW_KEY_F11:		ret = InvokeKey(0, tb::TB_KEY_F11, modifier, down); break;
		case GLFW_KEY_F12:		ret = InvokeKey(0, tb::TB_KEY_F12, modifier, down); break;
		case GLFW_KEY_LEFT:		ret = InvokeKey(0, tb::TB_KEY_LEFT, modifier, down); break;
		case GLFW_KEY_UP:		ret = InvokeKey(0, tb::TB_KEY_UP, modifier, down); break;
		case GLFW_KEY_RIGHT:		ret = InvokeKey(0, tb::TB_KEY_RIGHT, modifier, down); break;
		case GLFW_KEY_DOWN:		ret = InvokeKey(0, tb::TB_KEY_DOWN, modifier, down); break;
		case GLFW_KEY_PAGE_UP:	ret = InvokeKey(0, tb::TB_KEY_PAGE_UP, modifier, down); break;
		case GLFW_KEY_PAGE_DOWN:	ret = InvokeKey(0, tb::TB_KEY_PAGE_DOWN, modifier, down); break;
		case GLFW_KEY_HOME:		ret = InvokeKey(0, tb::TB_KEY_HOME, modifier, down); break;
		case GLFW_KEY_END:		ret = InvokeKey(0, tb::TB_KEY_END, modifier, down); break;
		case GLFW_KEY_INSERT:	ret = InvokeKey(0, tb::TB_KEY_INSERT, modifier, down); break;
		case GLFW_KEY_TAB:		ret = InvokeKey(0, tb::TB_KEY_TAB, modifier, down); break;
		case GLFW_KEY_DELETE:	ret = InvokeKey(0, tb::TB_KEY_DELETE, modifier, down); break;
		case GLFW_KEY_BACKSPACE:	ret = InvokeKey(0, tb::TB_KEY_BACKSPACE, modifier, down); break;
		case GLFW_KEY_ENTER:		
		case GLFW_KEY_KP_ENTER:	ret = InvokeKey(0, tb::TB_KEY_ENTER, modifier, down); break;
		case GLFW_KEY_ESCAPE:		
			ret = InvokeKey(0, tb::TB_KEY_ESC, modifier, down);
			break;
		case GLFW_KEY_MENU:
			if (!down) {
				tb::TBWidgetEvent ev(tb::EVENT_TYPE_CONTEXT_MENU);
				ev.modifierkeys = modifier;
				if (tb::TBWidget::focused_widget) tb::TBWidget::focused_widget->InvokeEvent(ev);
				else app->getRootWidget()->InvokeEvent(ev);
			}
			break;
		case GLFW_KEY_LEFT_SHIFT:
		case GLFW_KEY_RIGHT_SHIFT:
			key_shift = down;
			break;
		case GLFW_KEY_LEFT_CONTROL:
		case GLFW_KEY_RIGHT_CONTROL:
			key_ctrl = down;
			break;
		case GLFW_KEY_LEFT_ALT:
		case GLFW_KEY_RIGHT_ALT:
			key_alt = down;
			break;
		case GLFW_KEY_LEFT_SUPER:
		case GLFW_KEY_RIGHT_SUPER:
			key_super = down;
			break;
		default:
			// glfw calls key_callback instead of char_callback
			// when pressing a character while ctrl or alt is also pressed.
			if ((key_ctrl || key_alt) && key >= 32 && key <= 255) {
				ret = InvokeKey(key, tb::TB_KEY_UNDEFINED, modifier, down);
			}
			break;
	}
	return ret;
}

static void char_callback(GLFWwindow *window, unsigned int character)
{
	InvokeKey(character, tb::TB_KEY_UNDEFINED, GetModifierKeys(), true);
	InvokeKey(character, tb::TB_KEY_UNDEFINED, GetModifierKeys(), false);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//printf("XXX key_callback %c, %d, %d\n", key, key, scancode);
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		doKey(key, mods, true);
		/*int centerRow, centerCol, rowsDisplayed, colsDisplayed;
		switch (key) {
			case GLFW_KEY_G:
				map->tiles[map->row][map->col].type = Grass;
				if (map->selectedUnit) selectUnit(map, map->selectedUnit);
				break;
			case GLFW_KEY_F:
				map->tiles[map->row][map->col].type = Forest;
				if (map->selectedUnit) selectUnit(map, map->selectedUnit);
				break;
			case GLFW_KEY_W:
				map->tiles[map->row][map->col].type = Water;
				if (map->selectedUnit) selectUnit(map, map->selectedUnit);
				break;
			case GLFW_KEY_H:
				map->tiles[map->row][map->col].type = Hill;
				if (map->selectedUnit) selectUnit(map, map->selectedUnit);
				break;
			case GLFW_KEY_M:
				miniMapDirty(maprenderer);
				map->tiles[map->row][map->col].type = Mountain;
				if (map->selectedUnit) selectUnit(map, map->selectedUnit);
				break;
			case GLFW_KEY_S:
				if (mods & GLFW_MOD_CONTROL) writeMap(map, "new.map");
				else {
					map->tiles[map->row][map->col].type = Swamp;
					if (map->selectedUnit) selectUnit(map, map->selectedUnit);
				}
				break;
			case GLFW_KEY_C:
				addCity(map, "Some City", map->row, map->col, 5);
				if (map->selectedUnit) selectUnit(map, map->selectedUnit);
				break;

			case GLFW_KEY_T:
				if (mods & GLFW_MOD_CONTROL) {
					resetMovePoints(map->unitCtx, Side_Black);
					turn++;
				}
				break;


		}*/
	}
	if (action == GLFW_RELEASE) {
		doKey(key, mods, false);
	}
}

//static bool b1Down = false;
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			app->buttonPress(button, GetModifierKeys(mods));
			/*b1Down = true;
			double winX, winY;
			glfwGetCursorPos(window, &winX, &winY);
			// XXX TODO are winX/Y correct?
			int mx, my, mw, mh;
			//getMiniMapPostion(maprenderer, &mx, &my, &mw, &mh);
			if (winX >= mx && winX < (mx + mw) && winY >= my && winY < (my + mh)) {
				// Clicked in mini map.
				int hw = mw / 2;
				int hh = mh / 2;
				float relX = (float)((winX-mx)-hw)/(float)hw;
				float relY = -(float)((winY-my)-hh)/(float)hh;
				//setCenterMiniMap(maprenderer, relX, relY, NULL, NULL);
			}*/
		}
	}
	if (action == GLFW_RELEASE) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			app->buttonRelease(button, GetModifierKeys(mods));
			//b1Down = false;
		}
	}
}

static void cursor_position_callback(GLFWwindow *window, double x, double y)
{
	app->cursorPosition(x, y, GetModifierKeys());
	/*int mx, my, mw, mh;
	//getMiniMapPostion(maprenderer, &mx, &my, &mw, &mh);
	if (x >= mx && x < (mx + mw) && y >= my && y < (my + mh)) {
		// In mini map.
		if (b1Down) {
			int hw = mw / 2;
			int hh = mh / 2;
			float relX = (float)((x-mx)-hw)/(float)hw;
			float relY = -(float)((y-my)-hh)/(float)hh;
			//setCenterMiniMap(maprenderer, relX, relY, NULL, NULL);
		}
	}*/
}

static void scroll_callback(GLFWwindow *window, double x, double y)
{
	app->scroll(x, y, GetModifierKeys());
}

static void refresh_callback(GLFWwindow* window) {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	//resizeMap(maprenderer, width, height);
	double sw, sx;
	app->setWindowSize(width, height);
	getScaleFactors(window, &sw, &sx);
	app->setScaleFactors(sw, sx);
	//setWindowWH(width, height);
}

static void window_pos_callback(GLFWwindow* window, int xpos, int ypos)
{
	//_dpi = ctx->glw->getCurrentDPI();
	// XXX TODO If DPI changed should update skin DPI...
}

static void loop(GLFWwindow *window) {
	glfwSwapInterval(1);
	init(window);
	glfwSetCharCallback(window, char_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetWindowRefreshCallback(window, refresh_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetWindowPosCallback(window, window_pos_callback);
	running = true;
	//if (!ctx->gamePadDef) searchGamePad(ctx);
	//ctx->gamePadSearchCount = 0;
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(idle, 0, true);
#else
	while (running && !glfwWindowShouldClose(window))
	{
		idle();
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

/*		if (!ctx->gamePadDef) {
			// No gamepad, check for one every now and then...
			if (ctx->gamePadSearchCount > 200) {
				ctx->gamePadSearchCount = 0;
				searchGamePad(ctx);
			} else ctx->gamePadSearchCount++;
		}*/
	}
#endif
	/*if (ctx->reset) {
		_setFullscreen();
		ctx->reset = false;
		return false;
	}*/
	shutdown();
	glfwTerminate();
}

static GLFWwindow *openWindow(int width, int height,
                       bool startFullscreen, std::string title) {
	GLFWwindow *window = nullptr;
	/* Initialize the library */
	if (!glfwInit()) {
		return nullptr;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	/* Create a windowed mode window and its OpenGL context */
	if (startFullscreen) {
		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		if (!monitor) {
			glfwTerminate();
			return nullptr;
		}
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		window = glfwCreateWindow(mode->width, mode->height,
		        title.c_str(), monitor, nullptr);
	} else {
		window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	}
	if (!window) {
		glfwTerminate();
		return nullptr;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

#ifndef __EMSCRIPTEN__
#ifndef _P_OSX
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		printf("GLEW Error: %s\n", glewGetErrorString(err));
		glfwTerminate();
		return nullptr;
	}
	// clear glewinit errors....
	// XXX TODO, why does this happen?
	GLenum err2 = GL_NO_ERROR;
	while((err2 = glGetError()) != GL_NO_ERROR) {
		//std::cout << "GL Error init 1- "<<std::hex<<err2<<"\n";
	}
	printf("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif // _P_OSX
#endif // __EMSCRIPTEN__
	//_dpi = getCurrentDPI();
	return window;
}

static void handler(int sig) {
	running = false;
}

void parseCommandLine(int argc, char *argv[]) {
#ifdef __EMSCRIPTEN__
	Settings::i()->setAssetDir("/assets/");
	Settings::i()->setShaderDir("/assets/shaders/es_2.0/");
#else
	int zlen = strlen(argv[0]);
	int i;
	for (i = zlen - 1; i > 0; i--) {
		if (argv[0][i] == '/') break;
	}
	std::string baseDir;
	if (i == 0) {  // In case it is run with no path...
		baseDir = "../";
	} else {
		baseDir = argv[0];
		baseDir.resize(i+1);
		baseDir += "../";
	}
#ifdef _P_POSIX
	char *fullpath = realpath(baseDir.c_str(), nullptr);
	if (fullpath) {
		baseDir = fullpath+std::string("/");
		free(fullpath);
	}
#endif
	std::string assetDir = baseDir+"assets/";
	Settings::i()->setAssetDir(assetDir);
	Settings::i()->setShaderDir(assetDir+"shaders/330_core/");
#endif
}

int main(int argc, char *argv[]) {
	parseCommandLine(argc, argv);
	signal(SIGINT, handler);
	signal(SIGTERM, handler);
	GLFWwindow *window = nullptr;
	if ((window = openWindow(1024, 768, false, "HEX Game!"))) {
		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		Application a(std::make_unique<Services>(), w, h);
		app = &a;
		double sw, sx;
		getScaleFactors(window, &sw, &sx);
		app->setScaleFactors(sw, sx);

		loop(window);

		app = nullptr;
	}
	return 0;
}
