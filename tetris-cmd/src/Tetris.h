#pragma once
#ifndef Tetris_h
#define Tetris_h

/**
  * The Tetris engine. In main(), have Tetris.update() run every loop.
  * Excuse the Lasagna Code :)
  */

#include "Windows.h"
#include "Tetro.h"

using namespace std::chrono_literals;

class Tetris
{
private:
	bool started = false;
	bool lost = false;
	uint64_t score = 0;
	uint8_t level = 1;

	uint32_t lines = 0; // lines scored
	uint32_t lines_nextlvl = lines + 10;

	// bag system (will rework)
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
	bool hldDebug = false;

	// TETROMINO TIMING
	int64_t interval = 1000; // how long it takes (ms) until tetromino must go down, will change with difficulty
	std::chrono::time_point<std::chrono::steady_clock> down_time;
	std::chrono::time_point<std::chrono::steady_clock> spawn_time; // ARE(lock->spawn)

	// DAS TIMING
	uint16_t autoshift_delay_ms = 225;
	uint16_t autoshift_time_ms = 40;
	std::chrono::time_point<std::chrono::steady_clock> autoshift_delay; // delay until we start auto shifting
	std::chrono::time_point<std::chrono::steady_clock> autoshift_time; // interval between each movement attempt
	
	// general purpose duration object
	std::chrono::duration<float, std::milli> duration;
	
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
	// Put the SActive onto ghost
	void SActive_ghostfield()
	{
		// remove trail
		for (int y = 0; y < 40; y++)
		{
			for (int x = 0; x < 14; x++)
			{
				ghostfield[x][y] = char_info();
			}
		}

		memcpy(&ghost, &SActive, sizeof(STetro));

		while (can_down(ghost))
			ghost.y++;



		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				// recolour and put
				if (ghost.tetro[x][y].Char.AsciiChar != 0 && ghost.tetro[x][y].Char.AsciiChar != ' ')
				{
					ghost.tetro[x][y].Attributes = 0b111;
					ghost.tetro[x][y].Char.AsciiChar = 176;
					ghostfield[ghost.x + x][ghost.y + y] = ghost.tetro[x][y];
				}
			}
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

	// process controls during active mode (move? hold? etc.)
	void controls()
	{
		// MOVE //
		if ((GetAsyncKeyState(VK_LEFT) & 32768) >> 15) //left
		{
			if (can_left())
			{
				if (!hldL && active)
				{
					autoshift_delay = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(120);
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
				if (!hldR && active)
				{
					autoshift_delay = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(autoshift_delay_ms);
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
		if ((GetAsyncKeyState(0x58) & 32768) >> 15 && active) // CW, [X]
		{
			if (!hldCW)
			{
				if (allow_rot)
				{
					moved = cw_super();
				}
				hldCW = true;

			}
		}
		else
		{
			hldCW = false;
		}
		if ((GetAsyncKeyState(0x5A) & 32768) >> 15 && active) // CCW, [Z]
		{
			if (!hldCCW)
			{
				if (allow_rot)
				{
					moved = ccw_super();
				}
				hldCCW = true;
			}
		}
		else
		{
			hldCCW = false;
		}

		// DROPS //
		if ((GetAsyncKeyState(VK_SPACE) & 32768) >> 15) // hard, [SPACE]
		{
			if (active)
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
		}
		else
			hldHard = false;
		if ((GetAsyncKeyState(VK_DOWN) & 32768) >> 15 && active) // soft, [DOWN ARROW]
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
						down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(interval / 20);
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

		misc_controls();
		if (hldL || hldR)
			autoshift(); // DAS (delayed autoshift)
	}
	// controls available at any time
	void misc_controls()
	{
		if (GetAsyncKeyState(VK_ESCAPE) & 32768)
		{
			// TODO: make pause instead of quit
			lost = true;
		}

		if ((GetAsyncKeyState(VK_F3) & 32768) >> 15) // debug, [F3]
		{
			if (!hldDebug)
			{
				hldDebug = true;
				show_debug = !show_debug;

			}
		}
		else
			hldDebug = false;
	}

	// one of the two movement buttons is currently held
	void autoshift()
	{
		duration = autoshift_delay - std::chrono::high_resolution_clock::now();

		if (duration.count() <= 0)
		{
			duration = autoshift_time - std::chrono::high_resolution_clock::now();
			if (duration.count() <= 0)
			{
				if (hldR && can_right())
				{
					SActive.x++;
					moved = true;
				}
				else if (hldL && can_left())
				{
					SActive.x--;
					moved = true;
				}
				autoshift_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(autoshift_time_ms);
			}

		}
	}
	
	// check against the playfield if given STetro is in a good spot, in that
	//    1. is not in an occupying spot
	//    2. is not out of bounds
	bool is_valid(STetro &test)
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				// don't check blank spots
				if (test.tetro[x][y].Char.AsciiChar != ' ')
				{
					// check against the playfield
					if ((playfield[test.x + x][test.y + y].Char.AsciiChar != ' ') || ((test.x + x > 13 || test.x + x < 4) || test.y + y > 39 ))
						return false;
				}
			}
		}
		return true;
	}

	// clock-wise SRS
	bool cw_super()
	{
		if (SActive.id == 6)
			return true;

		STetro test;
		memcpy(&test, &SActive, sizeof(STetro));
		test.cw();

		// (0,0; test 1)
		if (is_valid(test))
		{
			memcpy(&SActive, &test, sizeof(STetro));
			test_progress = 1;
			return true;
		}

		// I test
		if (test.id == 0)
		{
			/** Start: 0 */
			if (SActive.rot == 0)
			{
				// TEST 2
				test.x = SActive.x - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 2;
					return true;
				}
				// TEST 3
				test.x = SActive.x + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 3;
					return true;
				}
				// TEST 4
				test.x = SActive.x - 2;
				test.y = SActive.y + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 4;
					return true;
				}
				// TEST 5
				test.x = SActive.x + 1;
				test.y = SActive.y - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 5;
					return true;
				}
			}
			/** Start: 1 */
			if (SActive.rot == 1)
			{
				// TEST 2
				test.x = SActive.x - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 3
				test.x = SActive.x + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 4
				test.x = SActive.x - 1;
				test.y = SActive.y - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 5
				test.x = SActive.x + 2;
				test.y = SActive.y + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
			}
			/** Start: 2 */
			if (SActive.rot == 2)
			{
				// TEST 2
				test.x = SActive.x + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 3
				test.x = SActive.x - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 4
				test.x = SActive.x + 2;
				test.y = SActive.y - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 5
				test.x = SActive.x - 1;
				test.y = SActive.y + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
			}
			/** Start: 3 */
			if (SActive.rot == 3)
			{
				// TEST 2
				test.x = SActive.x + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 3
				test.x = SActive.x - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 4
				test.x = SActive.x + 1;
				test.y = SActive.y + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 5
				test.x = SActive.x - 2;
				test.y = SActive.y - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
			}
		}
		// Everything else (excluding 'O')
		else
		{
			/** Start: 0 */
			if (SActive.rot == 0)
			{
				// TEST 2
				test.x = SActive.x - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 2;
					return true;
				}
				// TEST 3
				test.x = SActive.x - 1;
				test.y = SActive.y - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 3;
					return true;
				}
				// TEST 4
				test.x = SActive.x;
				test.y = SActive.y + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 4;
					return true;
				}
				// TEST 5
				test.x = SActive.x - 1;
				test.y = SActive.y + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 5;
					return true;
				}
			}
			/** Start: 1 */
			if (SActive.rot == 1)
			{
				// TEST 2
				test.x = SActive.x + 1;
				test.y = SActive.y;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 3
				test.x = SActive.x + 1;
				test.y = SActive.y + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 4
				test.x = SActive.x;
				test.y = SActive.y - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 5
				test.x = SActive.x + 1;
				test.y = SActive.y - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
			}
			/** Start: 2 */
			if (SActive.rot == 2)
			{
				// TEST 2
				test.x = SActive.x + 1;
				test.y = SActive.y;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 3
				test.x = SActive.x + 1;
				test.y = SActive.y - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 4
				test.x = SActive.x;
				test.y = SActive.y + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 5
				test.x = SActive.x + 1;
				test.y = SActive.y + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
			}
			/** Start: 3 */
			if (SActive.rot == 3)
			{
				// TEST 2
				test.x = SActive.x - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 3
				test.x = SActive.x - 1;
				test.y = SActive.y + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 4
				test.x = SActive.x;
				test.y = SActive.y - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 5
				test.x = SActive.x - 1;
				test.y = SActive.y - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
			}
		}
		return false;
	}

	// counter-clockwise SRS
	bool ccw_super()
	{
		STetro test;
		memcpy(&test, &SActive, sizeof(STetro));
		test.ccw();

		if (SActive.id == 6)
			return true;

		// (0,0; test 1)
		if (is_valid(test))
		{
			memcpy(&SActive, &test, sizeof(STetro));
			test_progress = 1;
			return true;
		}

		// I test
		if (test.id == 0)
		{
			/** Start: 0 */
			if (SActive.rot == 1)
			{
				// TEST 2
				test.x = SActive.x + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 2;
					return true;
				}
				// TEST 3
				test.x = SActive.x - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 3;
					return true;
				}
				// TEST 4
				test.x = SActive.x + 2;
				test.y = SActive.y - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 4;
					return true;
				}
				// TEST 5
				test.x = SActive.x - 1;
				test.y = SActive.y + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 5;
					return true;
				}
			}
			/** Start: 2 */
			if (SActive.rot == 2)
			{
				// TEST 2
				test.x = SActive.x + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 3
				test.x = SActive.x - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 4
				test.x = SActive.x + 1;
				test.y = SActive.y + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 5
				test.x = SActive.x - 2;
				test.y = SActive.y - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
			}
			/** Start: 3 */
			if (SActive.rot == 3)
			{
				// TEST 2
				test.x = SActive.x - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 3
				test.x = SActive.x + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 4
				test.x = SActive.x - 2;
				test.y = SActive.y + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 5
				test.x = SActive.x + 1;
				test.y = SActive.y - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
			}
			/** Start: 0 */
			if (SActive.rot == 0)
			{
				// TEST 2
				test.x = SActive.x - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 3
				test.x = SActive.x + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 4
				test.x = SActive.x - 1;
				test.y = SActive.y - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 5
				test.x = SActive.x + 2;
				test.y = SActive.y + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
			}
		}
		// Everything else (excluding 'O')
		else
		{
			/** Start: 1 */
			if (SActive.rot == 1)
			{
				// TEST 2
				test.x = SActive.x + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 2;
					return true;
				}
				// TEST 3
				test.x = SActive.x + 1;
				test.y = SActive.y + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 3;
					return true;
				}
				// TEST 4
				test.x = SActive.x;
				test.y = SActive.y - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 4;
					return true;
				}
				// TEST 5
				test.x = SActive.x + 1;
				test.y = SActive.y - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					test_progress = 5;
					return true;
				}
			}
			/** Start: 2 */
			if (SActive.rot == 2)
			{
				// TEST 2
				test.x = SActive.x - 1;
				test.y = SActive.y;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 3
				test.x = SActive.x - 1;
				test.y = SActive.y - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 4
				test.x = SActive.x;
				test.y = SActive.y + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 5
				test.x = SActive.x - 1;
				test.y = SActive.y + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
			}
			/** Start: 3 */
			if (SActive.rot == 3)
			{
				// TEST 2
				test.x = SActive.x - 1;
				test.y = SActive.y;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 3
				test.x = SActive.x - 1;
				test.y = SActive.y + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 4
				test.x = SActive.x;
				test.y = SActive.y - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 5
				test.x = SActive.x - 1;
				test.y = SActive.y - 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
			}
			/** Start: 0 */
			if (SActive.rot == 0)
			{
				// TEST 2
				test.x = SActive.x + 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 3
				test.x = SActive.x + 1;
				test.y = SActive.y - 1;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 4
				test.x = SActive.x;
				test.y = SActive.y + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
				// TEST 5
				test.x = SActive.x + 1;
				test.y = SActive.y + 2;
				if (is_valid(test))
				{
					memcpy(&SActive, &test, sizeof(STetro));
					return true;
				}
			}
		}
		return false;
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

	// ghost piece
	STetro ghost;

	char test_progress = -1;

	bool show_debug = false;

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
	// check if STetro in arg. can go down
	bool can_down(STetro &st)
	{
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				// don't check blank spots
				if (st.tetro[x][y].Char.AsciiChar != ' ')
				{
					// check against the playfield
					if ((playfield[st.x + x][st.y + y + 1].Char.AsciiChar != ' ') || (st.y + y == 39))
						return false;
				}
			}
		}
		return true;
	}

	bool can_cw()
	{
		STetro test = Tetro::cw(SActive);
		
		return is_valid(test);
	}
	bool can_ccw()
	{
		STetro test = Tetro::ccw(SActive);

		return is_valid(test);
	}

	// based on SActive's last coordinates, return cleared lines.
	// ARRAY SIZE: 4 bytes
	uint8_t *get_clears()
	{
		uint8_t lines[4] = { 0,0,0,0 };

		// occupy lines[]
		for (int y = 0; y < 4; y++)
		{
			// avoid stack overflow
			if (SActive.y + y > 39)
				break;

			uint8_t minos = 0; // the field is 10 accross
			for (int x = 4; x <= 13; x++)
			{
				if (playfield[x][SActive.y + y].Char.AsciiChar != ' ')
				{
					++minos;
				}
			}
			if (minos == 10)
				lines[y] = SActive.y + y;
		}
		return lines;
	}

	// check playfield for lines, update and score as needed
	void playfield_check()
	{
		const uint8_t *lines = get_clears();

		for (int i = 0; i < 4; i++)
		{
			if (lines[i] != 0)
			{
				this->lines++;
				for (int x = 4; x <= 13; x++)
				{
					playfield[x][lines[i]] = char_info();
				}
			}
		}
		for (int i = 0; i < 4; i++)
		{
			if (lines[i] != 0)
			{
				for (int y = lines[i]; y >= 15; y--)
				{
					for (int x = 4; x <= 13; x++)
					{
						playfield[x][y] = playfield[x][y - 1];
					}
				}
			}
		}
	}

	/** GETS */
	uint64_t get_score() { return score; }
	float get_interval() { return interval; }
	bool has_lost() { return lost; }
	uint8_t get_next() { return next; }
	char get_hold() { return hold;  }
	bool get_allowrot() { return allow_rot; }
	uint64_t lines_achieved() { return lines; }
	uint8_t get_level() { return level; }
	uint32_t get_line_nextlvl() { return lines_nextlvl; }

	void start()
	{
		autoshift_time = std::chrono::high_resolution_clock::now();

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

		if (lost)
			return;

		if (bag_index > 6)
		{
			shuffle_bag();
			bag_index = 0;
		}

		bool can_down_previously = can_down();

		controls();

		duration = std::chrono::high_resolution_clock::now() - down_time;
		if (active)
		{
			if (active)
			{
				if (moved) //only runs after player adjusted
				{
					moved = false;
					if (!can_down())
						down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(500);
					if (can_down() && !can_down_previously)
						down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(interval);
				}

				if (duration.count() >= 0) // time to down?
				{
					if (can_down()) // we can move down, so move down!
					{
						SActive.y++;
						if (!can_down())
							down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(500);
						else
							down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(interval);
					}
					else // cannot move down, LOCK!
					{
						SActive_playfield();
						playfield_check();
						charinfo_clear(*activefield, 14 * 40);
						SActive.id = -1;
						active = false;
						allow_swap = true;
						allow_rot = false;

						if (lines >= lines_nextlvl)
						{
							++level;
							lines_nextlvl += 10;
						}
					}
				}
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

			allow_rot = true;

			interval = get_interval(level);
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
				if (!can_down())
					down_time = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(500);
			}

			active = true;
		}
		if (active)
		{
			SActive_activefield();
			SActive_ghostfield();
		}
	}

	Tetris(int alevel)
	{
		level = alevel;
	}
	Tetris() {}

};

#endif // Tetris_h