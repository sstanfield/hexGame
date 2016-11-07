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
#include "map.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace hexgame {
namespace state {

struct RowCol {
	unsigned short row;
	unsigned short col;
};


// XXX TODO, don't be D-U-M with memory..
#define MAX_STACK 400
static RowCol rcStack[MAX_STACK];
static int rcStackLoc = 0;
static RowCol rcRunStack[MAX_STACK];
static int rcRunStackLoc = 0;
static bool allUpdated;

static void addCloseToStack(Map *map, int row, int col, bool stackCon) {
	if (row < 0 || row >= (int)map->numRows() || col < 0 || col >= (int)map->numCols()) return;
	if (rcStackLoc+6 >= MAX_STACK) {
		printf("ERROR: Out of map move stack space!\n");
		return;
	}
	int trow, tcol;
	trow = row-1; tcol = col;
	if (trow >= 0 && trow < (int)map->numRows() && tcol >= 0 && tcol < (int)map->numCols()) {
		if (map->tile(trow, tcol).stacked == stackCon) {
			rcStack[rcStackLoc].row = trow;
			rcStack[rcStackLoc++].col = tcol;
			map->tile(trow, tcol).stacked = !map->tile(trow, tcol).stacked;
		}
	}
	trow = row+1; tcol = col;
	if (trow >= 0 && trow < (int)map->numRows() && tcol >= 0 && tcol < (int)map->numCols()) {
		if (map->tile(trow, tcol).stacked == stackCon) {
			rcStack[rcStackLoc].row = trow;
			rcStack[rcStackLoc++].col = tcol;
			map->tile(trow, tcol).stacked = !map->tile(trow, tcol).stacked;
		}
	}
	trow = row; tcol = col-1;
	if (trow >= 0 && trow < (int)map->numRows() && tcol >= 0 && tcol < (int)map->numCols()) {
		if (map->tile(trow, tcol).stacked == stackCon) {
			rcStack[rcStackLoc].row = trow;
			rcStack[rcStackLoc++].col = tcol;
			map->tile(trow, tcol).stacked = !map->tile(trow, tcol).stacked;
		}
	}
	trow = row; tcol = col+1;
	if (trow >= 0 && trow < (int)map->numRows() && tcol >= 0 && tcol < (int)map->numCols()) {
		if (map->tile(trow, tcol).stacked == stackCon) {
			rcStack[rcStackLoc].row = trow;
			rcStack[rcStackLoc++].col = tcol;
			map->tile(trow, tcol).stacked = !map->tile(trow, tcol).stacked;
		}
	}
	trow = row+(col%2?-1:1); tcol = col-1;
	if (trow >= 0 && trow < (int)map->numRows() && tcol >= 0 && tcol < (int)map->numCols()) {
		if (map->tile(trow, tcol).stacked == stackCon) {
			rcStack[rcStackLoc].row = trow;
			rcStack[rcStackLoc++].col = tcol;
			map->tile(trow, tcol).stacked = !map->tile(trow, tcol).stacked;
		}
	}
	trow = row+(col%2?-1:1); tcol = col+1;
	if (trow >= 0 && trow < (int)map->numRows() && tcol >= 0 && tcol < (int)map->numCols()) {
		if (map->tile(trow, tcol).stacked == stackCon) {
			rcStack[rcStackLoc].row = trow;
			rcStack[rcStackLoc++].col = tcol;
			map->tile(trow, tcol).stacked = !map->tile(trow, tcol).stacked;
		}
	}
}

static int tileEntryCost(Map *map, MoveCost *cost, int row, int col) {
	if (row < 0 || row >= (int)map->numRows() || col < 0 || col >= (int)map->numCols()) return 0;
	Tile& tile = map->tile(row, col);
	int tcost = 0;
	switch (tile.type) {
		case TileType::Grass: tcost = cost->grass; break;
		case TileType::Hill: tcost = cost->hill;  break;
		case TileType::Forest: tcost = cost->forest;  break;
		case TileType::Mountain: tcost = cost->mountain;  break;
		case TileType::Water: tcost = cost->water;  break;
		case TileType::Swamp: tcost = cost->swamp; break;
		case TileType::Desert: tcost = cost->desert; break;
		case TileType::Temple: tcost = cost->grass;  break;
		case TileType::Ruin: tcost = cost->grass;  break;
		case TileType::City: tcost = cost->city;  break;
	}
	return tcost;
}

static void entryCost(Map *map, int tcost, int row, int col, int *points) {
	if (row < 0 || row >= (int)map->numRows() || col < 0 || col >= (int)map->numCols()) return;
	if (map->tile(row, col).currentMoveCost > 0) {
		if (!*points || ((int)map->tile(row, col).currentMoveCost + tcost) < *points) {
			*points = map->tile(row, col).currentMoveCost + tcost;
		}
	}
}

static void genEntryCost(Map *map, MoveCost *cost, int row, int col, bool notCheckedState) {
	if (row < 0 || row >= (int)map->numRows() || col < 0 || col >= (int)map->numCols()) return;
	int tcost = tileEntryCost(map, cost, row, col);
	if (tcost == 0) return;
	int points = 0;
	entryCost(map, tcost, row-1, col, &points);
	entryCost(map, tcost, row+1, col, &points);
	entryCost(map, tcost, row, col-1, &points);
	entryCost(map, tcost, row, col+1, &points);
	entryCost(map, tcost, row+(col%2?-1:1), col+1, &points);
	entryCost(map, tcost, row+(col%2?-1:1), col-1, &points);
	if ((points && points < (int)map->tile(row, col).currentMoveCost) ||
	    (!map->tile(row, col).currentMoveCost && points > 0)) {
		map->tile(row, col).currentMoveCost = points;
		addCloseToStack(map, row, col, notCheckedState);
	} else {
		allUpdated = false;
		// Make this hex available for later calculations.
		map->tile(row, col).stacked = !map->tile(row, col).stacked;
	}
//	addCloseToStack(map, row, col, notCheckedState);
}

static void prepClose(Map *map, MoveCost *cost, int row, int col) {
	if (row < 0 || row >= (int)map->numRows() || col < 0 || col >= (int)map->numCols()) return;
	map->tile(row, col).currentMoveCost = tileEntryCost(map, cost, row, col);
}

static void setStackedState(Map *map, int row, int col, bool stackedState) {
	if (row < 0 || row >= (int)map->numRows() || col < 0 || col >= (int)map->numCols()) return;
	map->tile(row, col).stacked = stackedState;
}

// Does one pass of the map for move points.
static void scanMapForPoints(Map *map, MoveCost *cost, int row, int col, bool stackedState) {
	if (row < 0 || row >= (int)map->numRows() || col < 0 || col >= (int)map->numCols()) return;
	rcStackLoc = 0;
	map->tile(row, col).stacked = stackedState;
	setStackedState(map, row-1, col, stackedState);
	setStackedState(map, row+1, col, stackedState);
	setStackedState(map, row, col-1, stackedState);
	setStackedState(map, row, col+1, stackedState);
	setStackedState(map, row+(col%2?-1:1), col-1, stackedState);
	setStackedState(map, row+(col%2?-1:1), col+1, stackedState);
	addCloseToStack(map, row-1, col, !stackedState);
	addCloseToStack(map, row+1, col, !stackedState);
	addCloseToStack(map, row, col-1, !stackedState);
	addCloseToStack(map, row, col+1, !stackedState);
	addCloseToStack(map, row+(col%2?-1:1), col-1, !stackedState);
	addCloseToStack(map, row+(col%2?-1:1), col+1, !stackedState);
	while (rcStackLoc) {
		memcpy(rcRunStack, rcStack, rcStackLoc * sizeof(RowCol));
		rcRunStackLoc = rcStackLoc;
		rcStackLoc = 0;
		int i;
		for (i = 0; i < rcRunStackLoc; i++) {
			genEntryCost(map, cost, rcRunStack[i].row, rcRunStack[i].col, !stackedState);
		}
	}
}

void Map::selectUnit(Unit *unit) {
	deSelectUnit();
	allUpdated = false;
	int row = unit->row;
	int col = unit->col;
	MoveCost *cost = units.getMoveCost(unit->moveCostId);
	if (!cost) return;

	tiles[row][col]->stacked = true;
	prepClose(this, cost, row-1, col);
	prepClose(this, cost, row+1, col);
	prepClose(this, cost, row, col-1);
	prepClose(this, cost, row, col+1);
	prepClose(this, cost, row+(col%2?-1:1), col-1);
	prepClose(this, cost, row+(col%2?-1:1), col+1);
	bool stackedState = true;
	//int pass = 1;
//	while (!allUpdated && pass < 5) {
		allUpdated = true;
		scanMapForPoints(this, cost, row, col, stackedState);
		stackedState = !stackedState;
//		printf("XXX pass %d\n", pass++);
//	}
	selectedUnit = unit;
}

void Map::deSelectUnit() {
	selectedUnit = nullptr;
	int r;
	for (r = 0; r < (int)numRows(); r++) {
		int c;
		for (c = 0; c < (int)numCols(); c++) {
			tiles[r][c]->currentMoveCost = 0;
			tiles[r][c]->stacked = false;
		}
	}
	rcStackLoc = 0;
}

} //state
} // hexgame
