// Player's settings

#pragma once
#ifndef Settings_h
#define Settings_h

#include <cstdint>
#include "windows.h"

class Settings
{
public:
	uint16_t start_level;

	uint8_t shift_delay; // when holding L/R, how long until we start repeating movement
	uint8_t shiftrate; // delay between shifts while holding L/R

};

#endif