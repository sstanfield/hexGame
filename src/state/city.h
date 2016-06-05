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
#ifndef CITY_H
#define CITY_H

#include "unit.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	Bool inUse;
	char *basename;
	uint time;
	uint cost;
	uint upkeep;
	uint strength;
	uint movement;
} UnitProd;

typedef struct {
	uint id;
	Side side;
	char *name;
	UnitProd units[4];
	uint row;
	uint col;
	uint walls;
} City;

typedef void *CityContext;

CityContext _initCities(uint numCities);
void _freeCities(CityContext ctx);
City *_newCity(CityContext ctx, char *name, uint row, uint col, uint walls);
void _addCityProd(CityContext ctx, uint id, char *basename, uint time, uint cost,
                 uint upkeep, uint strength, uint movement);
uint _numCities(CityContext ctx);
City *_getCity(CityContext ctx, uint id);
City *_firstCity(CityContext ctx);
City *_nextCity(CityContext ctx);

#ifdef __cplusplus
}
#endif

#endif // CITY_H
