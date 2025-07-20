#pragma once

#include <string>

#define GAME_UNITS_IN_METER 51.2f;

#define PI 3.14159265358979323846f  /* pi */
const static float Deg2Rad = (PI * 2.0f) / 360.0f;
const static float Rad2Deg = 360.0f / (PI * 2.0f);

#define E 0.00001f

float GameUnitsToMetres(float units);
float CheckUnits(float units);
bool UsingMetres();
std::string MetresOn();
void ToggleMetres();