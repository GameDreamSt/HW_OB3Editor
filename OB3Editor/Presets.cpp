
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>

#include <Presets.h>
#include <Files.h>
#include <ObjectDescriptors.h>
#include <Object.h>
#include <unordered_set>

using namespace std;
namespace fs = std::filesystem;

#define RET_ON_F(x) if(!x) return false;

struct Entry
{
	string objectID;
	string attachID;
	VertexUnPad offsetPosition;
	VertexUnPad offsetRotationEuler;

	bool LoadASCII(string data)
	{
		objectID = ConsumeSeekToChar(data);
		attachID = ConsumeSeekToChar(data);
		RET_ON_F(offsetPosition.LoadASCII(data));
		RET_ON_F(offsetRotationEuler.LoadASCII(data));
		return true;
	}

	string ToString()
	{
		return objectID + " " + attachID + " " + offsetPosition.ToString() + " " + offsetRotationEuler.ToString();
	}
};

struct PresetFile
{
	string name;
	string pathToFile;
	vector<Entry> entries;

	string ToString()
	{
		string out = "";
		for (size_t i = 0; i < entries.size(); i++)
		{
			out += to_string(i) + ": " + entries[i].ToString();
			if (i != entries.size() - 1)
				out += "\n";
		}
		return out;
	}

	string EntriesNameString()
	{
		string out = "";
		for (size_t i = 0; i < entries.size(); i++)
		{
			out += entries[i].objectID;
			if (i != entries.size() - 1)
				out += " ";
		}
		return out;
	}

	string Save()
	{
		string out = "";
		for (size_t i = 0; i < entries.size(); i++)
		{
			out += entries[i].ToString();
			if (i != entries.size() - 1)
				out += "\n";
		}
		return out;
	}
};

vector<PresetFile> presetFiles;

string GetPresetDirectory() 
{
	string presetDir = RemovePathLast(GetApplicationPath()) + "\\Presets"; 

	if (!fs::exists(presetDir))
	{
		if (!fs::create_directory(presetDir))
		{
			cout << "Failed to create a directory at " << presetDir << "\n";
			system("Pause");
		}
	}

	return presetDir;
}

void LoadPresets()
{
	presetFiles.clear();

	string presetDir = GetPresetDirectory();
	bool hadErrors = false;

	for (const auto& entry : fs::directory_iterator(presetDir))
	{
		fs::path filePath = entry.path();

		if (!filePath.has_extension() || filePath.extension() != ".txt")
			continue;

		PresetFile presetFile;
		presetFile.pathToFile = filePath.string();

		ifstream read = ifstream(filePath.c_str());
		string line;

		if (!getline(read, presetFile.name))
		{
			cout << "Failed to read preset name in " << presetFile.pathToFile << "\n";
			hadErrors = true;
		}

		while (getline(read, line))
		{
			Entry newPreset;
			newPreset.LoadASCII(line);
			presetFile.entries.push_back(newPreset);
		}

		presetFiles.push_back(presetFile);
	}

	if(hadErrors)
		system("Pause");
}

void SavePreset(PresetFile &file)
{
	string presetDir = GetPresetDirectory();
	unordered_set<string> usedFilenames;

	for (const auto& entry : fs::directory_iterator(presetDir))
	{
		fs::path filePath = entry.path();

		if (!filePath.has_extension() || filePath.extension() != ".txt")
			continue;

		usedFilenames.insert(filePath.filename().string());
	}

	size_t testIndex = usedFilenames.size();
	string filePath;
	do
	{
		filePath = presetDir + "\\Preset" + to_string(testIndex) + ".txt";
		testIndex++;
	} while (fs::exists(filePath));

	ofstream write(filePath);

	if (!write.is_open())
	{
		cout << "Failed to save preset " << file.name << " to " << filePath << '\n';
		return;
	}

	write << file.name << '\n';
	write << file.Save();
	write.close();

	cout << "Preset " << file.name << " saved!\n";
}

void ListAllPresetFiles()
{
	if (presetFiles.size() == 0)
	{
		cout << "No presets to list!\n";
		return;
	}

	string str;
	for (size_t i = 0; i < presetFiles.size(); i++)
	{
		str += to_string(i) + ": '" + presetFiles[i].name + "' " + presetFiles[i].EntriesNameString() + "\n";
	}
	cout << str;
}


void AddObject(PresetFile &file)
{
	Entry newEntry;

	newEntry.objectID = GetTypeName();
	newEntry.attachID = GetAttachName();
	newEntry.offsetPosition = GetPosition();
	newEntry.offsetRotationEuler = GetRotation();

	file.entries.push_back(newEntry);
}

void DeleteObject(PresetFile &file)
{
	cout << file.ToString() << "\n"
		<< "Enter entry index for deletion (-1 to cancel): ";

	int choice = -1;
	cin >> choice;

	if (choice < 0 || choice >= file.entries.size())
		return;

	file.entries.erase(file.entries.begin() + choice);
}

void ChangePresetName(PresetFile &file)
{
	cout << "Enter a new preset name: ";
	cin >> file.name;
}

void AddMode()
{
	ListAllPresetFiles();
	cout << "\nEnter preset file index to add to scene (-1 to cancel): ";

	int choice = -1;
	cin >> choice;

	if (choice < 0 || choice >= presetFiles.size())
		return;

	VertexUnPad commonPosition = GetPosition();
	VertexUnPad commonRotation = GetRotation();

	cout << "Enter team ID: ";
	int teamNumber;
	cin >> teamNumber;

	PresetFile file = presetFiles[choice];
	for (size_t i = 0; i < file.entries.size(); i++)
	{
		Entry entry = file.entries[i];
		LevelObject* newObj = new LevelObject();

		newObj->dwSize = sizeof(ObjectDescription12);

		newObj->SetTypeName(entry.objectID);
		newObj->SetAttachName(entry.attachID);

		newObj->TeamNumber = teamNumber;

		newObj->RenderableId = GetUnusedRenderableID();
		newObj->entryID = (unsigned long)((*GetLoadedObjects()).size());

		newObj->ObjMatrix.t = entry.offsetPosition + commonPosition;
		newObj->ResetRotation();
		RotateMatByVector(newObj->ObjMatrix, entry.offsetRotationEuler + commonRotation);

		GetLoadedObjects()->push_back(newObj);

		cout << "Object " << entry.objectID << " added!\n";
	}

	cout << "Done!\n";
	system("pause");
}

void PickObjectsFromScene(PresetFile &file)
{
	vector<LevelObject*> pickedObjects;
	auto objects = GetLoadedObjects();

	do
	{
		system("cls");
		PrintLowInfo();

		cout << "Enter object ID (-1 end) Currently selected count(" << pickedObjects.size() << "): ";
		int index;
		cin >> index;

		if (index < 0 || index >= objects->size())
			break;

		pickedObjects.push_back(objects->at(index));
	} while (true);

	
	if (pickedObjects.size() == 0)
		return;

	VertexUnPad mainPos = pickedObjects[0]->ObjMatrix.t;

	for (size_t i = 0; i < pickedObjects.size(); i++)
	{
		LevelObject* object = pickedObjects[i];
		Entry newEntry;

		newEntry.objectID = object->TypeName;
		newEntry.attachID = object->AttachName;

		newEntry.offsetPosition = object->ObjMatrix.t - mainPos;
		newEntry.offsetRotationEuler = MatrixToEulerAngles(object->ObjMatrix.m);

		file.entries.push_back(newEntry);
		cout << newEntry.objectID << " added!\n";
	}

	cout << "Done!\n";
	system("pause");
}

void EditPreset(PresetFile &preset, string modeString, bool addAfterSave)
{
	do
	{
		system("cls");
		cout << "Preset '" << preset.name << "' " << modeString << "\n"
			<< "1. Add object\n"
			<< "2. Add objects from scene\n"
			<< "3. Delete object\n"
			<< "4. Save preset\n"
			<< "5. Change name\n"
			<< "6. Cancel and go back\n"
			<< "Current objects:\n"
			<< preset.ToString() << '\n';

		int choice = -1;
		cin >> choice;

		if (choice == 1)
			AddObject(preset);
		else if (choice == 2)
			PickObjectsFromScene(preset);
		else if (choice == 3)
			DeleteObject(preset);
		else if (choice == 4)
		{
			if (addAfterSave)
				presetFiles.push_back(preset);
			SavePreset(preset);
			system("pause");
			break;
		}
		else if (choice == 5)
			ChangePresetName(preset);
		else
			break;

	} while (true);
}

void CreateNewMode()
{
	PresetFile newPreset;
	ChangePresetName(newPreset);

	EditPreset(newPreset, "add mode", true);
}

void EditMode()
{
	ListAllPresetFiles();
	cout << "\nEnter preset file index for editing (-1 to cancel): ";

	int choice = -1;
	cin >> choice;

	if (choice < 0 || choice >= presetFiles.size())
		return;

	EditPreset(presetFiles[choice], "edit mode", false);
}

void DeleteMode()
{
	ListAllPresetFiles();
	cout << "\nEnter preset file index for deletion (-1 to cancel): ";

	int choice = -1;
	cin >> choice;

	if (choice < 0 || choice >= presetFiles.size())
		return;

	PresetFile presetFile = presetFiles[choice];
	if (!fs::remove(presetFile.pathToFile))
		cout << "Failed to delete a preset file at " << presetFile.pathToFile << "\n";

	presetFiles.erase(presetFiles.begin() + choice);
}

void GoToPresets()
{
	LoadPresets();

	do
	{
		system("cls");
		cout << "Preset manager:\n"
			<< "1. List presets\n"
			<< "2. Add preset to scene\n"
			<< "3. Create a new preset\n"
			<< "4. Edit preset\n"
			<< "5. Delete preset\n"
			<< "6. Back\n";

		int choice = -1;
		cin >> choice;

		if (choice == 1)
		{
			ListAllPresetFiles();
			system("Pause");
		}
		else if (choice == 2)
			AddMode();
		else if (choice == 3)
			CreateNewMode();
		else if (choice == 4)
			EditMode();
		else if (choice == 5)
			DeleteMode();
		else
			break;

	} while (true);
}