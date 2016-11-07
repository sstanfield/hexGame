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
#include "city.h"

#include <cstdlib>

namespace hexgame {
namespace state {

struct CityManager::CityContext {
	std::vector<City::u_ptr> cities;
	uint cityPos;
};

CityManager::CityManager() : _ctx(std::make_unique<CityContext>())
{
}

CityManager::~CityManager() = default;

const City& CityManager::newCity(std::string name, uint row, uint col, uint walls)
{
	City::u_ptr c = std::make_unique<City>();
	c->name = name;
	c->row = row;
	c->col = col;
	c->walls = walls;
	c->side = Side::Side_Grey;
	c->id = _ctx->cities.size();
	_ctx->cities.push_back(std::move(c));
	return *_ctx->cities[c->id];
}

void CityManager::addCityProd(uint id, std::string basename, uint time, uint cost,
                              uint upkeep, uint strength, uint movement)
{
	UnitProd::u_ptr up = std::make_unique<UnitProd>();
	up->basename = basename;
	up->time = time;
	up->cost = cost;
	up->upkeep = upkeep;
	up->strength = strength;
	up->movement = movement;
	_ctx->cities.at(id)->units.push_back(std::move(up));
}

uint CityManager::numCities() const
{
	return _ctx->cities.size();
}

const City* CityManager::getCity(uint id) const
{
	return _ctx->cities.at(id).get();
}

const City* CityManager::firstCity()
{
	City *c = nullptr;
	_ctx->cityPos = 0;
	if (_ctx->cityPos < _ctx->cities.size())
		c = _ctx->cities[_ctx->cityPos].get();
	return c;
}

const City* CityManager::nextCity()
{
	City *c = nullptr;
	_ctx->cityPos++;
	if (_ctx->cityPos < _ctx->cities.size())
		c = _ctx->cities[_ctx->cityPos].get();
	return c;
}

} //state
} // hexgame

