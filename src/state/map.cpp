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
#include "c_raii.h"

#include <cstdlib>
#include <cstdio>
#include <sstream>

namespace hexgame {
namespace state {

Map::Map(uint rows, uint cols, TileType defaultType)
{
	allocMap(rows, cols, defaultType);
}


Map::Map(std::string file)
{
	ssize_t read;
	std::string line;
	Map *map = nullptr;
	int rows = 0, cols = 0;

	CFile stream (file, "r");
	std::tie(read, line) = stream.readline();
	if (read != -1) {
		rows = strtol(line.c_str(), nullptr, 10);
	}
	std::tie(read, line) = stream.readline();
	if (read != -1) {
		cols = strtol(line.c_str(), nullptr, 10);
	}
	allocMap(rows, cols, TileType::Grass);

	int r = 0;
	std::tie(read, line) = stream.readline();
	while (read != -1 && r < rows) {
		int c = 0;
		while (line[c] && c < cols) {
			switch (line[c]) {
				case 'g': map->tiles[r][c]->type = TileType::Grass; break;
				case 'h': map->tiles[r][c]->type = TileType::Hill; break;
				case 'f': map->tiles[r][c]->type = TileType::Forest; break;
				case 'm': map->tiles[r][c]->type = TileType::Mountain; break;
				case 'w': map->tiles[r][c]->type = TileType::Water; break;
				case 's': map->tiles[r][c]->type = TileType::Swamp; break;
				case 'd': map->tiles[r][c]->type = TileType::Desert; break;
				case 't': map->tiles[r][c]->type = TileType::Temple; break;
				case 'r': map->tiles[r][c]->type = TileType::Ruin; break;
				case 'c': map->tiles[r][c]->type = TileType::City; break;
			}
			c++;
		}
		r++;
		std::tie(read, line) = stream.readline();
	}
}

Map::~Map() = default;

void Map::write(std::string file) {
	CFile stream(file, "w");

	fprintf(stream.fp, "%d\n", numRows());
	fprintf(stream.fp, "%d\n", numCols());

	uint r;
	for (r = 0; r < numRows(); r++) {
		uint c;
		for (c = 0; c < numCols(); c++) {
			char ch = 'g';
			switch (tiles[r][c]->type) {
				case TileType::Grass: break;
				case TileType::Hill:     ch = 'h'; break;
				case TileType::Forest:   ch = 'f'; break;
				case TileType::Mountain: ch = 'm'; break;
				case TileType::Water:    ch = 'w'; break;
				case TileType::Swamp:    ch = 's'; break;
				case TileType::Desert:   ch = 'd'; break;
				case TileType::Temple:   ch = 't'; break;
				case TileType::Ruin:     ch = 'r'; break;
				case TileType::City:     ch = 'c'; break;
			}
			fputc(ch, stream.fp);
		}
		fputc('\n', stream.fp);
	}
}

void Map::placeUnit(Unit* unit, uint row, uint col) {
	// XXX TODO, handle errors...
	Tile *toTile = tiles[row][col].get();
	if (toTile->numUnits >= max_stack_size) return;
	for (int i = 0; i < max_stack_size; i++) {
		if (!toTile->units[i]) {
			unit->row = row;
			unit->col = col;
			toTile->units[i] = unit;
			toTile->numUnits++;
			break;
		}
	}
}

void Map::removeUnit(Unit *unit) {
	Tile *fromTile = tiles[unit->row][unit->col].get();
	for (uint i = 0; i < max_stack_size; i++) {
		if (fromTile->units[i] && fromTile->units[i]->id == unit->id) {
			fromTile->units[i] = nullptr;
			fromTile->numUnits--;
			break;
		}
	}
}

void Map::moveUnit(Unit *unit, uint row, uint col) {
	removeUnit(unit);
	placeUnit(unit, row, col);
}

void Map::killUnit(Unit *unit) {
	removeUnit(unit);
	units.deleteUnit(unit);
}

void Map::killTile(uint row, uint col) {
	Tile *fromTile = tiles[row][col].get();
	for (int i = 0; i < max_stack_size; i++) {
		Unit *u = fromTile->units[i];
		fromTile->units[i] = nullptr;
		if (u) units.deleteUnit(u);
	}
	fromTile->numUnits = 0;
}

const City& Map::addCity(std::string name, uint row, uint col, uint walls) {
	const City& c = cities.newCity(name, row, col, walls);
	tiles[row][col]->type = TileType::City;
	return c;
}

const City* Map::getCity(uint row, uint col) {
	const City* c = cities.firstCity();
	while (c) {
		if (c->row == row && c->col == col) return c;
		c = cities.nextCity();
	}
	return nullptr;
}

void Map::allocMap(uint rows, uint cols, TileType defaultType)
{
	if (rows < min_rows || rows > max_rows || cols < min_cols || cols > max_cols) {
		std::stringstream ss;
		ss << "Invalid map dimensions: "<<rows<<", "<<cols<<".";
		throw map_alloc_error(ss.str());
	}
	tiles.resize(rows);
	uint r;
	for (r = 0; r < rows; r++) {
		tiles[r].resize(cols);
		uint c;
		for (c = 0; c < cols; c++) {
			Tile::u_ptr t = std::make_unique<Tile>(r, c);
			t->type = defaultType;
			tiles[r][c] = std::move(t);
		}
	}
	col = cols / 2;
	row = rows / 2;
}

} //state
} // hexgame
