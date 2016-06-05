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
#ifndef UNIT_H
#define UNIT_H

#include "gtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	Side_Black = 0,
	Side_White,
	Side_Red,
	Side_Green,
	Side_Blue,
	Side_Orange,
	Side_Yellow,
	Side_Purple,
	Side_Grey  // Neutral (i.e. no side).
} Side;
#define NUM_SIDES Side_Grey

#define INVALID_MOVECOST -1;
typedef struct {
	int  id;
	uint grass;
	uint forest;
	uint swamp;
	uint desert;
	uint hill;
	uint mountain;
	uint water;
	uint shore;
	uint bridge;
	uint road;
	uint city;
} MoveCost;

typedef struct {
	uint id;
	Side side;
	char *name;
	uint upkeep;
	uint strength;
	uint movement;
	uint moveCostId;
	uint baseMovePoints;
	uint row;
	uint col;
} Unit;

typedef void *UnitContext;

UnitContext initUnits();
void freeUnits(UnitContext ctx);

Unit *newUnit(UnitContext ctx, Side side, char *name, uint upkeep,
              uint strength, uint movement, uint moveCostId);
void deleteUnit(UnitContext ctx, Unit *unit);

int aquireMoveCost(UnitContext ctx, uint grass, uint forest, uint swamp,
                   uint desert, uint hill, uint mountain, uint water,
                   uint shore, uint bridge, uint road, uint city);
MoveCost *getMoveCost(UnitContext ctx, int id);

void resetMovePoints(UnitContext ctx, Side side);

#ifdef __cplusplus
}
#endif

#endif // UNIT_H
