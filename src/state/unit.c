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
#include "unit.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
	uint sideCnts[NUM_SIDES];
	int sideAllocs[NUM_SIDES];
	Unit **units[NUM_SIDES];

	uint moveCnt;
	int moveAlloc;
	MoveCost **moveCost;
} _UnitContext;

UnitContext initUnits() {
	_UnitContext *_ctx = (_UnitContext *)malloc(sizeof(_UnitContext));
	if (!_ctx) return NULL;
	memset(_ctx, 0, sizeof(_UnitContext));
	int i;
	for (i = 0; i < NUM_SIDES; i++) {
		_ctx->sideCnts[i] = 0;
		_ctx->sideAllocs[i] = 20;  // Initial unit capactiy of 20.
		_ctx->units[i] = (Unit **)malloc(sizeof(Unit *) * _ctx->sideAllocs[i]);
		if (!_ctx->units[i]) return NULL;  // Out of memory, do more to fail hard...
		memset(_ctx->units[i], 0, sizeof(Unit *) * _ctx->sideAllocs[i]);
	}

	_ctx->moveAlloc = 10;  // Initial move type capactiy of 10.
	_ctx->moveCnt = 0;
	_ctx->moveCost = (MoveCost **)malloc(sizeof(MoveCost *) * _ctx->moveAlloc);
	if (!_ctx->moveCost) return NULL;  // Out of memory, do more to fail hard...
	memset(_ctx->moveCost, 0, sizeof(MoveCost *) * _ctx->moveAlloc);
	return _ctx;
}

void freeUnits(UnitContext ctx) {
	_UnitContext *_ctx = (_UnitContext *)ctx;
	int i;
	for (i = 0; i < NUM_SIDES; i++) {
		int c;
		for (c = 0; c < _ctx->sideCnts[i]; c++) {
			Unit *u = _ctx->units[i][c];
			if (u) deleteUnit(ctx, u);
		}
		free(_ctx->units[i]);
	}

	for (i = 0; i < _ctx->moveCnt; i++) {
		MoveCost *mc = _ctx->moveCost[i];
		if (mc) free(mc);
	}
	free(_ctx);
}

Unit *newUnit(UnitContext ctx, Side side, char *name, uint upkeep,
              uint strength, uint movement, uint moveCostId) {
	_UnitContext *_ctx = (_UnitContext *)ctx;
	uint id = -1;
	int c;
	for (c = 0; c < _ctx->sideCnts[side]; c++) {
		if (!_ctx->units[side][c]) {
			id = c;
			break;
		}
	}
	if (id == -1) id = _ctx->sideCnts[side]++;
	if (id >= _ctx->sideAllocs[side]) {  // Make room for more units.
		_ctx->sideAllocs[side] += 20;
		_ctx->units[side] = (Unit **)realloc(_ctx->units[side],
		                                     sizeof(Unit *) * _ctx->sideAllocs[side]);
		if (!_ctx->units[side]) return NULL;
	}
	Unit *u = (Unit *)malloc(sizeof(Unit));
	if (!u) return NULL;
	memset(u, 0, sizeof(Unit));
	if (name) {
		int len = strlen(name);
		u->name = (char *)malloc(len + 1);
		if (!u->name) {
			free(u);
			return NULL;
		}
		memcpy(u->name, name, len);
		u->name[len] = 0;
	}
	u->id = id;
	u->side = side;
	u->upkeep = upkeep;
	u->strength = strength;
	u->movement = movement;
	u->moveCostId = moveCostId;
	u->baseMovePoints = movement;
	_ctx->units[side][id] = u;
	return u;
}

void deleteUnit(UnitContext ctx, Unit *unit) {
	_UnitContext *_ctx = (_UnitContext *)ctx;
	if (!unit) return;
	_ctx->units[unit->side][unit->id] = NULL;
	if (unit->name) free(unit->name);
	free(unit);
}


int aquireMoveCost(UnitContext ctx, uint grass, uint forest, uint swamp,
                   uint desert, uint hill, uint mountain, uint water,
                   uint shore, uint bridge, uint road, uint city) {
	_UnitContext *_ctx = (_UnitContext *)ctx;
	int i;
	for (i = 0; i < _ctx->moveCnt; i++) {
		MoveCost *mc = _ctx->moveCost[i];
		if (mc->grass == grass && mc->forest == forest && mc->swamp == swamp &&
		    mc->desert == desert && mc->hill == hill && mc->mountain == mountain &&
		    mc->water == water && mc->shore == shore && mc->bridge == bridge &&
		    mc->road == road && mc->city == city)
			return mc->id;
	}

	if (_ctx->moveCnt >= _ctx->moveAlloc) {
		_ctx->moveAlloc += 10;
		_ctx->moveCost = (MoveCost **)realloc(_ctx->moveCost,
		                                      sizeof(MoveCost *) * _ctx->moveAlloc);
		if (!_ctx->moveCost) return INVALID_MOVECOST;
	}

	MoveCost *mc = (MoveCost *)malloc(sizeof(MoveCost));
	if (!mc) return INVALID_MOVECOST;
	memset(mc, 0, sizeof(MoveCost));
	mc->grass = grass;
	mc->forest = forest;
	mc->swamp = swamp;
	mc->desert = desert;
	mc->hill = hill;
	mc->mountain = mountain;
	mc->water = water;
	mc->shore = shore;
	mc->bridge = bridge;
	mc->road = road;
	mc->city = city;
	if (_ctx->moveCnt >= _ctx->moveAlloc) {
		_ctx->moveAlloc += 10;
		_ctx->moveCost = (MoveCost **)realloc(_ctx->moveCost,
		                                      sizeof(MoveCost *) * _ctx->moveAlloc);
		if (!_ctx->moveCost) return INVALID_MOVECOST;
	}
	_ctx->moveCost[_ctx->moveCnt] = mc;
	return _ctx->moveCnt++;
}

MoveCost *getMoveCost(UnitContext ctx, int id) {
	_UnitContext *_ctx = (_UnitContext *)ctx;
	if (id < 0 || id > _ctx->moveCnt) return NULL;
	return _ctx->moveCost[id];
}

void resetMovePoints(UnitContext ctx, Side side) {
	_UnitContext *_ctx = (_UnitContext *)ctx;
	int c;
	for (c = 0; c < _ctx->sideCnts[side]; c++) {
		Unit *u = _ctx->units[side][c];
		// Can carry over 1 or 2 two ununsed points.
		u->movement = u->baseMovePoints + (u->movement>0?(u->movement==1?1:2):0);
	}
}
