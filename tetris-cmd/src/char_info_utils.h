#pragma once
#ifndef char_info_utils_h
#define char_info_utils_h

#include "Windows.h"

// CHAR_INFO "constructor"
CHAR_INFO char_info(char c = ' ', WORD attr = 0)
{
	CHAR_INFO a;
	a.Char.AsciiChar = c;
	a.Attributes = attr;
	return a;
}

// CHAR_INFO array clearer
// (size = number of elements)
void charinfo_clear(CHAR_INFO *a, size_t size)
{
	for (int i = 0; i < size; i++)
	{
		a[i] = char_info();
	}
}

// CHAR_INFO array filler
// (size = number of elements)
void charinfo_fill(CHAR_INFO *a, size_t size, CHAR_INFO c)
{
	for (int i = 0; i < size; i++)
	{
		a[i] = c;
	}
}

// COORD "constructor"
COORD coord(int x = 0, int y = 0)
{
	COORD a;
	a.X = x;
	a.Y = y;
	return a;
}

//SMALL_RECT "constructor"
SMALL_RECT small_rect(int x0, int y0, int x1, int y1)
{
	SMALL_RECT a;
	a.Left = x0;
	a.Top = y0;
	a.Right = x1;
	a.Bottom = y1;
	return a;
}

#endif // char_info_utils_h