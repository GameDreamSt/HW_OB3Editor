#pragma once

#include <string>

std::string RemoveQuotes(std::string str)
{
	if (str.size() < 2)
		return str;

	if (str[0] == '"')
		str.erase(str.begin());

	if (str[str.size() - 1] == '"')
		str.erase(str.begin() + str.size() - 1);

	return str;
}

bool IsAPath(std::string str)
{
	if (str.size() < 2)
		return false;

	if (str[1] != ':')
		return false;

	return true;
}

bool SeekToSlash(std::string path, int& slashI)
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

std::string RemovePathLast(std::string path)
{
	int slashI;
	bool foundSlash = SeekToSlash(path, slashI);

	if (foundSlash)
		return path.substr(0, slashI - (size_t)1);
	else
		return path;
}

std::string GetFileName(std::string path)
{
	int slashI;
	bool foundSlash = SeekToSlash(path, slashI);

	if (!foundSlash)
		return "NONAME";

	return path.substr(slashI, path.size() - slashI);
}