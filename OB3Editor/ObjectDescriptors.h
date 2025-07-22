#pragma once

#include <vector>
#include <string>

using namespace std;

class VertexUnPad
{
public:
	float x, y, z;

	VertexUnPad() { x = y = z = 0; }
	VertexUnPad(float X, float Y, float Z) { x = X; y = Y; z = Z; }

	bool LoadASCII(string data);
	string ToString();
	string ToStringSpaced();

	VertexUnPad operator +(VertexUnPad other);
	VertexUnPad operator -(VertexUnPad other);
};

class MatrixUnPad
{
public:
	VertexUnPad m[3];
	VertexUnPad t;
	bool Normal;
};

struct ObjectDescription12
{
	unsigned long dwSize;		// Size of structure with extra data

	char	TypeName[32];
	char	AttachName[32];

	MatrixUnPad		ObjMatrix;

	unsigned long 	RenderableId;
	unsigned long 	ControllableId;	// Remove Me, I'm Evil!
	unsigned long 	ShadowFlags;
	char			Permanent;
	unsigned long	TeamNumber;
	unsigned long	SpecificData;

	unsigned long 	ExtraDataSize[1];	// Size of extra data folowed by data
};

class LevelObject
{
public:
	LevelObject();
	LevelObject(ObjectDescription12 desctiptor, std::vector<unsigned long> extraDataSize);

	~LevelObject();

	unsigned long dwSize;

	char	TypeName[32];
	char	AttachName[32];

	MatrixUnPad		ObjMatrix;

	unsigned long 	RenderableId;
	unsigned long 	ControllableId;
	unsigned long 	ShadowFlags;
	char			Permanent;
	unsigned long	TeamNumber;
	unsigned long	SpecificData;

	std::vector<unsigned long> ExtraDataSize;

	unsigned long entryID;

	void SetTypeName(string name);
	void SetAttachName(string name);

	void ResetRotation();
};

void RotateByX(VertexUnPad& vec, float alpha);
void RotateByY(VertexUnPad& vert, float alpha);
void RotateByZ(VertexUnPad& vec, float alpha);
void RotateMatByVector(MatrixUnPad& objMat, VertexUnPad euler);