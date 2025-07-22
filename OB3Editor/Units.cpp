
#include <Units.h>
#include <Files.h>

#include <filesystem>
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

bool usingMetres;

string GetUnitFilePath()
{
	string filePath = RemovePathLast(GetApplicationPath()) + "\\Units.txt";
	return filePath;
}

void SaveUnitsSettings()
{
	ofstream write(GetUnitFilePath());
	write << "Using meters: " << usingMetres << '\n';
	write.close();
}

void LoadUnitsSettings()
{
	string filePath = GetUnitFilePath();
	if (!fs::exists(filePath))
		return;

	string lineString;
	ifstream read(filePath);
	getline(read, lineString);
	read.close();

	ConsumeSeekToChar(lineString, ':'); // Consume "using meters: "
	ConsumeSeekToChar(lineString); // Consume space
	usingMetres = ConsumeToBool(lineString);
}

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
	SaveUnitsSettings();
}