#pragma once
#ifndef Tetro_h
#define Tetro_h

#include "Windows.h"
#include "char_info_utils.h"

typedef struct _STetro
{
	CHAR_INFO tetro[4][4];

	// starting position on field
	uint8_t x = 7;
	uint8_t y = 20;
} STetro;

namespace Tetro
{
	const CHAR_INFO I[4][4] = { { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(178, 0b11), char_info(178, 0b11), char_info(178, 0b11), char_info(178, 0b11) },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
						        { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO L[4][4] = { { char_info(' '), char_info(' '), char_info(178, 0b110), char_info(' ') },
	                            { char_info(178, 0b110), char_info(178, 0b110), char_info(178, 0b110), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO J[4][4] = { { char_info(178,1), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(178,1), char_info(178,1), char_info(178,1), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO T[4][4] = { { char_info(' '), char_info(178, 0b101), char_info(' '), char_info(' ') },
	                            { char_info(178, 0b101), char_info(178, 0b101), char_info(178, 0b101), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO S[4][4] = { { char_info(' '), char_info(178, 0b10), char_info(178, 0b10), char_info(' ') },
	                            { char_info(178, 0b10), char_info(178, 0b10), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO Z[4][4] = { { char_info(178, 0b100), char_info(178, 0b100), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(178, 0b100), char_info(178, 0b100), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO O[4][4] = { { char_info(' '), char_info(178, 0b1110), char_info(178, 0b1110), char_info(' ') },
	                            { char_info(' '), char_info(178, 0b1110), char_info(178, 0b1110), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') },
	                            { char_info(' '), char_info(' '), char_info(' '), char_info(' ') } };

	const CHAR_INFO(*tetro[7])[4][4] = { &I, &L, &J, &T, &S, &Z, &O };
	
	COORD size = coord(4, 4);
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

#endif // Tetro_h