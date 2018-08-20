#pragma once
#ifndef Tetris_h
#define Tetris_h

/**
  * Utility/game logic. update() will act as the "main game loop."
  */

#include "Windows.h"
#include "Tetro.h"
#include <vector>

using namespace std::chrono_literals;

class Tetris
{
private:
	bool lost = false;
	uint64_t score = 0;

	uint8_t hold; //tetro[hold] value of held piece
	uint8_t next; //next piece, to preview
	uint8_t bag[7] = { 0,1,2,3,4,5,6 };
	uint8_t bag_index = 0;

	bool active = false;

	// TIMING
	int64_t interval = 250; // how long it takes (ms) until tetromino must go down, will change with difficulty

	std::chrono::time_point<std::chrono::steady_clock> down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(interval);
	
	// Put the SActive onto activefield
	void SActive_activefield()
	{
		// remove trail
		for (int y = 0; y < 40; y++)
		{
			for (int x = 0; x < 14; x++)
			{
				activefield[x][y] = char_info();
			}
		}
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				if (SActive.tetro[x][y].Char.AsciiChar != ' ')
					activefield[SActive.x + x][SActive.y + y] = SActive.tetro[x][y];
			}
		}
	}
	// Put the SActive onto playfield
	void SActive_playfield()
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				if (SActive.tetro[x][y].Char.AsciiChar != 0 && SActive.tetro[x][y].Char.AsciiChar != ' ')
					playfield[SActive.x + x][SActive.y + y] = SActive.tetro[x][y];
			}
		}
	}

public:
	// 14x40 playfield; player only sees 10x20
	// x: 4-13 inclusive
	// y: 20-39 inclusive
	// (top 20 rows hidden): playfield[x][20] is where top is visible
	// all set blocks go here
	CHAR_INFO playfield[14][40];

	// player's moving blocks go here
	CHAR_INFO activefield[14][40];

	// player's active piece
	STetro SActive;

	std::chrono::duration<float, std::milli> down_elapsed; // for test to see if it's time to down

	bool can_right()
	{
		return true;
	}

	bool can_left()
	{
		return true;
	}
	bool can_down()
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				// don't check blank spots
				if (SActive.tetro[x][y].Char.AsciiChar != ' ')
				{
					// check against the playfield
					if ((playfield[SActive.x + x][SActive.y + y + 1].Char.AsciiChar != ' ') || (SActive.y + y == 39))
						return false;
				}
			}
		}
		return true;
	}

	/** GETS */
	uint64_t get_score() { return score; }
	float get_interval() { return interval; }
	bool has_lost() { return lost; }
	uint8_t get_next() { return next; }

	// shuffle tetrominos bag array
	void inline shuffle_bag()
	{
		for (size_t i = 0; i < 6; i++)
		{
			size_t j = i + rand() / (RAND_MAX / (7 - i) + 1);
			int t = bag[j];
			bag[j] = bag[i];
			bag[i] = t;
		}
		bag_index = 0;
	}

	void update()
	{
		if (bag_index >= 6)
			shuffle_bag();

		if (active)
		{
			down_elapsed = std::chrono::high_resolution_clock::now() - down_time;
			if (down_elapsed.count() >= 0) // time to down?
			{
				down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(interval);
				if (can_down())
				{
					SActive.y++;
				}
				else //cannot move down, LOCK!
				{
					SActive_playfield();
					charinfo_clear(*activefield, 14 * 40);
					active = false;
				}
			}
			SActive_activefield();
		}
		else
		{
			SActive = get_STetro(*Tetro::tetro[next]);
			next = bag_index++;
			if (!can_down()) // did we lose?
			{
				lost = true;
				return;
			}
			else
			{
				SActive.y++;
				if (can_down()) // room to move down one more?
				{
					SActive.y++;
				}
				else { }
			}


			SActive_activefield();
			active = true;
		}
	}

	Tetris()
	{
		// seed
		srand(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

		// clear fields
		charinfo_clear(*playfield, 14 * 40);
		charinfo_clear(*activefield, 14 * 40);

		shuffle_bag();

		next = bag[bag_index++];
	}
};

#endif // Tetris_h