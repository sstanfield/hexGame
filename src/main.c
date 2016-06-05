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
#include "loWcore.h"
#include "render/shaders.h"
#include "render/pc/GL/glew.h"
#include "render/imageutils.h"
#include "render/maprender.h"
#include "render/rendertext.h"
#include "util/string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/keysym.h>
#include <signal.h>

static loWBool running = loW_true;
static Map *map;
static MapRenderer *maprenderer;
static FontContext font1;
static FontContext font2;
static FontContext font3;
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

static void init(loWContext ctx) {
	printf("Initted...\n");

//	glClearColor(.0, .0, .4, 1.0);
	glClearColor(.5, .5, .5, 1.0);
	// Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int width, height;
	loWgetWidthHeight(ctx, &width, &height);
	maprenderer = initMapRender(map, width, height, assetDir);
	char strBuf[512];
	font1 = renderTextInit(strAdd(strBuf, 512, assetDir, "fonts/DroidSerif-BoldItalic.ttf"), 56/*112*/, assetDir);
//	font2 = renderTextInit(strAdd(strBuf, 512, assetDir, "fonts/DroidSansMono.ttf"), 128, assetDir);
//	font3 = renderTextInit(strAdd(strBuf, 512, assetDir, "fonts/DroidSans.ttf"), 128, assetDir);
}

static void expose(loWContext ctx, int width, int height) {
	resizeMap(maprenderer, width, height);
	setWindowWH(width, height);
}

static void render(loWContext ctx) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	selectUnit(map, u1);
	renderMap(maprenderer, FALSE);  // Main map.
	renderMap(maprenderer, TRUE);   // Mini map.

//	float black[] = {0, 0, 0, 1};
	if (map->selectedUnit) {
		float red[] = {1, 0, 0, 1};
		char str[100];
		snprintf(str, 199, "%d", map->tiles[map->row][map->col].currentMoveCost);
		float w, h;
		renderTextSize(font1, str, .1, &w, &h);
		renderText(font1, str, 0 - (w / 2.0f), -1.0 + (h), .1, red);
	}
	//	char *str = "SLS (sls) [I i] `was_ here ghysj";
////	char *str = "TESTING, 1, 2, 3, TESTING AJGSB";
//	float w, h;

//	renderTextSize(font1, str, .1, &w, &h);
//	renderText(font1, str, 0 - (w / 2.0f), .5, .1, red);
//	renderText(font2, str, -.9, .1, .1, red);
//	renderText(font2, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", -.9, 0, .1, red);
////	red[3] = .5;
//	renderText(font2, "abcdefghijklmnopqrstuvwxyz", -.9, -.1, .1, red);
//	renderTextSize(font3, str, .1, &w, &h);
//	renderText(font3, str, 0 - (w / 2.0f), -.5, .1, red);

//	saveMiniMap(maprenderer);
	loWswapBuffers(ctx);
//	saveMiniMap(maprenderer);
}

static double lastFpsTime = -1;
static void idle(loWContext ctx) {
	if (!running) {
		loWshutdown(ctx);
		return;
	}
	double curtime = loWgetSeconds();
	if (lastFpsTime == -1) lastFpsTime = curtime;
	double renderStart = loWgetSeconds();
	render(ctx);
	double renderEnd = loWgetSeconds();
	float renderTime = (float)( renderEnd - renderStart);

	float rFPS = 1.0f / renderTime;
	if (rFPS > 30) {
		float sleep = (1.0f / 30.0f) - renderTime;
		loWnanoSleep(sleep * 1e9);
	}

	float secondsPerFrame = (float)(curtime - lastFpsTime);

	if (secondsPerFrame > 0) {
		float FPS = 1.0f / secondsPerFrame;
//		printf("FPS: %f\n", FPS);
	}
	lastFpsTime           = curtime;
}

static void shutdown(loWContext ctx) {
	freeMapRenderer(maprenderer);
	maprenderer = NULL;
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

static void keyPress(loWContext ctx, unsigned long keysym, unsigned long metaKeys) {
//	printf("mk: %x, %x, %d\n", metaKeys, loW_Control, (metaKeys & loW_Control));
	int centerRow, centerCol, rowsDisplayed, colsDisplayed;
	switch (keysym) {
	case XK_F11:
		loWtoggleFullscreen(ctx);
		break;
	case XK_q:
	case XK_Q:
		if (metaKeys & loW_Control) loWshutdown(ctx);
		break;
	case XK_g:
		map->tiles[map->row][map->col].type = Grass;
		if (map->selectedUnit) selectUnit(map, map->selectedUnit);
		break;
	case XK_f:
		map->tiles[map->row][map->col].type = Forest;
		if (map->selectedUnit) selectUnit(map, map->selectedUnit);
		break;
	case XK_w:
		map->tiles[map->row][map->col].type = Water;
		if (map->selectedUnit) selectUnit(map, map->selectedUnit);
		break;
	case XK_h:
		map->tiles[map->row][map->col].type = Hill;
		if (map->selectedUnit) selectUnit(map, map->selectedUnit);
		break;
	case XK_m:
		miniMapDirty(maprenderer);
		map->tiles[map->row][map->col].type = Mountain;
		if (map->selectedUnit) selectUnit(map, map->selectedUnit);
		break;
	case XK_s:
		if (metaKeys & loW_Control) writeMap(map, "new.map");
		else {
			map->tiles[map->row][map->col].type = Swamp;
			if (map->selectedUnit) selectUnit(map, map->selectedUnit);
		}
		break;
	case XK_c:
		addCity(map, "Some City", map->row, map->col, 5);
		if (map->selectedUnit) selectUnit(map, map->selectedUnit);
		break;

	case XK_t:
	case XK_T:
		if (metaKeys & loW_Control) {
			resetMovePoints(map->unitCtx, Side_Black);
			turn++;
		}
		break;

	case XK_Return:
		selectHex(map->col, map->row);
		break;

	case XK_1:
		zoomOutMap(maprenderer);
		setMapDisplayCenter(maprenderer, map->row, map->col);
		break;
	case XK_2:
		zoomInMap(maprenderer);
		setMapDisplayCenter(maprenderer, map->row, map->col);
		break;

	case XK_Up:
		moveUp();
		break;
	case XK_Down:
		moveDown();
		break;
	case XK_Right:
		if (map->col < (map->numCols-1)) map->col++;
		getMapDisplayCenter(maprenderer, &centerRow, &centerCol,
		                         &rowsDisplayed, &colsDisplayed);
		if (map->col > (centerCol + (colsDisplayed / 2))) {
			setMapDisplayCenter(maprenderer, centerRow, map->col);
		}
		break;
	case XK_Left:
		if (map->col > 0) map->col--;
		getMapDisplayCenter(maprenderer, &centerRow, &centerCol,
		                         &rowsDisplayed, &colsDisplayed);
		if (map->col < (centerCol - (colsDisplayed / 2))) {
			setMapDisplayCenter(maprenderer, centerRow, map->col);
		}
		break;
	}
}

static void keyRelease(loWContext ctx, unsigned long keysym, unsigned long metaKeys) {
}

static Bool b1Down = FALSE;
void buttonPress(loWContext ctx, unsigned long button, int winX, int winY,
                 unsigned long metaKeys) {
//	printf("Button press: %d (%f, %f)\n", button, glX, glY);
	if (button == LoW_LeftMouse) {
		b1Down = TRUE;
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
//		printf("Big map (%d, %d) %dx%d\n", mx, my, mw, mh);
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
	if (button == LoW_ScrollDownMouse) {
		moveDown();
	}
	if (button == LoW_ScrollUpMouse) {
		moveUp();
	}
}

void buttonRelease(loWContext ctx, unsigned long button, int winX, int winY,
                   unsigned long metaKeys) {
//	printf("Button release: %d (%d, %d) (%f, %f)\n", button, winX, winY, glX, glY);
	if (button == LoW_LeftMouse) {
		b1Down = FALSE;
	}
}

void mouseMove(loWContext ctx, int winX, int winY, unsigned long metaKeys) {
//	printf("Mouse move (%f, %f)\n", x, y);
	int mx, my, mw, mh;
	getMiniMapPostion(maprenderer, &mx, &my, &mw, &mh);
	if (winX >= mx && winX < (mx + mw) && winY >= my && winY < (my + mh)) {
		// In mini map.
		if (b1Down) {
			int hw = mw / 2;
			int hh = mh / 2;
			float relX = (float)((winX-mx)-hw)/(float)hw;
			float relY = -(float)((winY-my)-hh)/(float)hh;
			setCenterMiniMap(maprenderer, relX, relY, NULL, NULL);
		}
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
//			selectHex(hexcol, hexrow);
		}
	}
}


static void handler(int sig) {
	running = loW_false;
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
	loWCallbacks c;
	c.init = init;
	c.expose = expose;
	c.shutdown = shutdown;
	c.idle = idle;
	c.keyPress = keyPress;
	c.keyRelease = keyRelease;
	c.buttonPress = buttonPress;
	c.buttonRelease = buttonRelease;
	c.mouseMove = mouseMove;
	loWContext ctx = loWinit(&c, 1024, 768, loW_false);
	loWsetWindowTitle(ctx, "SLS made this window!");
	loWloop(ctx);
	freeMap(map);
	free(assetDir);
	return 0;
}
