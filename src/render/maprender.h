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
#ifndef MAPRENDER_H
#define MAPRENDER_H

#include "../state/map.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *MapRenderer;

MapRenderer initMapRender(Map *map, int width, int height, char *assetDir);
void resizeMap(MapRenderer mapctx, int width, int height);
void renderMap(MapRenderer mapctx, Bool mini);
void miniMapDirty(MapRenderer mapctx);
void getMapDisplayCenter(MapRenderer mapctx, int *centerRow, int *centerCol,
                         int *rowsDisplayed, int *colsDisplayed);
void setMapDisplayCenter(MapRenderer mapctx, int displayCenterRow,
                         int displayCenterCol);
void zoomInMap(MapRenderer mapctx);
void zoomOutMap(MapRenderer mapctx);
void freeMapRenderer(MapRenderer mapctx);

void getMapPostion(MapRenderer ctx, int *x, int *y, int *width, int *height);
void getMiniMapPostion(MapRenderer ctx, int *x, int *y, int *width, int *height);
Bool setCenterMiniMap(MapRenderer ctx, float x, float y, int *hexcol, int *hexrow);
Bool setCenterMap(MapRenderer ctx, float x, float y, int *hexcol, int *hexrow);

#ifdef __cplusplus
}
#endif

#endif // MAPRENDER_H
