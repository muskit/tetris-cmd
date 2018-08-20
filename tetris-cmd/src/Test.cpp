/**
  * Test console buffer output
  */

#include <string>
#include <thread>
#include "Tetro.h"
#include "char_info_utils.h"
#include "Windows.h"

using namespace std::chrono_literals; // allows 10s, 4ms, 20us, etc.

int main()
{
	//setup console
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	
	COORD size = coord(2,2);

	COORD origin = coord();

	CHAR_INFO a[11][11];

	SMALL_RECT rect;
	rect.Left = 0;
	rect.Top = 0;
	rect.Bottom = 9999;
	rect.Right = 119;

	WriteConsoleOutput(hConsole, *a, coord(5,5), origin, &rect);

	std::this_thread::sleep_for(1h);

}