/**
  * Main game stuff. Drawing, controls, and more are done here.
  */

#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include "windows.h"

#include "char_info_utils.h"
#include "Tetro.h"
#include "Tetris.h"

using namespace std::chrono_literals;

const uint8_t CONSOLE_WIDTH = 120;
const uint8_t CONSOLE_HEIGHT = 30;
CHAR_INFO screen[CONSOLE_HEIGHT][CONSOLE_WIDTH]; // buffer to draw

SMALL_RECT srect = small_rect(0, 0, CONSOLE_WIDTH - 1, CONSOLE_HEIGHT - 1);

COORD playfield_origin = coord(28, 3); // top-left coord to start drawing playfield (INSIDE the HUD)
COORD size = coord(CONSOLE_WIDTH, CONSOLE_HEIGHT);

Tetris tetris;

/** RENDER FUNCTIONS (screen[y][x]) */
void put_hud()
{
	for (int y = -1; y < 21; y++)
	{
		if (y == -1)
		{
			for (int x = -1; x < 11; x++)
			{
				if (x <= 1 || x >= 8)
					screen[playfield_origin.Y + y][playfield_origin.X + x] = char_info(177, 0b111);
			}
		}
		else if (y == 20)
		{
			for (int x = -1; x < 11; x++)
			{
				screen[playfield_origin.Y + y][playfield_origin.X + x] = char_info(177, 0b111);
			}
		}
		else
		{
			screen[playfield_origin.Y + y][playfield_origin.X - 1] = char_info(177, 0b111);
			screen[playfield_origin.Y + y][playfield_origin.X + 10] = char_info(177, 0b111);
		}
	}
}
// Put playfield onto screen, its origin from COORD playfield_origin
void put_playfield()
{
	for (int y = 0; y <= 19; y++)
	{
		for (int x = 0; x <= 9; x++)
		{
			if (tetris.playfield[x + 4][y + 20].Char.AsciiChar != ' ' && tetris.playfield[x + 4][y + 20].Char.AsciiChar != 0)
				screen[playfield_origin.Y + y][playfield_origin.X + x] = tetris.playfield[x + 4][y + 20];
		}
	}
}
// Put activefield onto screen, its origin from COORD playfield_origin
void put_activefield()
{
	for (int y = 0; y <= 19; y++)
	{
		for (int x = 0; x <= 9; x++)
		{
			if (tetris.activefield[x + 4][y + 20].Char.AsciiChar != ' ' && tetris.activefield[x + 4][y + 20].Char.AsciiChar != 0)
			{
				screen[playfield_origin.Y + y][playfield_origin.X + x] = tetris.activefield[x + 4][y + 20];
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

// Puts a std::string into screen.
void put_string(int x, int y, std::string str)
{
	for (int i = 0; i < str.size(); i++)
	{
		screen[y][x + i] = char_info(str[i], 0b111);
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
			if(st.tetro[x][y].Char.AsciiChar != ' ')
				screen[y + sy][x + sx] = st.tetro[x][y];
		}
	}
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
	HANDLE hConsole = init_console();

	auto fr_start = std::chrono::high_resolution_clock::now();
	auto fr_end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> fr_duration;

	tetris.start();

	while (!tetris.has_lost())
	{
		charinfo_clear(*screen, CONSOLE_WIDTH*CONSOLE_HEIGHT);
		fr_start = std::chrono::high_resolution_clock::now();
		tetris.update();

		// RENDER //
		put_hud();
		put_activefield();
		put_playfield();

		put_string(0, 0, "FPS: " + std::to_string(1.0f / fr_duration.count()));

		put_string(0, 2, "Interval: " + std::to_string(tetris.get_interval()));
		put_string(0, 3, "can_down: " + std::to_string(tetris.can_down()));
		put_string(0, 4, "can_left: " + std::to_string(tetris.can_left()));
		put_string(0, 5, "can_right: " + std::to_string(tetris.can_right()));

		put_string(0, 10, "SActive.x = " + std::to_string(tetris.SActive.x));
		put_string(0, 11, "SActive.y = " + std::to_string(tetris.SActive.y));

		put_STetro(playfield_origin.X + 15, playfield_origin.Y, get_STetro(*Tetro::tetro[tetris.get_next()]));

		WriteConsoleOutput(hConsole, *screen, size, coord(0, 0), &srect);
		fr_end = std::chrono::high_resolution_clock::now();
		fr_duration = fr_end - fr_start;
	}
	CloseHandle(hConsole);

	std::cout << std::string(8, '\n');
	std::cout << "Your score was " << tetris.get_score() << '\n';
	std::cout << "Thanks for playing Tetris!\n";
	std::cout << "Made in Vietnam and USA.\n\n";
	std::cout << "clone by muskit\n2018\n\n";
	std::this_thread::sleep_for(3s);
}
