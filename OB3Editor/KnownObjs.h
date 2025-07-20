#pragma once

#include <string>
#include <vector>

std::vector<std::pair<std::string, unsigned long>>* GetKnownAddons();
std::vector<std::pair<std::string, unsigned long>>* GetKnownSouls();
std::vector<std::string>* GetKnownWeapons();
std::vector<std::string>* GetKnownObjects();