#pragma once

#include <string>
#include <vector>

class VertexUnPad;
class LevelObject;

std::string GetTypeName();
std::string GetAttachName();
int GetTypeChoice(std::string promptText, std::vector<std::string>* printStuff);
int GetTypeChoicePair(std::string promptText, std::vector<std::pair<std::string, unsigned long>>* printStuff);
VertexUnPad GetPosition();
VertexUnPad GetRotation();
int GetUnusedRenderableID();
std::vector<LevelObject*>* GetLoadedObjects();
void PrintLowInfo();
VertexUnPad MatrixToEulerAngles(VertexUnPad m[3]);