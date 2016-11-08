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

#include "city.h"

#include <string>
#include <memory>
#include <vector>
#include <array>

namespace hexgame {
namespace state {

constexpr int max_stack_size = 8;
constexpr int max_rows = 600;
constexpr int max_cols = 600;
constexpr int min_rows = 20;
constexpr int min_cols = 20;

enum class TileType {
	Grass,
	Forest,
	Swamp,
	Desert,
	Hill,
	Mountain,
	Water,
	Temple,
	Ruin,
	City,
};

struct Tile {
	TileType type;
	bool frozen = false;
	bool road = false;
	const uint row;
	const uint col;
	uint currentMoveCost = 0;
	bool stacked = false;
	int  numUnits = 0;
	std::array<Unit *, max_stack_size> units;

	Tile(uint row, uint col) : row(row), col(col) { }
	~Tile() = default;

	using u_ptr = std::unique_ptr<Tile>;
};

class map_alloc_error : std::runtime_error {
public:
	map_alloc_error(std::string m) : std::runtime_error(m) { }
};

class Map {
public:
	Map(uint rows, uint cols, TileType defaultType);
	Map(std::string file);
	~Map();
	void write(std::string file);

	uint numRows() const { return tiles.size(); }
	uint numCols() const { return tiles[0].size(); }

	void placeUnit(Unit* unit, uint row, uint col);
	void moveUnit(Unit *unit, uint row, uint col);
	void killUnit(Unit *unit);
	void killTile(uint row, uint col);
	void selectUnit(Unit *unit);
	void deSelectUnit();

	const City& addCity(std::string name, uint row, uint col, uint walls);
	const City* getCity(uint row, uint col);

	Tile& tile(uint row, uint col) { return *tiles[row][col]; }

	CityManager cities;
	UnitManager units;

	uint row;
	uint col;
	Unit* selectedUnit = nullptr;

	using s_ptr = std::shared_ptr<Map>;

private:
	std::vector<std::vector<Tile::u_ptr>> tiles;

	void allocMap(uint rows, uint cols, TileType defaultType);
	void removeUnit(Unit* unit);
};

} //state
} // hexgame
