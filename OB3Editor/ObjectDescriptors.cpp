
#include <ObjectDescriptors.h>
#include <Files.h>

bool VertexUnPad::LoadASCII(string data)
{
	ConsumeSeekToChar(data, '{');
	x = ConsumeToFloat(data);
	y = ConsumeToFloat(data);
	z = ConsumeToFloat(data);
	ConsumeSeekToChar(data);

	return false;
}

string VertexUnPad::ToString()
{
	return "{" + to_string(x) + " " + to_string(y) + " " + to_string(z) + "}";
}

string VertexUnPad::ToStringSpaced()
{
	return "  " + to_string(x) + " " + to_string(y) + " " + to_string(z) + "  ";
}

VertexUnPad VertexUnPad::operator+(VertexUnPad other)
{
	return VertexUnPad(x + other.x, y + other.y, z + other.z);
}

VertexUnPad VertexUnPad::operator-(VertexUnPad other)
{
	return VertexUnPad(x - other.x, y - other.y, z - other.z);
}

LevelObject::LevelObject()
{
	dwSize = 0;

	for (int i = 0; i < 32; i++)
		TypeName[i] = '\0';
	for (int i = 0; i < 32; i++)
		AttachName[i] = '\0';

	ResetRotation();

	RenderableId = 0;
	ControllableId = 0;
	ShadowFlags = 0;
	Permanent = 1;
	TeamNumber = 0;
	SpecificData = 0;
	entryID = 0;
}

LevelObject::LevelObject(ObjectDescription12 desctiptor, std::vector<unsigned long> extraDataSize)
{
	dwSize = desctiptor.dwSize;

	for (int i = 0; i < 32; i++)
		TypeName[i] = desctiptor.TypeName[i];
	for (int i = 0; i < 32; i++)
		AttachName[i] = desctiptor.AttachName[i];

	ObjMatrix = desctiptor.ObjMatrix;

	RenderableId = desctiptor.RenderableId;
	ControllableId = desctiptor.ControllableId;
	ShadowFlags = desctiptor.ShadowFlags;
	Permanent = desctiptor.Permanent;
	TeamNumber = desctiptor.TeamNumber;
	SpecificData = desctiptor.SpecificData;

	ExtraDataSize = extraDataSize;

	entryID = 0;
}

LevelObject::~LevelObject()
{

}

void LevelObject::SetTypeName(string name)
{
	for (int i = 0; i < 32; i++)
		TypeName[i] = 0;

	size_t m = 32;
	if (name.length() < m)
		m = name.length();

	for (size_t i = 0; i < m; i++)
		TypeName[i] = name[i];
}

void LevelObject::SetAttachName(string name)
{
	for (int i = 0; i < 32; i++)
		AttachName[i] = 0;

	int m = 32;
	if (name.length() < m)
		m = name.length();

	for (int i = 0; i < m; i++)
		AttachName[i] = name[i];
}

void LevelObject::ResetRotation()
{
	VertexUnPad pos = ObjMatrix.t;
	char normal = ObjMatrix.Normal;

	ObjMatrix = MatrixUnPad{};

	ObjMatrix.m[0].x = ObjMatrix.m[1].y = ObjMatrix.m[2].z = 1;
	ObjMatrix.m[0].y = ObjMatrix.m[0].z = ObjMatrix.m[1].z = -0;

	ObjMatrix.t = pos;
	ObjMatrix.Normal = normal; // Wtf is normal, like mesh data normal?
}

void RotateByX(VertexUnPad& vec, float alpha)
{
	float Y = vec.y;
	float Z = vec.z;

	vec.y = Y * cosf(alpha) - Z * sinf(alpha);
	vec.z = Y * sinf(alpha) + Z * cosf(alpha);
}

void RotateByY(VertexUnPad& vert, float alpha)
{
	float X = vert.x;
	float Z = vert.z;

	vert.x = X * cos(alpha) + Z * sin(alpha);
	vert.z = -X * sin(alpha) + Z * cos(alpha);
}

void RotateByZ(VertexUnPad& vec, float alpha)
{
	float X = vec.x;
	float Y = vec.y;

	vec.x = X * cosf(alpha) - Y * sinf(alpha);
	vec.y = X * sinf(alpha) + Y * cosf(alpha);
}

void RotateMatByVector(MatrixUnPad& objMat, VertexUnPad euler)
{
	RotateByX(objMat.m[0], euler.x);
	RotateByX(objMat.m[1], euler.x);
	RotateByX(objMat.m[2], euler.x);

	RotateByY(objMat.m[0], euler.y);
	RotateByY(objMat.m[1], euler.y);
	RotateByY(objMat.m[2], euler.y);

	RotateByZ(objMat.m[0], euler.z);
	RotateByZ(objMat.m[1], euler.z);
	RotateByZ(objMat.m[2], euler.z);
}