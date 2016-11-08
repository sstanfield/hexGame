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

#include <cstdlib>
#include <cstring>
#include <vector>
#include <array>

namespace hexgame {
namespace state {

struct UnitManager::UnitContext {
	std::array<std::vector<Unit::u_ptr>, num_sides> units;
	std::vector<MoveCost::u_ptr> moveCosts;
};

UnitManager::UnitManager() : _ctx(std::make_unique<UnitContext>())
{
}

UnitManager::~UnitManager() = default;

Unit *UnitManager::newUnit(Side sidet, std::string name, uint upkeep,
              uint strength, uint movement, uint moveCostId) {
	int id = -1;
	int side = static_cast<int>(sidet);
	uint c = 0;
	for (auto& unit : _ctx->units[side]) {
		if (!unit) {
			id = c;
			break;
		}
		c++;
	}
	if (id == -1) {
		id = _ctx->units[side].size();
		_ctx->units[side].push_back(nullptr);
	}
	Unit::u_ptr u = std::make_unique<Unit>();
	u->name = name;
	u->id = id;
	u->side = sidet;
	u->upkeep = upkeep;
	u->strength = strength;
	u->movement = movement;
	u->moveCostId = moveCostId;
	u->baseMovePoints = movement;
	_ctx->units[side][id] = std::move(u);
	return _ctx->units[side][id].get();
}

void UnitManager::deleteUnit(Unit *unit) {
	if (!unit) return;
	_ctx->units[static_cast<int>(unit->side)][unit->id] = nullptr;
}


int UnitManager::aquireMoveCost(uint grass, uint forest, uint swamp,
                   uint desert, uint hill, uint mountain, uint water,
                   uint shore, uint bridge, uint road, uint city) {
	for (auto& mc : _ctx->moveCosts) {
		if (mc->grass == grass && mc->forest == forest && mc->swamp == swamp &&
		    mc->desert == desert && mc->hill == hill && mc->mountain == mountain &&
		    mc->water == water && mc->shore == shore && mc->bridge == bridge &&
		    mc->road == road && mc->city == city)
			return mc->id;
	}

	MoveCost::u_ptr mc = std::make_unique<MoveCost>();
	int id = mc->id = _ctx->moveCosts.size();
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
	_ctx->moveCosts.push_back(std::move(mc));
	return id;
}

MoveCost *UnitManager::getMoveCost(int id) {
	return _ctx->moveCosts.at(id).get();
}

void UnitManager::resetMovePoints(Side sidet) {
	int side = static_cast<int>(sidet);
	for (auto& u : _ctx->units[side]) {
		u->movement = u->baseMovePoints + (u->movement>0?(u->movement==1?1:2):0);
	}
}

} //state
} // hexgame

