
#include <Units.h>

bool usingMetres;

float GameUnitsToMetres(float units)
{
	return units / GAME_UNITS_IN_METER;
}

float CheckUnits(float units)
{
	if (usingMetres)
		return GameUnitsToMetres(units);
	else
		return units;
}

bool UsingMetres() { return usingMetres; }

std::string MetresOn()
{
	if (usingMetres)
		return "ON";
	else
		return "OFF";
}

void ToggleMetres()
{
	usingMetres = !usingMetres;
}