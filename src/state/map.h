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
#ifndef MAP_H
#define MAP_H

#include "city.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_STACK_SIZE 8

typedef enum {
	Grass,
	Forest,
	Swamp,
	Desert,
	Hill,
	Mountain,
	Water,
	Temple,
	Ruin,
	CityCenter,
	City1,
	City2,
	City3,
	City4,
	City5,
	City6
} TileType;

typedef struct {
	TileType type;
	Bool frozen;
	Bool road;
	uint row;
	uint col;
	uint currentMoveCost;
	Bool stacked;
	int  numUnits;
	Unit **units;
} Tile;

typedef struct {
	Tile **tiles;
	uint numRows;
	uint numCols;
	uint row;
	uint col;
	Unit *selectedUnit;

	CityContext cityCtx;
	UnitContext unitCtx;
} Map;

Map *allocMap(uint rows, uint cols, TileType defaultType);
void freeMap(Map *map);
Map *readMap(char *file);
void writeMap(Map *map, char *file);

void placeUnit(Map *map, Unit *unit, uint row, uint col);
void moveUnit(Map *map, Unit *unit, uint row, uint col);
void killUnit(Map *map, Unit *unit);
void killTile(Map *map, uint row, uint col);
void selectUnit(Map *map, Unit *unit);
void deSelectUnit(Map *map);

City *addCity(Map *map, char *name, uint row, uint col, uint walls);
void addCityProd(Map *map, uint id, char *basename, uint time, uint cost,
                 uint upkeep, uint strength, uint movement);
uint numCities(Map *map);
City *getCity(Map *map, uint row, uint col);


#ifdef __cplusplus
}
#endif

#endif // MAP_H
