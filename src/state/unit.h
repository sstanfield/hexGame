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

#include "hextypes.h"

#include <string>
#include <memory>

namespace hexgame {
namespace state {

enum class Side {
	Side_Black = 0,
	Side_White,
	Side_Red,
	Side_Green,
	Side_Blue,
	Side_Orange,
	Side_Yellow,
	Side_Purple,
	Side_Grey  // Neutral (i.e. no side).
};
constexpr int num_sides = static_cast<int>(Side::Side_Grey);

constexpr int invalid_movecost  = -1;

struct MoveCost {
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

	using u_ptr = std::unique_ptr<MoveCost>;
};

struct Unit {
	uint id;
	Side side;
	std::string name;
	uint upkeep;
	uint strength;
	uint movement;
	uint moveCostId;
	uint baseMovePoints;
	uint row = 0;
	uint col = 0;

	using u_ptr = std::unique_ptr<Unit>;
};

class UnitManager {
public:
	UnitManager();
	~UnitManager();

	Unit *newUnit(Side sidet, std::string name, uint upkeep,
              uint strength, uint movement, uint moveCostId);
	void deleteUnit(Unit *unit);

	int aquireMoveCost(uint grass, uint forest, uint swamp,
                   uint desert, uint hill, uint mountain, uint water,
                   uint shore, uint bridge, uint road, uint city);
	MoveCost *getMoveCost(int id);

	void resetMovePoints(Side sidet);
private:
	struct UnitContext;
	std::unique_ptr<UnitContext> _ctx;
};

} //state
} // hexgame

