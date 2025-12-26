
#include <Files.h>
#include <Windows.h>
#include <iostream>

using namespace std;

string RemoveQuotes(string str)
{
	if (str.size() < 2)
		return str;

	if (str[0] == '"')
		str.erase(str.begin());

	if (str[str.size() - 1] == '"')
		str.erase(str.begin() + str.size() - 1);

	return str;
}

bool IsAPath(string str)
{
	if (str.size() < 2)
		return false;

	if (str[1] != ':')
		return false;

	return true;
}

bool SeekToSlash(string path, int& slashI)
{
	slashI = -1;

	for (int i = (int)path.size() - 1; i >= 0; i--)
		if (path[i] == '\\')
		{
			slashI = i + 1;
			return true;
		}

	return false;
}

string RemovePathLast(string path)
{
	int slashI;
	bool foundSlash = SeekToSlash(path, slashI);

	if (foundSlash)
		return path.substr(0, slashI - (size_t)1);
	else
		return path;
}

string GetFileName(string path)
{
	int slashI;
	bool foundSlash = SeekToSlash(path, slashI);

	if (!foundSlash)
		return "NONAME";

	return path.substr(slashI, path.size() - slashI);
}

string GetApplicationPath()
{
	string path;
	path.resize(256);
	unsigned long err = GetModuleFileNameA(NULL, (LPSTR)path.data(), (DWORD)path.capacity());
	if (err == 0)
		cout << "Error while getting application path! " << GetLastError() << "\n";
	path.resize(err);
	return path;
}

string ConsumeSeekToChar(string& data) { return ConsumeSeekToChar(data, ' '); }
string ConsumeSeekToChar(string &data, char seekToChar)
{
	for (size_t i = 0; i < data.length(); i++)
	{
		if (data[i] == seekToChar)
		{
			string sub = data.substr(0, i);
			data = data.substr(i + 1, data.length() - i - 1); // Ignore the seek char
			return sub;
		}
	}

	return data;
}

float ConsumeToFloat(string& data) { return ConsumeToFloat(data, ' '); }
float ConsumeToFloat(string& data, char seekToChar)
{
	string sub = ConsumeSeekToChar(data);
	return stof(sub);
}

bool ConsumeToBool(string& data) { return ConsumeToBool(data, ' '); }
bool ConsumeToBool(string& data, char seekToChar)
{
	string sub = ConsumeSeekToChar(data);
	if (sub == "true" || sub == "True" || sub == "TRUE")
		return true;
	if (sub == "false" || sub == "False" || sub == "FALSE")
		return false;

	return stoi(sub) == 1;
}

string FloatToString(float var, int precision)
{
	int whole = var;
	int multiplyBy = pow(10, precision);
	int remainder = (int)(var * multiplyBy) - whole * multiplyBy;
	if (remainder > 0)
		return to_string(whole) + "." + to_string(remainder);
	else
	{
		string returnStr = to_string(whole) + ".";
		for (int i = 0; i < precision; i++)
			returnStr.push_back('0');
		return returnStr;
	}
}