#pragma once

#include <string>

std::string RemoveQuotes(std::string str);
bool IsAPath(std::string str);
bool SeekToSlash(std::string path, int& slashI);
std::string RemovePathLast(std::string path);
std::string GetFileName(std::string path);
std::string GetApplicationPath();
std::string ConsumeSeekToChar(std::string& data);
std::string ConsumeSeekToChar(std::string& data, char seekToChar);
float ConsumeToFloat(std::string& data);
float ConsumeToFloat(std::string& data, char seekToChar);
int ConsumeToInt(std::string& data);
int ConsumeToInt(std::string& data, char seekToChar);
bool ConsumeToBool(std::string& data);
bool ConsumeToBool(std::string& data, char seekToChar);
std::string FloatToString(float var, int precision = 2);