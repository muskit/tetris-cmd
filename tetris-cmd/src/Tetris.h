#pragma once
#ifndef Tetris_h
#define Tetris_h

/**
  * The Tetris engine. In main(), have Tetris.update() run every loop.
  */

#include "Windows.h"
#include "Tetro.h"
#include <vector>

using namespace std::chrono_literals;

class Tetris
{
private:
	bool started = false;
	bool lost = false;
	uint64_t score = 0;
	uint16_t level = 1;

	uint8_t next; //next piece, to preview
	uint8_t bag[7] = { 0,1,2,3,4,5,6 };
	uint8_t bag_index = 0;

	char hold = -1; //tetro[hold] value of held piece
	bool active = false;
	bool moved = false;

	bool allow_swap = true;
	bool allow_rot = true;

	// input controls
	bool hldR = false;
	bool hldL = false;
	bool hldCW = false;
	bool hldCCW = false;
	bool hldHard = false;
	bool hldHold = false;
	bool hldSoft = false;
	bool hldPause = false;

	

	// TIMING
	int64_t interval = 1000; // how long it takes (ms) until tetromino must go down, will change with difficulty
	std::chrono::time_point<std::chrono::steady_clock> down_time;
	std::chrono::time_point<std::chrono::steady_clock> spawn_time;
	std::chrono::duration<float, std::milli> duration; // general purpose duration object
	
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

	// Check playfield for any lines. 
	void playfield_check()
	{

		for (int y = 39; y >= 20; y--)
		{

		}
	}

	// shuffle tetrominos bag array
	void shuffle_bag()
	{
		int i, j, temp;
		for (i = 6; i > 0; i--)
		{
			j = rand() % (i + 1);
			temp = bag[i];
			bag[i] = bag[j];
			bag[j] = temp;
		}
	}

	// process controls during active mode (move blocks? hold? etc.)
	void active_controls()
	{
		// MOVE //
		if ((GetAsyncKeyState(VK_LEFT) & 32768) >> 15) //left
		{
			if (can_left())
			{
				if (!hldL)
				{
					hldL = true;
					SActive.x--;
					moved = true;
				}
			}
		}
		else
		{
			hldL = false;
		}
		if ((GetAsyncKeyState(VK_RIGHT) & 32768) >> 15) // right
		{
			if (can_right())
			{
				if (!hldR)
				{
					hldR = true;
					SActive.x++;
					moved = true;
				}
			}
		}
		else
		{
			hldR = false;
		}

		// ROTATION //
		if ((GetAsyncKeyState(0x58) & 32768) >> 15) // CW, [X]
		{
			if (can_cw())
			{
				if (!hldCW)
				{
					if (allow_rot)
					{
						SActive = cw(SActive);
						moved = true;
					}
					hldCW = true;

				}
			}
		}
		else
		{
			hldCW = false;
		}
		if ((GetAsyncKeyState(0x5A) & 32768) >> 15) // CCW, [Z]
		{
			if (can_ccw())
			{
				if (!hldCCW)
				{
					if (allow_rot)
					{
						SActive = ccw(SActive);
						moved = true;
					}
					hldCCW = true;
				}
			}
		}
		else
		{
			hldCCW = false;
		}

		// DROPS //
		if ((GetAsyncKeyState(VK_SPACE) & 32768) >> 15) // hard, [SPACE]
		{
			if (!hldHard)
			{
				hldHard = true;
				while (can_down())
					SActive.y++;
				down_time = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(100);
				allow_rot = false;
			}
		}
		else
			hldHard = false;
		if ((GetAsyncKeyState(VK_DOWN) & 32768) >> 15) // soft, [DOWN ARROW]
		{
			if (!hldSoft)
			{
				if (can_down())
				{
					SActive.y++;
					moved = true;
					if (can_down())
					{
						interval = interval / 20;
						down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(interval);
					}
					hldSoft = true;
				}
			}
		}
		else
		{
			if (hldSoft)
			{
				interval = get_interval(level);
				if(can_down())
					down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(interval);
			}
			hldSoft = false;
		}

		if ((GetAsyncKeyState(0x43) & 32768) >> 15) // hold/swap, [C]
		{
			if (!hldHold && allow_swap)
			{
				hldHold = true;
				allow_swap = false;
				active = false;

				char temp = 0;
				temp = SActive.id;
				SActive.id = hold;
				hold = temp;
			}
		}
		else
			hldHold = false;
	}
	// controls available at any time
	void passive_controls()
	{
		if (GetAsyncKeyState(VK_ESCAPE) & 32768)
		{
			// TODO: make pause instead of quit
			lost = true;
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

	// show ghost piece
	CHAR_INFO ghostfield[14][40];

	// player's active piece
	STetro SActive;

	// return a drop interval in milliseconds, given level
	double get_interval (uint16_t a)
	{
		return pow((0.8 - ((a - 1) * 0.007)), a - 1)*1000;
	}

	bool can_right()
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				// don't check blank spots
				if (SActive.tetro[x][y].Char.AsciiChar != ' ')
				{
					// check against the playfield
					if ((playfield[SActive.x + x + 1][SActive.y + y].Char.AsciiChar != ' ') || (SActive.x + x == 13))
						return false;
				}
			}
		}
		return true;
	}

	bool can_left()
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				// don't check blank spots
				if (SActive.tetro[x][y].Char.AsciiChar != ' ')
				{
					// check against the playfield
					if ((playfield[SActive.x + x - 1][SActive.y + y].Char.AsciiChar != ' ') || (SActive.x + x == 4))
						return false;
				}
			}
		}
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
	bool can_cw()
	{
		STetro test = cw(SActive);
		
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				// don't check blank spots
				if (test.tetro[x][y].Char.AsciiChar != ' ')
				{
					// check against the playfield
					if ((playfield[test.x + x][test.y + y].Char.AsciiChar != ' ') || ( (test.x+x > 13 || test.x+x < 4) || ( test.y+y > 39 || test.y+y < 20 ) ))
						return false;
				}
			}
		}
		return true;
	}
	bool can_ccw()
	{
		STetro test = ccw(SActive);

		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				// don't check blank spots
				if (test.tetro[x][y].Char.AsciiChar != ' ')
				{
					// check against the playfield
					if ((playfield[test.x + x][test.y + y].Char.AsciiChar != ' ') || ((test.x + x > 13 || test.x + x < 4) || (test.y + y > 39 || test.y + y < 20)))
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
	char get_hold() { return hold;  }
	bool get_allowrot() { return allow_rot; }

	void start()
	{
		// seed
		srand(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

		// clear fields
		charinfo_clear(*playfield, 14 * 40);
		charinfo_clear(*activefield, 14 * 40);
		charinfo_clear(*ghostfield, 14 * 40);

		shuffle_bag();
		next = bag[bag_index++];
		SActive.id = -1;

		interval = get_interval(level);

		started = true;
	}

	void update()
	{
		if (!started)
			return;

		passive_controls();
		if (lost)
			return;

		if (bag_index > 6)
		{
			shuffle_bag();
			bag_index = 0;
		}

		duration = std::chrono::high_resolution_clock::now() - down_time;
		if (active)
		{
			bool can_down_previously = can_down();

			active_controls();

			if (active)
			{
				if (moved) //only runs after player moved
				{
					moved = false;
					if (!can_down())
						down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(500);
					if (can_down() && !can_down_previously)
						down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(interval);
				}

				if (duration.count() >= 0) // time to down?
				{
					if (can_down())
					{
						SActive.y++;
						if (!can_down())
							down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(500);
						else
							down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(interval);
					}
					else //cannot move down, LOCK!
					{
						SActive_playfield();
						playfield_check();
						charinfo_clear(*activefield, 14 * 40);
						SActive.id = -1;
						active = false;
						allow_swap = true;
						allow_rot = true;
					}
				}
				SActive_activefield();
			}
		}
		else
		{
			if (SActive.id == -1)
			{
				SActive = Tetro::tetro[next];
				next = bag[bag_index++];
			}
			else
			{
				SActive = Tetro::tetro[SActive.id];
			}

			down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(interval);
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
};

#endif // Tetris_h