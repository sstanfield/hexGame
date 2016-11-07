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

#include "unit.h"

#include <string>
#include <memory>
#include <vector>

namespace hexgame {
namespace state {

struct UnitProd {
	std::string basename;
	uint time;
	uint cost;
	uint upkeep;
	uint strength;
	uint movement;

	using u_ptr = std::unique_ptr<UnitProd>;
};

struct City {
	uint id;
	Side side;
	std::string name;
	std::vector<UnitProd::u_ptr> units;
	uint row;
	uint col;
	uint walls;

	using u_ptr = std::unique_ptr<City>;
};

class CityManager {
public:
	CityManager();
	~CityManager();
	const City& newCity(std::string name, uint row, uint col, uint walls);
	void        addCityProd(uint id, std::string basename, uint time, uint cost,
	                        uint upkeep, uint strength, uint movement);
	uint         numCities() const;
	const City* getCity(uint id) const;
	const City* firstCity();
	const City* nextCity();

private:
	struct CityContext;
	std::unique_ptr<CityContext> _ctx;
};

} //state
} // hexgame
