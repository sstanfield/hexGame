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

#include <stdlib.h>
#include <string.h>

typedef struct {
	uint cityCnt;
	int cityAlloc;
	City **cities;
	uint cityPos;
} _CityContext;

static void deleteCity(City *city) {
	if (!city) return;
	if (city->name) free(city->name);
	free(city);
}

CityContext _initCities(uint numCities) {
	_CityContext *_ctx = (_CityContext *)malloc(sizeof(_CityContext));
	if (!_ctx) return NULL;
	memset(_ctx, 0, sizeof(_CityContext));
	if (numCities > 0) {
		_ctx->cityAlloc = numCities;
		_ctx->cities = (City **)malloc(sizeof(City *) * _ctx->cityAlloc);
		if (!_ctx->cities) _ctx->cityAlloc = 0;
	}
	return _ctx;
}

void _freeCities(CityContext ctx) {
	_CityContext *_ctx = (_CityContext *)ctx;
	if (!_ctx->cities) return;
	int i;
	for (i = 0; i < _ctx->cityCnt; i++) {
		City *c = _ctx->cities[i];
		deleteCity(c);
	}
	_ctx->cityCnt = 0;
	free(_ctx->cities);
	_ctx->cityAlloc = 0;
	_ctx->cities = NULL;
}

City *_newCity(CityContext ctx, char *name, uint row, uint col, uint walls) {
	_CityContext *_ctx = (_CityContext *)ctx;
	if (!_ctx->cities || _ctx->cityCnt >= _ctx->cityAlloc) {
		_ctx->cityAlloc += 10;
		_ctx->cities = (City **)realloc(_ctx->cities, sizeof(City *) * _ctx->cityAlloc);
		if (!_ctx->cities) return NULL;
	}
	City *c = (City *)malloc(sizeof(City));
	if (!c) return NULL;
	memset(c, 0, sizeof(City));
	if (name) {
		int len = strlen(name);
		c->name = (char *)malloc(len + 1);
		if (!c->name) {
			free(c);
			return NULL;
		}
		memcpy(c->name, name, len);
		c->name[len] = 0;
	}
	c->row = row;
	c->col = col;
	c->walls = walls;
	c->side = Side_Grey;
	c->id = _ctx->cityCnt;
	_ctx->cities[_ctx->cityCnt++] = c;
	return c;
}

void _addCityProd(CityContext ctx, uint id, char *basename, uint time, uint cost,
                 uint upkeep, uint strength, uint movement) {
	_CityContext *_ctx = (_CityContext *)ctx;
	City *c = _getCity(ctx, id);
	int i;
	for (i = 0; i < 4; i++) {
		if (!c->units[i].inUse) break;
	}
	if (i < 4) {  // No slots left if this fails...
		c->units[i].inUse = TRUE;
		char *basename;
		if (basename) {
			int len = strlen(basename);
			c->units[i].basename = (char *)malloc(len + 1);
			if (!c->units[i].basename) {
				return;
			}
			memcpy(c->units[i].basename, basename, len);
			c->units[i].basename[len] = 0;
		}
		c->units[i].time = time;
		c->units[i].cost = cost;
		c->units[i].upkeep = upkeep;
		c->units[i].strength = strength;
		c->units[i].movement = movement;
	}
}

uint _numCities(CityContext ctx) {
	_CityContext *_ctx = (_CityContext *)ctx;
	return _ctx->cityCnt;
}

City *_getCity(CityContext ctx, uint id) {
	_CityContext *_ctx = (_CityContext *)ctx;
	return _ctx->cities[id];
}

City *_firstCity(CityContext ctx) {
	_CityContext *_ctx = (_CityContext *)ctx;
	City *c = NULL;
	_ctx->cityPos = 0;
	if (_ctx->cityPos < _ctx->cityCnt)
		c = _ctx->cities[_ctx->cityPos];
	return c;
}

City *_nextCity(CityContext ctx) {
	_CityContext *_ctx = (_CityContext *)ctx;
	City *c = NULL;
	_ctx->cityPos++;
	if (_ctx->cityPos < _ctx->cityCnt)
		c = _ctx->cities[_ctx->cityPos];
	return c;
}
