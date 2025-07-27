
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <algorithm>
#include <cctype>

#include <unordered_map>
#include <unordered_set>

#include <Files.h>
#include <ObjectDescriptors.h>
#include <KnownObjs.h>
#include <Presets.h>
#include <Object.h>
#include <Units.h>

using namespace std;

bool ReadAllBytes(string filePath, vector<unsigned char> &data)
{
	ifstream ifs(filePath, ios::binary | ios::ate);

	if (!ifs.is_open())
		return false;

	ifstream::pos_type pos = ifs.tellg();

	if (pos == 0)
		return false;

	ifs.seekg(0, ios::beg);

	vector<char> tempData;
	tempData.resize((size_t)pos);

	ifs.read(&tempData[0], pos);

	std::copy(tempData.begin(), tempData.end(), std::back_inserter(data));

	ifs.close();

	return true;
}

unsigned long ReadULong(vector<unsigned char> &allData, int index)
{
	unsigned long finalValue = *(unsigned long*)&allData[index];

	/*for (int i = index; i < index + 4; i++)
	{
		int power = i - index;
		finalValue += (unsigned long)(allData[i] * pow(10, power));
	}*/

	return finalValue;
}

string ReadString(vector<unsigned char>& allData, int index, int length)
{
	string out = "";

	for (int i = index; i < index + length; i++)
		out += allData[i];

	return out;
}

string MatrixToString(MatrixUnPad mat)
{
	string out = "";

	out += "Rotation matrix:\n";
	out += to_string(mat.m[0].x) + " " + to_string(mat.m[0].y) + " " + to_string(mat.m[0].z) + "\n";
	out += to_string(mat.m[1].x) + " " + to_string(mat.m[1].y) + " " + to_string(mat.m[1].z) + "\n";
	out += to_string(mat.m[2].x) + " " + to_string(mat.m[2].y) + " " + to_string(mat.m[2].z) + "\n";

	out += "Position:\n";
	out += to_string(CheckUnits(mat.t.x)) + " " + to_string(CheckUnits(mat.t.y)) + " " + to_string(CheckUnits(mat.t.z)) + "\n";

	out += "Normal: " + to_string(mat.Normal);

	return out;
}

string FindSoulNameFromID(unsigned long ID)
{
	auto souls = GetKnownSouls();

	for (size_t i = 0; i < souls->size(); i++)
	{
		if (souls->at(i).second == ID)
			return souls->at(i).first;
	}

	return "UNKNOWN";
}

string ExtraDataToString(vector<unsigned long>& extraData)
{
	string out = "";

	if (extraData.size() == 0)
		return "\n";

	out += "Amount of addons: " + to_string(extraData.size()) + '\n';

	auto addons = GetKnownAddons();
	for (unsigned long i = 0; i < extraData.size(); i++)
		for (int j = 0; j < 9; j++)
		{
			unsigned long addonTag = extraData[i] & 0xFF;
			if (addonTag == addons->at(j).second)
			{
				out += "    " + to_string(i) + ". " + addons->at(j).first;

				if (addonTag == 0x57) // Addon soul unit
				{
					int soulID = extraData[i] & 0xFFFF0000;
					soulID = soulID >> 16;
					out += " " + FindSoulNameFromID(soulID);
				}

				out += "\n";
				break;
			}
		}

	return out;
}

void PrintInfoAboutObject(LevelObject* objectInfo)
{
	string info = "";

	info += "\nEntry: " + to_string(objectInfo->entryID) + '\n';
	info += "dwSize: " + to_string(objectInfo->dwSize) + '\n';
	info += "TypeName: " + string(objectInfo->TypeName) + '\n';
	info += "AttachName: " + string(objectInfo->AttachName) + '\n';
	info += "ObjMatrix:\n" + MatrixToString(objectInfo->ObjMatrix) + '\n';
	info += "RenderableId: " + to_string(objectInfo->RenderableId) + '\n';
	info += "ControllableId: " + to_string(objectInfo->ControllableId) + '\n';
	info += "ShadowFlags: " + to_string(objectInfo->ShadowFlags) + '\n';
	info += "Permanent flag: " + to_string(objectInfo->Permanent) + '\n';
	info += "TeamNumber: " + to_string(objectInfo->TeamNumber) + '\n';
	info += "SpecificData: " + to_string(objectInfo->SpecificData) + '\n';
	info += ExtraDataToString(objectInfo->ExtraDataSize);

	cout << info;
}

bool LoadOB3File(string filePath)
{
	string fileName = GetFileName(filePath);
	//cout << "File name: " << fileName << '\n';

	vector<unsigned char> allData;
	if (!ReadAllBytes(filePath, allData))
	{
		cout << "Error finding the file: " << fileName << '\n';
		return false;
	}

	//cout << "File size in bytes: " << allData.size() << '\n';

	if (allData.size() <= 4)
		return false;

	string fileVersion = "";
	fileVersion.resize(4);
	memcpy(&fileVersion[0], &allData[0], 4);

	if (fileVersion != "OBJC")
	{
		cout << "The file version is not OBJC, but {" << fileVersion << "}!\nAre you sure you want to continue? It might crash or allocate all your memory! [Y/N]\n";

		string choice = "";
		cin >> choice;

		if(choice[0] != 'Y' && choice[0] != 'y')
			return false;
	}

	if (allData.size() <= 8)
		return false;

	unsigned long entries = ReadULong(allData, 4); // Number of entires is stored in the file now
	
	if (entries == 0)
	{
		cout << "Entry count is 0! Reading will stop\n";
		return false;
	}

	unsigned int descriptorSize = sizeof(ObjectDescription12); // 148
	unsigned int matrixSize = sizeof(MatrixUnPad); // 52 = 4 * 12 + 1
	unsigned int vertexSize = sizeof(VertexUnPad); // 12

	int cursor = 8;

	GetLoadedObjects()->clear();
	for (unsigned long i = 0; i < entries; i++)
	{
		if (cursor >= allData.size())
		{
			cout << "There are less entries then expected! Expected{"<< entries <<"}, actual{"<< i <<"}\n";
			system("pause");
			break;
		}

		ObjectDescription12* ptr = (struct ObjectDescription12*)(&allData[cursor]);
		
		if (ptr->dwSize + (size_t)cursor > allData.size())
		{
			cout << "Incomplete object detected! Entry {" << i << "}, expected {"<< ptr->dwSize <<"} bytes, but only {"<< allData.size() - cursor <<"} are left\n";
			system("pause");
			break;
		}

		vector<unsigned long> ExtraDataSize;
		ExtraDataSize.resize(ptr->ExtraDataSize[0]);
		int addons = ptr->ExtraDataSize[0];
		for (int i = 0; i < addons; i++)
			ExtraDataSize[i] = ReadULong(allData, (size_t)cursor + 148 + (size_t)i * 4);

		LevelObject* newObject = new LevelObject(*ptr, ExtraDataSize);
		newObject->entryID = i;
		GetLoadedObjects()->push_back(newObject);

		cursor += ptr->dwSize; // Offset to read next entry
	}

	return true;
}

/*
 // FOR DEBUGGING
		//int tempCursor = cursor;
		//ObjectDescription12 ptr{};

		//ptr.dwSize = ReadULong(allData, tempCursor);
		//tempCursor += 4;

		//string objectName = ReadString(allData, tempCursor, 32);
		//_memccpy(ptr.TypeName, &allData[tempCursor], 0, 32);
		//tempCursor += 32;

		//string objectAttachment = ReadString(allData, tempCursor, 32);
		//_memccpy(ptr.AttachName, &allData[tempCursor], 0, 32);
		//tempCursor += 32;

		//MatrixUnPad objectMatrix = *(MatrixUnPad*)&allData[tempCursor];
		//ptr.ObjMatrix = *(MatrixUnPad*)&allData[tempCursor];
		//tempCursor += matrixSize;

*/

bool ReadFile(string path)
{
	if (path.size() < 5)
		path += ".ob3";
	else
	{
		string ending = path.substr(path.size() - 4, 4);
		if (ending != ".ob3")
			path += ".ob3";
	}

	if (!LoadOB3File(path.c_str()))
	{
		system("pause");
		return false;
	}

	return true;
}

void PrintDetailedInfo(vector<LevelObject*>* objects)
{
	for (size_t i = 0; i < objects->size(); i++)
		PrintInfoAboutObject(objects->at(i));

	system("pause");
}

void PrintDetailedInfo()
{
	PrintDetailedInfo(GetLoadedObjects());
}

char asciitolower(char in) {
	if (in <= 'Z' && in >= 'A')
		return in - ('Z' - 'z');
	return in;
}

void FindObject()
{
	string nameChunk;
	cout << "What is the name of the object?: ";
	cin >> nameChunk;
	cout << '\n';

	transform(nameChunk.begin(), nameChunk.end(), nameChunk.begin(), asciitolower);

	vector<LevelObject*> foundObjects;

	string s1;
	auto objects = GetLoadedObjects();
	for (size_t i = 0; i < objects->size(); i++)
	{
		LevelObject* foundObject = objects->at(i);
		s1 = string(foundObject->TypeName);

		if (nameChunk.size() > s1.size())
			continue;

		transform(s1.begin(), s1.end(), s1.begin(), asciitolower);

		if (s1.find(nameChunk) == std::string::npos)
			continue;

		foundObjects.push_back(foundObject);
	}

	if (foundObjects.size() == 0)
	{
		cout << "No objects found... :(\n";
		system("pause");
		return;
	}

	cout << "Found " << to_string(foundObjects.size()) << " objects!\n";
	PrintDetailedInfo(&foundObjects);
}

bool needsToSave = false;
void SaveFile()
{
	ofstream write("level1.ob3", ios::binary);
	
	if (!write.is_open())
	{
		cout << "Failed to create a save file?\n";
		system("pause");
		return;
	}

	write << "OBJC";

	unsigned long entries = (unsigned long)GetLoadedObjects()->size();
	char* tempData = (char*)&entries;
	write.write(tempData, sizeof(unsigned long));

	ObjectDescription12 dummyDesc{};
	for (size_t i = 0; i < GetLoadedObjects()->size(); i++)
	{
		LevelObject* ptr = GetLoadedObjects()->at(i);

		dummyDesc.dwSize = ptr->dwSize;

		for (int j = 0; j < 32; j++)
		{
			dummyDesc.TypeName[j] = ptr->TypeName[j];
			dummyDesc.AttachName[j] = ptr->AttachName[j];
		}

		dummyDesc.ObjMatrix = ptr->ObjMatrix;

		dummyDesc.RenderableId = ptr->RenderableId;
		dummyDesc.ControllableId = ptr->ControllableId;
		dummyDesc.ShadowFlags = ptr->ShadowFlags;
		dummyDesc.Permanent = ptr->Permanent;
		dummyDesc.TeamNumber = ptr->TeamNumber;
		dummyDesc.SpecificData = ptr->SpecificData;
		dummyDesc.ExtraDataSize[0] = (unsigned long)ptr->ExtraDataSize.size();

		tempData = (char*)&dummyDesc;
		write.write(tempData, sizeof(dummyDesc));

		if (ptr->ExtraDataSize.size() == 0)
			continue;

		for (unsigned long j = 0; j < ptr->ExtraDataSize.size(); j++)
		{
			unsigned long extra = ptr->ExtraDataSize[j];

			tempData = (char*)&extra;
			write.write(tempData, sizeof(unsigned long));
		}
	}

	write.close();

	needsToSave = false;

	cout << "Done!\n";
	system("pause");
}

unsigned long GetTeamNumber()
{
	unsigned long teamNumber = 0; // 0 - player

	cout << "Enter the team number (0 - player, 1 - probably enemy...)\n";
	cin >> teamNumber;

	return teamNumber;
}

void ProcessSoulcatcherChoice(unsigned long &addonID)
{
	auto knownSouls = GetKnownSouls();
	unsigned long choice = GetTypeChoicePair("Choose the soul for your soulcatcher\n", knownSouls);

	choice = knownSouls->at(choice).second << 16; // YY 00 00 00 -> 00 00 YY 00
	addonID = addonID | choice; // XX 00 YY 00
}

int AddAddon(LevelObject* newObj)
{
	auto addons = GetKnownAddons();
	int type = GetTypeChoicePair("Select an addon you want to add. (-1 to stop, -2 to enter custom code)\n", addons);

	if (type == -2)
	{
		cout << "Ok hackerman, what's the secret code for this addon? (Should be from 80 to 89 I think)\n";
		cin >> type;

		newObj->ExtraDataSize.push_back(type);
		newObj->dwSize += 4; // More data than expected, needs to be bigger
	}
	else if (type != -1)
	{
		if (type >= addons->size() || type < 0)
		{
			cout << "Selection out of range!\n";
			system("pause");
			return false;
		}

		unsigned long addonID = addons->at(type).second; // XX 00 00 00
		if (addonID == 87) // It's a soulcatcher, add a soulname in a hacky way... these damn devs
			ProcessSoulcatcherChoice(addonID);

		newObj->ExtraDataSize.push_back(addonID);
		newObj->dwSize += 4; // More data than expected, needs to be bigger
	}

	return type == -1;
}

void ReplaceAddon(LevelObject* newObj, int where)
{
	auto addons = GetKnownAddons();
	int type = GetTypeChoicePair("Select an addon you want to use for replacement. (-1 to enter custom code)\n", addons);

	if (type == -1)
	{
		cout << "Ok hackerman, what's the secret code for this addon? (Should be from 80 to 89 I think)\n";
		cin >> type;

		newObj->ExtraDataSize[where] = type;
	}
	else if (type > 0)
	{
		if (type >= addons->size())
		{
			cout << "Selection out of range!\n";
			system("pause");
			return;
		}

		unsigned long addonID = addons->at(type).second; // XX 00 00 00
		if (addonID == 87) // It's a soulcatcher, add a soulname in a hacky way... these damn devs
			ProcessSoulcatcherChoice(addonID);

		newObj->ExtraDataSize[where] = addonID;
	}
}

void AddSceneObject()
{
	string typeName = GetTypeName();
	string attachName = GetAttachName();
	VertexUnPad pos = GetPosition();

	unsigned long teamNumber = GetTeamNumber();

	LevelObject* newObj = new LevelObject();

	newObj->dwSize = sizeof(ObjectDescription12);

	newObj->SetTypeName(typeName);
	newObj->SetAttachName(attachName);

	newObj->TeamNumber = teamNumber;

	newObj->RenderableId = GetUnusedRenderableID();
	newObj->entryID = (unsigned long)GetLoadedObjects()->size();

	newObj->ObjMatrix.t = pos;

	string answer = "";
	cout << "Do you want any addons(components/modules)? Y/N\n";
	cin >> answer;

	if (answer == "Y" || answer == "y")
		while (!AddAddon(newObj));

	GetLoadedObjects()->push_back(newObj);

	needsToSave = true;

	cout << "Object added!\n";
	system("pause");
}

void RemoveObject()
{
	PrintLowInfo();

again:;

	int id;
	cout << "Which object do you want to remove?\n";
	cin >> id;

	if (id < 0 || id >= GetLoadedObjects()->size())
	{
		cout << "ID is out of bounds!\n";
		system("pause");
		goto again;
	}

	string name = GetLoadedObjects()->at(id)->TypeName;

	delete GetLoadedObjects()->at(id);
	GetLoadedObjects()->erase(GetLoadedObjects()->begin() + id);

	for (int i = id; i < GetLoadedObjects()->size(); i++)
		GetLoadedObjects()->at(i)->entryID = i;

	needsToSave = true;

	cout << id << " " << name << " removed!\n";
	system("pause");
}

void RemoveAllObjects()
{
	char answer;
	cout << "Are you sure you want to remove ALL objects? Y/N\n";
	cin >> answer;

	if (answer != 'Y' && answer != 'y')
		return;

	GetLoadedObjects()->clear();

	needsToSave = true;

	cout << "All objects removed!\n";
	system("pause");
}

float Magnitude(VertexUnPad foo)
{
	return sqrt(foo.x * foo.x + foo.y * foo.y + foo.z * foo.z);
}

float Scalar(VertexUnPad foo, VertexUnPad bar)
{
	return bar.x * foo.x + bar.y * foo.y + bar.z * foo.z;
}

float VectorAngle(VertexUnPad foo, VertexUnPad bar) // IN RADIANS
{
	return acos(
		Scalar(foo, bar)
		/
		(Magnitude(foo) * Magnitude(bar))
	);
}

string EulerAnglesToString(VertexUnPad eulers)
{
	return "X:" + to_string(eulers.x) + "  Y:" + to_string(eulers.y) + "  Z:" + to_string(eulers.z);
}

void EditObject()
{
	PrintLowInfo();

again:;

	int id;
	cout << "Which object do you want to edit?\n";
	cin >> id;

	if (id < 0 || id >= GetLoadedObjects()->size())
	{
		cout << "ID is out of bounds!\n";
		system("pause");
		goto again;
	}

	LevelObject* editObject = GetLoadedObjects()->at(id);

	int choice;
	do
	{
		system("cls");
		cout << "Currently editing: {" << editObject->TypeName << "}\n"
			<< "What do you want to change?"
			<< "\n1. Type name: " << editObject->TypeName
			<< "\n2. Main weapon: " << editObject->AttachName
			<< "\n3. Position: " << CheckUnits(editObject->ObjMatrix.t.x) << " " << CheckUnits(editObject->ObjMatrix.t.y) << " " << CheckUnits(editObject->ObjMatrix.t.z)
			<< "\n4. Rotation: " << EulerAnglesToString(MatrixToEulerAngles(editObject->ObjMatrix.m)) << " degrees"
			<< "\n5. Normal flag: " << to_string(editObject->ObjMatrix.Normal)
			<< "\n6. Renderable ID: " << to_string(editObject->RenderableId)
			<< "\n7. Controllable ID: " << to_string(editObject->ControllableId)
			<< "\n8. ShadowFlags Code: " << to_string(editObject->ShadowFlags)
			<< "\n9. Permanent flag: " << to_string(editObject->Permanent)
			<< "\n10. Team number: " << to_string(editObject->TeamNumber)
			<< "\n11. Specific data code: " << to_string(editObject->SpecificData)
			<< "\n12. Change addons: " << ExtraDataToString(editObject->ExtraDataSize)
			<< "13. Stop\n";

		cin >> choice;

		if (choice < 1 || choice > 12)
			break;

		unsigned int tmp_u_int;
		VertexUnPad angles;
		switch (choice)
		{
		case 1:
			editObject->SetTypeName(GetTypeName());
			needsToSave = true;
			break;

		case 2:
			editObject->SetAttachName(GetAttachName());
			needsToSave = true;
			break;

		case 3:
			editObject->ObjMatrix.t = GetPosition();
			needsToSave = true;
			break;

		case 4:
			angles = GetRotation();

			editObject->ResetRotation();
			RotateMatByVector(editObject->ObjMatrix, angles);
			needsToSave = true;
			break;

		case 5:
			cout << "Enter normal flag 0/1\n";
			cin >> editObject->ObjMatrix.Normal;
			needsToSave = true;
			break;

		case 6:
			cout << "Enter Renderable ID\n";
			cin >> editObject->RenderableId;
			needsToSave = true;
			break;

		case 7:
			cout << "Enter Controllable ID\n";
			cin >> editObject->ControllableId;
			needsToSave = true;
			break;

		case 8:
			cout << "Enter ShadowFlags Code\n";
			cin >> editObject->ShadowFlags;
			needsToSave = true;
			break;

		case 9:
			cout << "Enter permanent flag (0-255)\n";
			cin >> tmp_u_int;
			editObject->Permanent = (char)tmp_u_int;
			needsToSave = true;
			break;

		case 10:
			cout << "Enter team number\n";
			cin >> editObject->TeamNumber;
			needsToSave = true;
			break;

		case 11:
			cout << "Specific data code\n";
			cin >> editObject->SpecificData;
			needsToSave = true;
			break;

		case 12:

			chooseAgain:;
			system("cls");

			cout << ExtraDataToString(editObject->ExtraDataSize) << "Do you want to:\n1. Add an addon\n2. Remove it\n3. Replace\n4. Stop\n";
			cin >> choice;

			if (choice == 1)
				AddAddon(editObject);
			else if (choice == 2)
			{
				cout << "Choose an addon to remove:\n" << ExtraDataToString(editObject->ExtraDataSize);
				cin >> choice;

				if (choice < 0 || choice >= editObject->ExtraDataSize.size())
				{
					cout << "Choice is out of range!\n";
					system("pause");
					goto chooseAgain;
				}

				editObject->ExtraDataSize.erase(editObject->ExtraDataSize.begin() + choice);
				editObject->dwSize -= 4;
			}
			else if (choice == 3)
			{
				cout << "Choose an addon to replace:\n" << ExtraDataToString(editObject->ExtraDataSize);
				cin >> choice;

				if (choice < 0 || choice >= editObject->ExtraDataSize.size())
				{
					cout << "Choice is out of range!\n";
					system("pause");
					goto chooseAgain;
				}

				ReplaceAddon(editObject, choice);
			}
			else
				break;

			needsToSave = true;
			goto chooseAgain;
			break;

		default:
			break;
		}

		if (needsToSave)
		{
			cout << "Changes saved in memory\n";
			system("pause");
		}

	} while (true);

}

void PrintCredits()
{
	system("cls");
	cout << "Made by Game Dream St.\nJoin our Hostile Waters Community Discord server! https://discord.gg/sqsKRw4 \n";
	system("pause");
}

size_t GetRandomInt(size_t from, size_t to) // from - inclusive. to - exclusive
{
	return rand() % (to - from) + from;
}

void RandomizeAllUnits()
{
	unordered_map<string, vector<LevelObject*>> differentUnits;

	for (size_t i = 0; i < GetLoadedObjects()->size(); i++)
	{
		LevelObject* object = GetLoadedObjects()->at(i);
		differentUnits[object->TypeName].push_back(object);
	}

	unordered_map<string, vector<LevelObject*>>::iterator it = differentUnits.begin();

	auto objects = GetKnownObjects();
	vector<string> choiceObjs = *objects; // Hopefully does a deep copy

	while (it != differentUnits.end() && choiceObjs.size() > 0)
	{
		size_t rnd = GetRandomInt(0, choiceObjs.size());
		string randomType = choiceObjs[rnd];
		choiceObjs.erase(choiceObjs.begin() + rnd);

		vector<LevelObject*> objects = (*it).second;

		for (size_t i = 0; i < objects.size(); i++)
		{
			objects[i]->SetTypeName(randomType);
			objects[i]->TeamNumber = 1;
		}

		it++;
	}

	needsToSave = true;

	cout << "Done!\n";
	system("pause");
}

void RandomizeOneUnit()
{
	unordered_map<string, vector<LevelObject*>> differentUnits;

	for (size_t i = 0; i < GetLoadedObjects()->size(); i++)
	{
		LevelObject* object = GetLoadedObjects()->at(i);
		differentUnits[object->TypeName].push_back(object);
	}

	unordered_map<string, vector<LevelObject*>>::iterator it = differentUnits.begin();

	size_t itRnd = GetRandomInt(0, differentUnits.size());
	for (size_t i = 0; i < itRnd; i++)
		it++;

	vector<LevelObject*> objects = (*it).second;

	string from = "";
	if (objects.size() > 0)
		from = objects[0]->TypeName;

	string randomType;
	bool notOk = true;
	auto knownObjects = GetKnownObjects();
	do
	{
		size_t rnd = GetRandomInt(0, knownObjects->size());
		randomType = knownObjects->at(rnd);

		cout << "From " << from << " To: " << randomType << '\n'
		<< "Is that ok? Y/N (S = stop)\n";

		string answer;
		cin >> answer;

		if (answer == "y" || answer == "Y")
			notOk = false;
		else if (answer == "s" || answer == "S")
			return;
	} while (notOk);


	for (size_t i = 0; i < objects.size(); i++)
	{
		objects[i]->SetTypeName(randomType);
		objects[i]->TeamNumber = 1;
	}

	needsToSave = true;

	cout << "Done!\n";
	system("pause");
}

void Ranzomizer()
{
	do
	{
		system("cls");
		cout << "Randomizer:\n"
			//<< "1. Randomize EVERYTHING\n"
			<< "1. Randomize all units\n"
			<< "2. Randomize one unit\n"
			//<< "3. Randomize a specific unit\n"
			//<< "4. Randomize unlock\ns"
			//<< "5. Randomize EJ\n"
			//<< "6. Randomize carrier shells\n"
			//<< "7. Randomize enemy waves\n"
			//<< "8. Randomize enemy bases\n"
			<< "3. Back\n";

		int choice = -1;
		cin >> choice;

		if (choice == 1)
			RandomizeAllUnits();
		else if (choice == 2)
			RandomizeOneUnit();
		else
			break;

	} while (true);
}

int main(int argc, char* argv[])
{
	srand((unsigned int)time(NULL));

	string path = "";

	// Command line
	if (argc > 1)
		path = argv[1];

	bool close = true, editing = true;
	do
	{
		close = true;

		if (path == "")
		{
			typeFilename:;

			printf("What .ob3 file do you want to read?\n(Enter absolute or relative path. You can also drag and drop the file)\n");

			string fileName;
			getline(std::cin, fileName);

			fileName = RemoveQuotes(fileName);

			if (!IsAPath(fileName))
				path = RemovePathLast(string(argv[0])) + "\\" + fileName;
			else
				path = fileName;
		}
		else
			path = RemoveQuotes(path);

		if (!ReadFile(path))
		{
			system("cls");
			goto typeFilename;
		}

		string file = GetFileName(path);

		LoadUnitsSettings();

		do
		{
			editing = true;
			system("cls");

			if (needsToSave)
				cout << "Don't forget to save!\n\n";

			cout << "Curently editing " << file << '\n'
				<< "What would you like to do?\n"
				<< "1. List all objects\n"
				<< "2. List all objects in detail\n"
				<< "3. Find object\n"
				<< "4. Add object\n"
				<< "5. Edit object\n"
				<< "6. Remove object\n"
				<< "7. Remove all objects\n"
				<< "8. Save file\n"
				<< "9. Use meters instead of game units ["<< MetresOn() <<"]\n"
				<< "10. Randomizer\n"
				<< "11. Presets\n"
				<< "12. Credits\n"
				<< "13. Quit\n";

			int choice = -1;
			cin >> choice;

			if (choice == 1)
			{
				PrintLowInfo();
				system("pause");
			}
			else if (choice == 2)
				PrintDetailedInfo();
			else if (choice == 3)
				FindObject();
			else if (choice == 4)
				AddSceneObject();
			else if (choice == 5)
				EditObject();
			else if (choice == 6)
				RemoveObject();
			else if (choice == 7)
				RemoveAllObjects();
			else if (choice == 8)
				SaveFile();
			else if (choice == 9)
				ToggleMetres();
			else if (choice == 10)
				Ranzomizer();
			else if (choice == 11)
				GoToPresets();
			else if (choice == 12)
				PrintCredits();
			else
				editing = false;

		} while (editing);

		needsToSave = false;

		printf("Do you wish to continue? Y/N: ");
		cin >> path;
		printf("\n");

		if (path == "Y" || path == "y")
			close = false;

		path = "";
	} while (!close);

	return 0;
}