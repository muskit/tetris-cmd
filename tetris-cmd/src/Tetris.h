#pragma once
#ifndef Tetris_h
#define Tetris_h

/**
  * Utility/game logic. update() will act as the "main game loop."
  */

#include "Windows.h"
#include "Tetro.h"
#include <vector>



class Tetris
{
private:
	bool lost = false;
	uint64_t score = 0;
	uint8_t hold; //tetro[hold] value of held piece
	uint8_t next; //next piece, to preview
	bool active = false;
	std::vector<uint8_t> bag; // 7 tetrominos. bag system used for better distribution

	// TIMING
	uint8_t interval = 1000; // how long it takes (ms) until tetromino must go down, will change with difficulty
	std::chrono::time_point<std::chrono::steady_clock> down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(interval);
	std::chrono::duration<float, std::milli> down_elapsed; // for test to see if it's time to down

	// Put the SActive onto activefield
	void SActive_activefield()
	{
		// CLEAR activefield
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
				if (SActive.tetro[x][y].Char.AsciiChar != 0 && SActive.tetro[x][y].Char.AsciiChar != ' ')
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
		return true;
	}

	/** GETS */
	uint64_t get_score() { return score; }
	float get_interval() { return interval; }
	bool has_lost() { return lost; }

	Tetris()
	{
		// seed
		srand(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}

	void update()
	{
		//if (bag.empty()) // refill bag with values 0-6
		//{

		//}
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
			}
			SActive_activefield();
		}
		if (!active)
		{
			SActive = get_STetro(*Tetro::tetro[rand() % 7]);
			SActive_activefield();
			active = true;
		}
	}
};

#endif // Tetris_h