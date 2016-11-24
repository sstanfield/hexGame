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
#include "distancemap.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    float dx;
    float dy;
} Point;

static float DistSqr(const Point *p) { return (p->dx * p->dx) + (p->dy * p->dy); }
static float Dist(const Point *p) { return sqrtf(DistSqr(p)); }

static Point Outside = {1e6f, 1e6f};
static Point Inside = {0, 0};

static Point *get(Point *grid, int x, int y, int width, int height)
{
	if (x >= 0 && y >= 0 && x < width && y < height)
		return &grid[x + y * width];
	else
		return &Outside;
}

static Point *compare(Point *grid, Point *p, Point *other,
                      int x, int y, int offsetx, int offsety, int width, int height)
{
//	Point other = Get(x + offsetx, y + offsety);
	memcpy(other, get(grid, x + offsetx, y + offsety, width, height), sizeof(Point));
	Point o2 = {other->dx + offsetx, other->dy + offsety};
	memcpy(other, &o2, sizeof(Point));
//	other = new Point(other.dx + offsetx, other.dy + offsety);
	return DistSqr(other) < DistSqr(p) ? other : p;
}

static void generate(Point *grid, int width, int height)
{
	Point tmp;
	Point g;
	Point *p;
	int y;
	for (y = 0; y < height; y++)
	{
		int x;
		for (x = 0; x < width; x++)
		{
			memcpy(&g, &grid[(y * width) + x], sizeof(Point));
			p = &g;
			p = compare(grid, p, &tmp, x, y, -1, 0, width, height);
			p = compare(grid, p, &tmp, x, y, 0, -1, width, height);
			p = compare(grid, p, &tmp, x, y, -1, -1, width, height);
			p = compare(grid, p, &tmp, x, y, 1, -1, width, height);
			memcpy(&grid[(y * width) + x], p, sizeof(Point));
		}

		for (x = width - 1; x >= 0; x--)
		{
			memcpy(&g, &grid[(y * width) + x], sizeof(Point));
			p = &g;
			p = compare(grid, p, &tmp, x, y, 1, 0, width, height);
			memcpy(&grid[(y * width) + x], p, sizeof(Point));
		}
	}

	for (y = height - 1; y >= 0; y--)
	{
		int x;
		for (x = width - 1; x >= 0; x--)
		{
			memcpy(&g, &grid[(y * width) + x], sizeof(Point));
			p = &g;
			p = compare(grid, p, &tmp, x, y, 1, 0, width, height);
			p = compare(grid, p, &tmp, x, y, 0, 1, width, height);
			p = compare(grid, p, &tmp, x, y, -1, 1, width, height);
			p = compare(grid, p, &tmp, x, y, 1, 1, width, height);
			memcpy(&grid[(y * width) + x], p, sizeof(Point));
		}

		for (x = 0; x < width; x++)
		{
			memcpy(&g, &grid[(y * width) + x], sizeof(Point));
			p = &g;
			p = compare(grid, p, &tmp, x, y, -1, 0, width, height);
			memcpy(&grid[(y * width) + x], p, sizeof(Point));
		}
	}
}

static void fromBitmap(Point *g1, Point *g2, int width, int height,
                       const unsigned char *in)
{
	int idx = 0;

	int y;
	for (y = 0; y < height; y++)
	{
		int x;
		for (x = 0; x < width; x++)
		{
			if (in[idx] > 0) {
				memcpy(&g2[idx], &Inside, sizeof(Point));
				memcpy(&g1[idx], &Outside, sizeof(Point));
			} else {
				memcpy(&g1[idx], &Inside, sizeof(Point));
				memcpy(&g2[idx], &Outside, sizeof(Point));
			}
			idx++;
		}
	}
}


void generateDistanceMap(unsigned char *out, const unsigned char *in,
                         int width, int height) {
	int gSize = width * height * sizeof(Point);
	Point *g1 = (Point *)malloc(gSize);
	Point *g2 = (Point *)malloc(gSize);;
	if (g1 == NULL || g2 == NULL) return;
	memset(g1, 0, gSize);
	memset(g2, 0, gSize);
	fromBitmap(g1, g2, width, height, in);
	generate(g1, width, height);
	generate(g2, width, height);

	int scalefactor = 0;
	float spread = (float)(width<height?width:height) / (float)(1 << scalefactor);
	float min = -spread;
	float max = spread;

	int idx = 0;
	int y;
	for (y = 0; y < height; y++)
	{
		int x;
		for (x = 0; x < width; x++)
		{
			float dst = Dist(&g1[idx]) - Dist(&g2[idx]);
			dst = dst < 0
			      ? -128 * (dst - min) / min
			      : 128 + 128 * dst / max;

			float m = dst>255?255:dst;
			unsigned char val = (unsigned char)(m<0?0:m);
			out[idx] = val;
			idx++;
		}
	}

	free(g1);
	free(g2);
}
