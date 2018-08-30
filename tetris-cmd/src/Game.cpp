/**
  * Drawing + debug elements.
  */

#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include "Windows.h"

#include "char_info_utils.h"
#include "Tetro.h"
#include "Tetris.h"

#define GAME_VERSION "v0.2.1"

using namespace std::chrono_literals;

const uint8_t CONSOLE_WIDTH = 75;
const uint8_t CONSOLE_HEIGHT = 33;
CHAR_INFO screen[CONSOLE_HEIGHT][CONSOLE_WIDTH]; // buffer to draw

SMALL_RECT srect = small_rect(0, 0, CONSOLE_WIDTH - 1, CONSOLE_HEIGHT - 1);

COORD playfield_origin = coord(30, 5); // top-left coord to start drawing playfield (INSIDE the HUD)
COORD size = coord(CONSOLE_WIDTH, CONSOLE_HEIGHT);

auto fr_start = std::chrono::high_resolution_clock::now();
auto fr_end = std::chrono::high_resolution_clock::now();
std::chrono::duration<float> fr_duration;

auto game_start = std::chrono::high_resolution_clock::now();
auto game_end = std::chrono::high_resolution_clock::now();

Tetris tetris;

/** RENDER FUNCTIONS (screen[y][x]) */

// Puts a std::string into screen.
void put_string(int x, int y, std::string str, WORD attr = 0b111)
{
	for (int i = 0; i < str.size(); i++)
	{
		screen[y][x + i] = char_info(str[i], attr);
	}
}
// Puts a CHAR_INFO into screen.
void put_charinfo(int x, int y, CHAR_INFO c = char_info(' ', 0))
{
	screen[y][x] = c;
}
// Put a STetro into screen.
void put_STetro(int sx, int sy, STetro st)
{
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			if (st.tetro[x][y].Char.AsciiChar != ' ')
				screen[y + sy][x + sx] = st.tetro[x][y];
		}
	}
}

void put_field(CHAR_INFO a[14][40])
{
	for (int y = 0; y <= 19; y++)
	{
		for (int x = 0; x <= 9; x++)
		{
			if (a[x + 4][y + 20].Char.AsciiChar != ' ' && a[x + 4][y + 20].Char.AsciiChar != 0)
			{
				screen[playfield_origin.Y + y][playfield_origin.X + x] = a[x + 4][y + 20];
			}
		}
	}
}

void put_background()
{
	for (int y = 0; y <= 19; y++)
	{
		for (int x = 0; x <= 9; x++)
		{
			screen[playfield_origin.Y + y][playfield_origin.X + x] = char_info('!', 0b11);
		}
	}
}
void put_hud()
{
	for (char y = -1; y < 21; y++)
	{
		if (y == -1) // TOP
		{
			put_string(playfield_origin.X - 1, playfield_origin.Y + y, "\xC9\xCD\xCD\xBC    \xC8\xCD\xCD\xBB");
		}
		else if (y == 20) // BOTTOM
		{
			put_string(playfield_origin.X - 1, playfield_origin.Y + y, "\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC");
		}
		else
		{
			put_string(playfield_origin.X - 1, playfield_origin.Y + y, "\xBA          \xBA");
		}
	}

	// NEXT PIECE
	put_string(playfield_origin.X + 12, playfield_origin.Y, "\xC9\xCD    \xCD\xBB");
	put_string(playfield_origin.X + 14, playfield_origin.Y, "NEXT");
	put_string(playfield_origin.X + 12, playfield_origin.Y + 1, "\xBA      \xBA");
	put_string(playfield_origin.X + 12, playfield_origin.Y + 2, "\xBA      \xBA");
	put_string(playfield_origin.X + 12, playfield_origin.Y + 3, "\xBA      \xBA");
	put_string(playfield_origin.X + 12, playfield_origin.Y + 4, "\xBA      \xBA");
	put_string(playfield_origin.X + 12, playfield_origin.Y + 5, "\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xBC");
	if(tetris.get_started())
		put_STetro(playfield_origin.X + 14, playfield_origin.Y + 2, Tetro::tetro[tetris.get_next()]);

	// HOLD PIECE
	put_string(playfield_origin.X - 11, playfield_origin.Y, "\xC9\xCD    \xCD\xBB");
	put_string(playfield_origin.X - 9, playfield_origin.Y, "HOLD");
	put_string(playfield_origin.X - 11, playfield_origin.Y+1, "\xBA      \xBA");
	put_string(playfield_origin.X - 11, playfield_origin.Y+2, "\xBA      \xBA");
	put_string(playfield_origin.X - 11, playfield_origin.Y+3, "\xBA      \xBA");
	put_string(playfield_origin.X - 11, playfield_origin.Y+4, "\xBA      \xBA");
	put_string(playfield_origin.X - 11, playfield_origin.Y+5, "\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xBC");
	if (tetris.get_hold() != -1)
	{
		put_STetro(playfield_origin.X - 9, playfield_origin.Y + 2, Tetro::tetro[tetris.get_hold()]);
	}

	put_string(playfield_origin.X + 12, playfield_origin.Y + 7, "\xB0\xB1\xB2\xDB     \xDB\xB2\xB1\xB0");
	put_string(playfield_origin.X + 16, playfield_origin.Y + 7, "LINES");
	put_string(playfield_origin.X + 15, playfield_origin.Y + 9, std::to_string(tetris.lines_achieved()) + " / " + std::to_string(tetris.get_line_nextlvl()) );

	put_string(playfield_origin.X + 12, playfield_origin.Y + 13, "\xB0\xB1\xB2\xDB     \xDB\xB2\xB1\xB0");
	put_string(playfield_origin.X + 16, playfield_origin.Y + 13, "LEVEL");
	put_string(playfield_origin.X + 18, playfield_origin.Y + 15, std::to_string(tetris.get_level()));

	put_string(playfield_origin.X - 6, playfield_origin.Y + 22, "F3: toggle debug info");
	put_string(playfield_origin.X - 7, playfield_origin.Y + 23, "ESC: quit (instantly!)");
}

void put_time()
{
	std::chrono::duration<float, std::milli> duration = std::chrono::high_resolution_clock::now() - game_start;

	uint8_t mins = duration.count() / 60000;
	uint64_t secs = duration.count() / 1000;
	uint64_t centi = duration.count() / 10;

	while(secs >= 60)
	{
		secs -= 60;
	}
	while (centi >= 100)
	{
		centi -= 100;
	}

	std::string s_secs = std::to_string(secs);
	std::string s_mins = std::to_string(mins);
	std::string s_ms = std::to_string(centi);

	if (secs < 10)
	{
		s_secs.insert(0, "0");
	}
	if (mins < 10)
	{
		s_mins.insert(0, "0");
	}
	while (s_ms.size() < 2)
	{
		s_ms.insert(0, "0");
	}

	put_string(playfield_origin.X + 16, playfield_origin.Y + 20, s_mins + ":" + s_secs + "." + s_ms);



}

void put_debug()
{
	// HUD EXTRAS
	put_string(playfield_origin.X - 8, playfield_origin.Y + 6, std::to_string(tetris.get_hold()));
	put_string(playfield_origin.X + 21, playfield_origin.Y + 2, std::to_string(tetris.get_next()));
	put_string(playfield_origin.X + 20, playfield_origin.Y + 3, "[" + std::to_string(tetris.get_bagindex()) + "]");
	put_string(playfield_origin.X + 4, playfield_origin.Y - 3, std::to_string(tetris.test));

	// SIDE INFO
	put_string(0, 0, "FPS: " + std::to_string(1.0f / fr_duration.count()));

	put_string(0, 2, "Interval: " + std::to_string(tetris.get_interval()));
	put_string(0, 3, "can_down: " + std::to_string(tetris.can_down()));
	put_string(0, 4, "can_left: " + std::to_string(tetris.can_left()));
	put_string(0, 5, "can_right: " + std::to_string(tetris.can_right()));

	put_string(0, 7, "can_cw: " + std::to_string(tetris.can_cw()));
	put_string(0, 8, "can_ccw: " + std::to_string(tetris.can_ccw()));

	put_string(0, 10, "SActive.x = " + std::to_string(tetris.SActive.x));
	put_string(0, 11, "SActive.y = " + std::to_string(tetris.SActive.y));
	put_string(0, 12, "rot: " + (std::to_string(tetris.SActive.rot)));

	put_string(0, 14, "left: " + (std::to_string((GetAsyncKeyState(VK_LEFT) & 32768) >> 15)));
	put_string(0, 15, "right: " + (std::to_string((GetAsyncKeyState(VK_RIGHT) & 32768) >> 15)));
	put_string(0, 16, "down: " + (std::to_string((GetAsyncKeyState(VK_DOWN) & 32768) >> 15)));

	put_string(0, 18, "allow_rot: " + (std::to_string(tetris.get_allowrot())));

	put_string(0, 21, "ghost.x: " + std::to_string(tetris.ghost.x));
	put_string(0, 22, "ghost.y: " + std::to_string(tetris.ghost.y));

	put_string(0, 24, "test_progress: " + std::to_string(tetris.test_progress));
}

//////////////////////////////////////////////////////////////////
// Only to be called at the start of main()
HANDLE init_console()
{
	charinfo_clear(*screen, CONSOLE_WIDTH*CONSOLE_HEIGHT);
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	return hConsole;
}

int main()
{
	// prepare console
	HANDLE hConsole = init_console();
	std::string timestamp = __DATE__;
	timestamp += ", ";
	timestamp += __TIME__;

	std::string __title = "tetris-cmd ";
	__title += GAME_VERSION;
	SetConsoleTitle(__title.c_str());

	bool started = false;
	uint8_t level = 1;
	while (!started)
	{
		if ((GetAsyncKeyState(VK_RIGHT) & 32768 >> 15))
		{
			if (level < 20)
				level++;
			else
				level = 1;
		}
		if ((GetAsyncKeyState(VK_LEFT) & 32768 >> 15))
		{
			if (level <= 1)
				level = 20;
			else
				level--;
		}
		put_string(0, 0, "tetris-cmd ", 0b111 | 0x4000);
		put_string(11, 0, GAME_VERSION, 0b111 | 0x4000);
		put_string(0, 1, "Build timestamp:");
		put_string(0, 2, timestamp);

		put_string(0, 4, "LEVEL: ");
		put_string(7, 4, std::to_string(level), 0b111 | 0x4000);
		put_string(11, 4, "<- use L/R ARW keys");

		put_string(0, 6, "Press [SPACE] to start");

		put_string(0, 9, "For best experience, set this console font to");
		put_string(0, 10, "Raster Graphics (12x16) by right-clicking on the");
		put_string(0, 11, "window title bar, then going to Properties.");
		put_string(0, 13, "(Or at least get a square font!)");

		WriteConsoleOutput(hConsole, *screen, size, coord(0, 0), &srect);
		started = ((GetAsyncKeyState(VK_SPACE) & 32768 >> 15));
		charinfo_clear(*screen, CONSOLE_WIDTH*CONSOLE_HEIGHT);

		std::this_thread::sleep_for(1ms);
	}

	tetris = Tetris(level);

	charinfo_clear(*screen, CONSOLE_WIDTH*CONSOLE_HEIGHT);

	put_hud();
	put_string(playfield_origin.X + 2, playfield_origin.Y + 5, "READY");
	WriteConsoleOutput(hConsole, *screen, size, coord(0, 0), &srect);

	charinfo_clear(*screen, CONSOLE_WIDTH*CONSOLE_HEIGHT);
	put_hud();
	put_string(playfield_origin.X + 3, playfield_origin.Y + 5, "GO!");
	std::this_thread::sleep_for(833ms);
	
	WriteConsoleOutput(hConsole, *screen, size, coord(0, 0), &srect);
	std::this_thread::sleep_for(833ms);

	tetris.start();
	game_start = std::chrono::high_resolution_clock::now();
	while (!tetris.has_lost())
	{
		fr_start = std::chrono::high_resolution_clock::now();

		charinfo_clear(*screen, CONSOLE_WIDTH*CONSOLE_HEIGHT);

		tetris.update();

		// RENDER //
		put_hud();
		put_field(tetris.ghostfield);
		put_field(tetris.playfield);
		put_field(tetris.activefield);
		put_time();
		if (tetris.show_debug)
			put_debug();

		WriteConsoleOutput(hConsole, *screen, size, coord(0, 0), &srect);

		fr_end = std::chrono::high_resolution_clock::now();
		fr_duration = fr_end - fr_start;
	}

	// end
	put_hud();
	put_field(tetris.ghostfield);
	put_field(tetris.playfield);
	put_field(tetris.activefield);
	put_string(playfield_origin.X, playfield_origin.Y + 5, "GAME OVER!");
	WriteConsoleOutput(hConsole, *screen, size, coord(0, 0), &srect);

	while (!((GetAsyncKeyState(VK_RETURN) & 32768 >> 15))) { std::this_thread::sleep_for(1ms); }
}
