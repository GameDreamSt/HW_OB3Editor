#pragma once

#include <string>

std::string RemoveQuotes(std::string str);
bool IsAPath(std::string str);
bool SeekToSlash(std::string path, int& slashI);
std::string RemovePathLast(std::string path);
std::string GetFileName(std::string path);
std::string GetApplicationPath();
bool DirectoryExists(std::string path);
std::string ConsumeSeekToChar(std::string& data);
std::string ConsumeSeekToChar(std::string& data, char seekToChar);
float ConsumeToFloat(std::string& data);
float ConsumeToFloat(std::string& data, char seekToChar);