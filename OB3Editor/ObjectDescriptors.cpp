#include <ObjectDescriptors.h>

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

	int m = 32;
	if (name.length() < m)
		m = name.length();

	for (int i = 0; i < m; i++)
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
