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
#include "render/shaders.h"
#include "render/pc/GL/glew.h"
#include "render/imageutils.h"
#include "render/maprender.h"
#include "util/string.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <signal.h>
#include <thread>
#include <chrono>

static bool running = true;
static Map *map;
static MapRenderer maprenderer;
//static Unit *u1;

static uint turn = 1;

static char *assetDir;

static GLuint genBuffer(const GLfloat *buffer, int size) {
	GLuint bufferID;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &bufferID);

	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, bufferID);

	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
	return bufferID;
}

static void init(GLFWwindow *window) {
	printf("Initted, vender %s, version %s\n", glGetString(GL_VENDOR), glGetString(GL_VERSION));

//	glClearColor(.0, .0, .4, 1.0);
	glClearColor(.5, .5, .5, 1.0);
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	printf("XXX w: %d h: %d\n", width, height);
	maprenderer = initMapRender(map, width, height, assetDir);
	resizeMap(maprenderer, width, height);
}

static void render(GLFWwindow *window) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	selectUnit(map, u1);
	renderMap(maprenderer, FALSE);  // Main map.
	renderMap(maprenderer, TRUE);   // Mini map.

	/*if (map->selectedUnit) {
		float red[] = {1, 0, 0, 1};
		char str[100];
		snprintf(str, 199, "%d", map->tiles[map->row][map->col].currentMoveCost);
		float w, h;
	}*/

//	saveMiniMap(maprenderer);
//	saveMiniMap(maprenderer);
}

static double lastFpsTime = -1;
static void idle(GLFWwindow *window) {
	if (!running) {
		//loWshutdown(ctx);
		return;
	}
	double curtime = glfwGetTime();
	if (lastFpsTime == -1) lastFpsTime = curtime;
	double renderStart = glfwGetTime();
	render(window);
	double renderEnd = glfwGetTime();
	float renderTime = (float)( renderEnd - renderStart);

	float rFPS = 1.0f / renderTime;
	if (rFPS > 30) {
		float sleep = (1.0f / 30.0f) - renderTime;
		//std::this_thread::sleep_for(std::chrono::nanoseconds((long)(sleep * 1e9)));
	}

	float secondsPerFrame = (float)(curtime - lastFpsTime);

	if (secondsPerFrame > 0) {
		float FPS = 1.0f / secondsPerFrame;
		//printf("FPS: %f\n", FPS);
	}
	lastFpsTime           = curtime;
}

static void shutdown() {
	freeMapRenderer(maprenderer);
	maprenderer = nullptr;
	printf("Shutting down\n");
}

static void selectHex(int col, int row) {
	if (map->selectedUnit) {
		if (map->selectedUnit->row == row && map->selectedUnit->col == col) {
			// Select again so deselect.
			deSelectUnit(map);
		} else {  // Move here if it can.
			if (map->tiles[row][col].currentMoveCost <= map->selectedUnit->movement) {
				map->selectedUnit->movement -= map->tiles[row][col].currentMoveCost;
				moveUnit(map, map->selectedUnit, row, col);
//				selectUnit(map, map->selectedUnit);
				deSelectUnit(map);
			}
		}
	} else {  // If there is a unit to select then select it.
		if (map->tiles[row][col].numUnits) {
			selectUnit(map, map->tiles[row][col].units[0]);
		}
	}
}

static void moveUp() {
	int centerRow, centerCol, rowsDisplayed, colsDisplayed;
	if (map->row > 0) map->row--;
	getMapDisplayCenter(maprenderer, &centerRow, &centerCol,
	                         &rowsDisplayed, &colsDisplayed);
	if (map->row < (centerRow - (rowsDisplayed / 2))) {
		setMapDisplayCenter(maprenderer, map->row, centerCol);
	}
}

static void moveDown() {
	int centerRow, centerCol, rowsDisplayed, colsDisplayed;
	if (map->row < (map->numRows-1)) map->row++;
	getMapDisplayCenter(maprenderer, &centerRow, &centerCol,
	                         &rowsDisplayed, &colsDisplayed);
	if (map->row > (centerRow + (rowsDisplayed / 2))) {
		setMapDisplayCenter(maprenderer, map->row, centerCol);
	}
}

static void char_callback(GLFWwindow *window, unsigned int character)
{
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	//printf("XXX key_callback %c, %d, %d\n", key, key, scancode);
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		int centerRow, centerCol, rowsDisplayed, colsDisplayed;
		switch (key) {
			case GLFW_KEY_F11:
				//loWtoggleFullscreen(ctx);
				break;
			case GLFW_KEY_Q:
				if (mods & GLFW_MOD_CONTROL) running = false;
				break;
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

			case GLFW_KEY_ENTER:
				selectHex(map->col, map->row);
				break;

			case GLFW_KEY_1:
				zoomOutMap(maprenderer);
				setMapDisplayCenter(maprenderer, map->row, map->col);
				break;
			case GLFW_KEY_2:
				zoomInMap(maprenderer);
				setMapDisplayCenter(maprenderer, map->row, map->col);
				break;

			case GLFW_KEY_UP:
				moveUp();
				break;
			case GLFW_KEY_DOWN:
				moveDown();
				break;
			case GLFW_KEY_RIGHT:
				if (map->col < (map->numCols-1)) map->col++;
				getMapDisplayCenter(maprenderer, &centerRow, &centerCol,
						&rowsDisplayed, &colsDisplayed);
				if (map->col > (centerCol + (colsDisplayed / 2))) {
					setMapDisplayCenter(maprenderer, centerRow, map->col);
				}
				break;
			case GLFW_KEY_LEFT:
				if (map->col > 0) map->col--;
				getMapDisplayCenter(maprenderer, &centerRow, &centerCol,
						&rowsDisplayed, &colsDisplayed);
				if (map->col < (centerCol - (colsDisplayed / 2))) {
					setMapDisplayCenter(maprenderer, centerRow, map->col);
				}
				break;
		}
	}
	if (action == GLFW_RELEASE) {
	}
}

static bool b1Down = false;
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			b1Down = true;
			double winX, winY;
			glfwGetCursorPos(window, &winX, &winY);
			// XXX TODO are winX/Y correct?
			int mx, my, mw, mh;
			getMiniMapPostion(maprenderer, &mx, &my, &mw, &mh);
			if (winX >= mx && winX < (mx + mw) && winY >= my && winY < (my + mh)) {
				// Clicked in mini map.
				int hw = mw / 2;
				int hh = mh / 2;
				float relX = (float)((winX-mx)-hw)/(float)hw;
				float relY = -(float)((winY-my)-hh)/(float)hh;
				setCenterMiniMap(maprenderer, relX, relY, NULL, NULL);
			}
			getMapPostion(maprenderer, &mx, &my, &mw, &mh);
			if (winX >= mx && winX < (mx + mw) && winY >= my && winY < (my + mh)) {
				// Clicked in map.
				int hw = mw / 2;
				int hh = mh / 2;
				float relX = (float)((winX-mx)-hw)/(float)hw;
				float relY = -(float)((winY-my)-hh)/(float)hh;
				int hexcol, hexrow;
				if (setCenterMap(maprenderer, relX, relY, &hexcol, &hexrow)) {
					selectHex(hexcol, hexrow);
				}
			}
		}
		/*	if (button == LoW_ScrollDownMouse) {
			moveDown();
			}
			if (button == LoW_ScrollUpMouse) {
			moveUp();
			}*/
	}
	if (action == GLFW_RELEASE) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			b1Down = false;
		}
	}
}

static void cursor_position_callback(GLFWwindow *window, double x, double y)
{
	int mx, my, mw, mh;
	getMiniMapPostion(maprenderer, &mx, &my, &mw, &mh);
	if (x >= mx && x < (mx + mw) && y >= my && y < (my + mh)) {
		// In mini map.
		if (b1Down) {
			int hw = mw / 2;
			int hh = mh / 2;
			float relX = (float)((x-mx)-hw)/(float)hw;
			float relY = -(float)((y-my)-hh)/(float)hh;
			setCenterMiniMap(maprenderer, relX, relY, NULL, NULL);
		}
	}
	getMapPostion(maprenderer, &mx, &my, &mw, &mh);
	if (x >= mx && x < (mx + mw) && y >= my && y < (my + mh)) {
		// Clicked in map.
		int hw = mw / 2;
		int hh = mh / 2;
		float relX = (float)((x-mx)-hw)/(float)hw;
		float relY = -(float)((y-my)-hh)/(float)hh;
		int hexcol, hexrow;
		if (setCenterMap(maprenderer, relX, relY, &hexcol, &hexrow)) {
//			selectHex(hexcol, hexrow);
		}
	}
}

static void scroll_callback(GLFWwindow *window, double x, double y)
{
	//ctx->callbacks->scroll(ctx->glw, x, y);
}

static void refresh_callback(GLFWwindow* window) {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	resizeMap(maprenderer, width, height);
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
	while (running && !glfwWindowShouldClose(window))
	{
		idle(window);
		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

/*		if (!ctx->gamePadDef) {
			// No gamepad, check for one every now and then...
			if (ctx->gamePadSearchCount > 200) {
				ctx->gamePadSearchCount = 0;
				searchGamePad(ctx);
			} else ctx->gamePadSearchCount++;
		}*/
	}

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
	//_dpi = getCurrentDPI();
	return window;
}

static void handler(int sig) {
	running = false;
}

void parseCommandLine(int argc, char *argv[]) {
	int zlen = strlen(argv[0]);
	int i;
	for (i = zlen - 1; i > 0; i--) {
		if (argv[0][i] == '/') break;
	}
	if (i == 0) {  // In case it is run with no path...
		assetDir = (char *)malloc(sizeof(char) * 11);
		assetDir[0] = '.'; assetDir[1] = '.'; assetDir[2] = '/';
		assetDir[3]='a'; assetDir[4]='s'; assetDir[5]='s'; assetDir[6]='e';
		assetDir[7]='t'; assetDir[8]='s'; assetDir[9]='/';
		assetDir[10] = 0;
	} else {
		assetDir = (char *)malloc(sizeof(char) * (i + 12));
		memcpy(assetDir, argv[0], i+1);
		assetDir[i+1]='.'; assetDir[i+2]='.'; assetDir[i+3]='/';
		assetDir[i+4]='a'; assetDir[i+5]='s'; assetDir[i+6]='s'; assetDir[i+7]='e';
		assetDir[i+8]='t'; assetDir[i+9]='s'; assetDir[i+10]='/';
		assetDir[i+11] = 0;
	}
}

int main(int argc, char *argv[]) {
	parseCommandLine(argc, argv);
//	map = readMap("../test.map");//allocMap(100, 100);
	map = allocMap(100, 100, Grass);
//	map = readMap("./new.map");
	int u1move = aquireMoveCost(map->unitCtx, 2, 4, 5, 3, 4, 0, 0, 0, 1, 2, 1);
	Unit *u1 = newUnit(map->unitCtx, Side_Black, "Test Cav", 3, 5, 20, u1move);
	placeUnit(map, u1, 50, 50);
//	printf("XXXX 1\n");
//	selectUnit(map, u1);
//	printf("XXXX 2\n");
	signal(SIGINT, handler);
	signal(SIGTERM, handler);
	GLFWwindow *window = nullptr;
	if ((window = openWindow(1024, 768, false, "HEX Game!"))) {
		loop(window);
	}
	freeMap(map);
	free(assetDir);
	return 0;
}
