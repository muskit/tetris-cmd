#pragma once
#ifndef Tetro_h
#define Tetro_h

#include "Windows.h"
#include "char_info_utils.h"

typedef struct STetro
{
	CHAR_INFO tetro[4][4];

	uint8_t x = 7;
	uint8_t y = 18;

	uint8_t dim = 4;

	short id;

	STetro(const CHAR_INFO charinfo[4][4], uint8_t num, bool isthree = false)
	{
		id = num;
		if (isthree)
		{
			dim = 3;
		}
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				tetro[x][y] = charinfo[y][x];
			}
		}
	}

	STetro() {}

} STetro;


// TODO: make each array an STetro
namespace Tetro
{
	const CHAR_INFO _I[4][4] = { { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(178, 0b11), char_info(178, 0b11), char_info(178, 0b11), char_info(178, 0b11) },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
						        { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO _L[4][4] = { { char_info(' '), char_info(' '), char_info(178, 0b110), char_info(' ') },
	                            { char_info(178, 0b110), char_info(178, 0b110), char_info(178, 0b110), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO _J[4][4] = { { char_info(178,1), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(178,1), char_info(178,1), char_info(178,1), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO _T[4][4] = { { char_info(' '), char_info(178, 0b101), char_info(' '), char_info(' ') },
	                            { char_info(178, 0b101), char_info(178, 0b101), char_info(178, 0b101), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO _S[4][4] = { { char_info(' '), char_info(178, 0b10), char_info(178, 0b10), char_info(' ') },
	                            { char_info(178, 0b10), char_info(178, 0b10), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO _Z[4][4] = { { char_info(178, 0b100), char_info(178, 0b100), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(178, 0b100), char_info(178, 0b100), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO _O[4][4] = { { char_info(' '), char_info(178, 0b1110), char_info(178, 0b1110), char_info(' ') },
	                            { char_info(' '), char_info(178, 0b1110), char_info(178, 0b1110), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const STetro I(_I, 0);
	const STetro L(_L, 1, true);
	const STetro J(_J, 2, true);
	const STetro T(_T, 3, true);
	const STetro S(_S, 4, true);
	const STetro Z(_Z, 5, true);
	const STetro O(_O, 6, true);

	const STetro tetro[7] = { I, L, J, T, S, Z, O };
}

// Given a tetromino in 4x4 array and orientation value, return an STetro with
// appropriate tetromino and orientation.
STetro get_STetro(const CHAR_INFO tet[4][4])
{
	STetro result;
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			result.tetro[x][y] = tet[y][x];
		}
	}
	return result;
}

// Return STetro given, clockwise.
STetro cw(STetro a)
{
	if (a.id == 6)
		return a;

	STetro result;
	charinfo_clear(*result.tetro, 4 * 4);
	result.x = a.x;
	result.y = a.y;
	result.dim = a.dim;

	uint8_t ry = result.dim - 1;

	for (int y = 0; y < result.dim; y++)
	{
		for (int x = 0; x < result.dim; x++)
		{
			result.tetro[x][y] = a.tetro[y][ry--];
		}
		ry = result.dim - 1;
	}
	return result;
}
// Return given STetro, counter-clockwise.
STetro ccw(STetro a)
{
	if (a.id == 6)
		return a;

	STetro result;
	charinfo_clear(*result.tetro, 4 * 4);
	result.x = a.x;
	result.y = a.y;
	result.dim = a.dim;

	uint8_t rx = result.dim - 1;
	for (int y = 0; y < result.dim; y++)
	{
		for (int x = 0; x < result.dim; x++)
		{
			result.tetro[x][y] = a.tetro[rx][x];
		}
		rx--;
	}
	return result;
}

#endif // Tetro_h