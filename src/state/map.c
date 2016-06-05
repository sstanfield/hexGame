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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Map *allocMap(uint rows, uint cols, TileType defaultType) {
	Map *map;
	map = (Map *)malloc(sizeof(Map));
	map->selectedUnit = NULL;
	map->tiles = (Tile **)malloc(rows * sizeof(Tile *));
	int r;
	for (r = 0; r < rows; r++) {
		map->tiles[r] = (Tile *)malloc(cols * sizeof(Tile));
		memset(map->tiles[r], 0, cols * sizeof(Tile));
		int c;
		for (c = 0; c < cols; c++) {
			map->tiles[r][c].row = r;
			map->tiles[r][c].col = c;
			map->tiles[r][c].type = defaultType;
		}
	}
	map->numRows = rows;
	map->numCols = cols;
	map->col = cols / 2;
	map->row = rows / 2;
	map->cityCtx = _initCities(20);
	map->unitCtx = initUnits();
	return map;
}

void freeMap(Map *map) {
	int r;
	for (r = 0; r < map->numRows; r++) {
		int c;
		for (c = 0; c < map->numCols; c++) {
			if (map->tiles[r][c].units) {
				free(map->tiles[r][c].units);
			}
		}
		free(map->tiles[r]);
	}
	free(map->tiles);
	CityContext cctx = map->cityCtx;
	UnitContext uctx = map->unitCtx;
	free(map);
	_freeCities(cctx);
	freeUnits(uctx);
}

Map *readMap(char *file) {
	FILE *stream = NULL;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	Map *map = NULL;
	int rows, cols;

	stream = fopen(file, "r");
	if (stream == NULL) goto done;

	if ((read = getline(&line, &len, stream)) != -1) {
		rows = strtol(line, NULL, 10);
	}
	if ((read = getline(&line, &len, stream)) != -1) {
		cols = strtol(line, NULL, 10);
	}
	if (rows < 20 || rows > 600) goto done;
	if (cols < 20 || cols > 600) goto done;
	map = allocMap(rows, cols, Grass);

	int r = 0;
	while ((read = getline(&line, &len, stream)) != -1 && r < rows) {
		int c = 0;
		while (line[c] && c < cols) {
			switch (line[c]) {
			case 'g': map->tiles[r][c].type = Grass; break;
			case 'h': map->tiles[r][c].type = Hill; break;
			case 'f': map->tiles[r][c].type = Forest; break;
			case 'm': map->tiles[r][c].type = Mountain; break;
			case 'w': map->tiles[r][c].type = Water; break;
			case 's': map->tiles[r][c].type = Swamp; break;
			case 'd': map->tiles[r][c].type = Desert; break;
			case 'c': map->tiles[r][c].type = CityCenter; break;
			case '1': map->tiles[r][c].type = City1; break;
			case '2': map->tiles[r][c].type = City2; break;
			case '3': map->tiles[r][c].type = City3; break;
			case '4': map->tiles[r][c].type = City4; break;
			case '5': map->tiles[r][c].type = City5; break;
			case '6': map->tiles[r][c].type = City6; break;
			}
			c++;
		}
		r++;
//		printf("Retrieved line of length %zu :\n", read);
//		printf("%s", line);
	}

	done:
	if (line) free(line);
	if (stream) fclose(stream);
	map->cityCtx = _initCities(20);
	map->unitCtx = initUnits();
	return map;
}

void writeMap(Map *map, char *file) {
	FILE *stream = NULL;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	int rows, cols;

	stream = fopen(file, "w");
	if (stream == NULL) goto done;

	fprintf(stream, "%d\n", map->numRows);
	fprintf(stream, "%d\n", map->numCols);

	int r;
	for (r = 0; r < map->numRows; r++) {
		int c;
		for (c = 0; c < map->numCols; c++) {
			char ch = 'g';
			switch (map->tiles[r][c].type) {
			case Grass: break;
			case Hill: ch = 'h'; break;
			case Forest: ch = 'f'; break;
			case Mountain: ch = 'm'; break;
			case Water: ch = 'w'; break;
			case Swamp: ch = 's'; break;
			case Desert: ch = 'd'; break;
			case CityCenter: ch = 'c'; break;
			case City1: ch = '1'; break;
			case City2: ch = '2'; break;
			case City3: ch = '3'; break;
			case City4: ch = '4'; break;
			case City5: ch = '5'; break;
			case City6: ch = '6'; break;
			}
			fputc(ch, stream);
		}
		fputc('\n', stream);
	}

	done:
	if (stream) fclose(stream);
}

void placeUnit(Map *map, Unit *unit, uint row, uint col) {
	// XXX TODO, handle errors...
	Tile *toTile = &map->tiles[row][col];
	if (toTile->numUnits >= MAX_STACK_SIZE) return;
	if (!toTile->units) {
		toTile->numUnits = 0;
		toTile->units = (Unit **)malloc(sizeof(Unit *) * MAX_STACK_SIZE);
		if (!toTile->units) return;  // OOM
		memset(toTile->units, 0, sizeof(Unit *) * MAX_STACK_SIZE);
	}
	int i;
	for (i = 0; i < MAX_STACK_SIZE; i++) {
		if (!toTile->units[i]) {
			toTile->units[i] = unit;
			toTile->numUnits++;
			break;
		}
	}
	unit->row = row;
	unit->col = col;
}

static void removeUnit(Map *map, Unit *unit) {
	Tile *fromTile = &map->tiles[unit->row][unit->col];
	if (fromTile->units) {
		int i;
		for (i = 0; i < MAX_STACK_SIZE; i++) {
			Unit *u = fromTile->units[i];
			if (u && u->id == unit->id) {
				fromTile->units[i] = NULL;
				fromTile->numUnits--;
				break;
			}
		}
	}
	if (fromTile->numUnits <= 0) {
		free(fromTile->units);
		fromTile->units = NULL;
	}
}

void moveUnit(Map *map, Unit *unit, uint row, uint col) {
	removeUnit(map, unit);
	placeUnit(map, unit, row, col);
}

void killUnit(Map *map, Unit *unit) {
	removeUnit(map, unit);
	deleteUnit(map->unitCtx, unit);
}

void killTile(Map *map, uint row, uint col) {
	Tile *fromTile = &map->tiles[row][col];
	if (fromTile->units) {
		int i;
		for (i = 0; i < MAX_STACK_SIZE; i++) {
			Unit *u = fromTile->units[i];
			if (u) deleteUnit(map->unitCtx, u);
		}
		free(fromTile->units);
		fromTile->units = NULL;
		fromTile->numUnits = 0;
	}
}

City *addCity(Map *map, char *name, uint row, uint col, uint walls) {
	City *c = _newCity(map->cityCtx, name, row, col, walls);
	map->tiles[row][col].type = CityCenter;
	map->tiles[row-1][col].type = City1;
	map->tiles[row+1][col].type = City4;
	map->tiles[row][col-1].type = col%2?City5:City6;
	map->tiles[row+(col%2?-1:1)][col-1].type = col%2?City6:City5;
	map->tiles[row][col+1].type = col%2?City3:City2;
	map->tiles[row+(col%2?-1:1)][col+1].type = col%2?City2:City3;
	return c;
}

void addCityProd(Map *map, uint id, char *basename, uint time, uint cost,
                 uint upkeep, uint strength, uint movement) {
	_addCityProd(map->cityCtx, id, basename, time, cost, upkeep, strength, movement);
}

uint numCities(Map *map) {
	return _numCities(map->cityCtx);
}

City *getCity(Map *map, uint row, uint col) {
	City *c = _firstCity(map->cityCtx);
	while (c) {
		if (c->row == row && c->col == col) return c;
		c = _nextCity(map->cityCtx);
	}
	return NULL;
}
