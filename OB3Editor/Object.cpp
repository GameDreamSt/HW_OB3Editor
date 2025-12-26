
#include <Object.h>
#include <iostream>
#include <ObjectDescriptors.h>
#include <KnownObjs.h>
#include <Units.h>
#include <unordered_set>

using namespace std;

vector<LevelObject*> loadedObjects;

int GetTypeChoice(string promptText, vector<string> *printStuff)
{
	string out = "";

	for (size_t i = 0; i < printStuff->size(); i++)
		out += to_string(i) + ". " + printStuff->at(i) + "\n";

	out += promptText;
	cout << out;

	int type = -1;
	cin >> type;

	return type;
}

int GetTypeChoicePair(string promptText, vector<pair<string, unsigned long>> *printStuff)
{
	string out = "";

	for (size_t i = 0; i < printStuff->size(); i++)
		out += to_string(i) + ". " + printStuff->at(i).first + "\n";

	out += promptText;
	cout << out;

	int type = -1;
	cin >> type;

	return type;
}

string GetTypeName()
{
startAgain:;

	system("cls");

	auto objects = GetKnownObjects();
	int type = GetTypeChoice("Choose an object type, type -1 for custom type\n", objects);

	string typeName = "";
	if (type == -1)
	{
		cout << "ok smartypants, what is the object type name?\n";
		cin >> typeName;
	}
	else
	{
		if (type > objects->size() || type < 0)
		{
			cout << "Hey it's out of range! What are you doing?? >:(\n";
			system("pause");
			goto startAgain;
		}

		typeName = objects->at(type);
	}

	return typeName;
}

string GetAttachName()
{
startAgainB:;

	string attachName = "";
	auto weapons = GetKnownWeapons();
	int type = GetTypeChoice("Choose an weapon attachment type, type -1 to skip, -2 for custom type, \n", weapons);

	if (type == -2)
	{
		cout << "Ok smartypants, what is the object type name?\n";
		cin >> attachName;
	}
	else if (type != -1)
	{
		if (type > weapons->size() || type < 0)
		{
			cout << "Hey it's out of range! What are you doing?? >:(\n";
			system("pause");
			goto startAgainB;
		}

		attachName = weapons->at(type);
	}

	return attachName;
}

VertexUnPad GetPosition()
{
	float readF;
	VertexUnPad pos{};

	if (UsingMetres())
	{
		cout << "What should the position be for this unit? (in meters)\nx: ";
		cin >> readF;
		pos.x = readF * GAME_UNITS_IN_METER;

		cout << "y: ";
		cin >> readF;
		pos.y = readF * GAME_UNITS_IN_METER;

		cout << "z: ";
		cin >> readF;
		pos.z = readF * GAME_UNITS_IN_METER;
	}
	else
	{
		cout << "What should the position be for this unit? (in \"game units\" (NOT METERS >:( ))\nx: ";
		cin >> readF;
		pos.x = readF;

		cout << "y: ";
		cin >> readF;
		pos.y = readF;

		cout << "z: ";
		cin >> readF;
		pos.z = readF;
	}

	return pos;
}

VertexUnPad GetRotation()
{
	VertexUnPad angles;

	cout << "What angle do you want your object to look at? from -180 to 180\nx(up/down):";
	cin >> angles.x;
	cout << "y(left/right/around):";
	cin >> angles.y;
	cout << "z(forward swirl):";
	cin >> angles.z;

	angles.x *= Deg2Rad;
	angles.y *= Deg2Rad;
	angles.z *= Deg2Rad;

	return angles;
}

int GetUnusedRenderableID()
{
	unordered_set<unsigned long> loadedIds;
	for (size_t i = 0; i < loadedObjects.size(); i++)
		loadedIds.insert(loadedObjects[i]->RenderableId);

	unsigned long unusedRendId = -1;
	for (unsigned long i = 1; i < loadedObjects.size() + 10; i++)
		if (loadedIds.count(i) == 0)
		{
			unusedRendId = i;
			break;
		}

	if (unusedRendId == -1)
		unusedRendId = rand() % 1000 + (unsigned long)loadedObjects.size() + 10;

	return unusedRendId;
}

vector<LevelObject*>* GetLoadedObjects() { return &loadedObjects; }
void RefreshLoadedObjectIDs()
{
	for (size_t i = 0; i < loadedObjects.size(); i++)
	{
		loadedObjects[i]->entryID = (unsigned long)i;
	}
}

string tempString;

void PrintLowInfo()
{
	tempString.clear();
	char sep = ' ';
	for (size_t i = 0; i < loadedObjects.size(); i++)
	{
		LevelObject* object = loadedObjects[i];
		VertexUnPad checkedUnits(CheckUnits(object->ObjMatrix.t.x), CheckUnits(object->ObjMatrix.t.y), CheckUnits(object->ObjMatrix.t.z));
		tempString +=  to_string(i) + ". " + object->TypeName + "\t" + checkedUnits.ToStringSpaced() + '\n';
	}

	cout << tempString;
}

bool EpsilonFloatEquals(float f, float to)
{
	return f > to - E && f < to + E;
}

VertexUnPad MatrixToEulerAngles(VertexUnPad m[3])
{
	/*eulers.x = atan2(m[2].y, m[2].z) * Rad2Deg;
	eulers.y = atan2(-m[2].x, sqrt(m[2].y * m[2].y + m[2].z * m[2].z)) * Rad2Deg;
	eulers.z = atan2(m[1].x, m[0].x) * Rad2Deg;*/

	float R11 = m[0].x;
	float R12 = m[0].y;
	float R13 = m[0].z;

	float R21 = m[1].x;
	float R23 = m[1].z;

	float R31 = m[2].x;
	float R32 = m[2].y;
	float R33 = m[2].z;

	float E1, E2, E3, delta;
	if (EpsilonFloatEquals(R13, 1) || EpsilonFloatEquals(R13, -1))
	{
		E3 = 0;
		delta = atan2(R12, R13);

		if (EpsilonFloatEquals(R13, -1))
		{
			E2 = PI / 2.0f;
			E1 = E3 + delta;
		}
		else
		{
			E2 = -PI / 2.0f;
			E1 = -E3 + delta;
		}
	}
	else
	{
		E2 = -asin(R13);
		E1 = atan2(R23 / cos(E2), R33 / cos(E2));
		E3 = atan2(R12 / cos(E2), R11 / cos(E2));
	}

	VertexUnPad eulers = VertexUnPad(E1, E2, E3);

	eulers.x *= Rad2Deg;
	eulers.y *= Rad2Deg;
	eulers.z *= Rad2Deg;

	return eulers;
}