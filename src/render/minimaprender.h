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
#pragma once

#include "state/map.h"

#include <memory>
#include <string>

namespace hexgame { namespace render {

class MiniMapRenderer {
public:
	MiniMapRenderer(std::shared_ptr<Map> map, int width, int height, std::string assetDir);
	~MiniMapRenderer();

	void resizeMap(int width, int height);
	void renderMap();
	void miniMapDirty();
	void getMapDisplayCenter(unsigned int& centerRow, unsigned int& centerCol,
	                         unsigned int& rowsDisplayed, unsigned int& colsDisplayed);
	void setMapDisplayCenter(int displayCenterRow, int displayCenterCol);

	void getMiniMapPostion(int *x, int *y, int *width, int *height);
	bool setCenterMiniMap(float x, float y, int *hexcol, int *hexrow);
	bool setCenterMap(float x, float y, int *hexcol, int *hexrow);

private:
	struct CTX;
	std::unique_ptr<CTX> _ctx;
};

} }

