#include "ObjData.h"

ObjData::ObjData()
{

}

ObjData::~ObjData()
{
}

bool ObjData::load(char* filename)
{
	//filePath = filename;

	std::ifstream f(filename);
	if(!f)
	{
		MessageBox(NULL, "Could not find obj file", filename, MB_OK);
		return false;
	}

	std::string str;
	while(!f.eof())
	{
		f >> str;
		
		if(str == "f") readFace(f);
		else if(str == "vt") readTex(f);
		else if(str == "vn") readNormal(f);
		else if(str == "v")	readVertex(f);
		//else if(str == "g") ;
		//else if(str == "usemtl") ;
		else if(str == "mtllib") readMtllib(f);
		else	readComment(f);
	}

	f.close();

	//read material file

	std::string temp;
	temp = "..\\Shaders\\obj_mesh\\" + mtllib;
	f.open(temp.c_str());
	if(!f)
	{
		MessageBox(NULL, "Could not find mtl file", filename, MB_OK);
		return false;
	}

	while(!f.eof())
	{
		f >> str;

		if(str == "map_Kd")
		{
			getline(f, str);
			str.erase(str.begin());
			textureFilePath = str;
		}
		else
			readComment(f);
	}

	f.close();

	return true;
}

void ObjData::readVertex(std::ifstream& f)
{
	D3DXVECTOR3 temp;

	f >> temp.x;
	f >> temp.y;
	f >> temp.z;
	temp.z *= -1.0f;

	vertex.push_back(temp);
}

void ObjData::readNormal(std::ifstream& f)
{
	D3DXVECTOR3 temp;

	f >> temp.x;
	f >> temp.y;
	f >> temp.z;
	temp.z *= -1.0f;

	normals.push_back(temp);
}

void ObjData::readTex(std::ifstream& f)
{
	D3DXVECTOR2 temp;

	f >> temp.x;
	f >> temp.y;

	temp.y = 1 - temp.y;

	tex.push_back(temp);
}

void ObjData::readComment(std::ifstream& f)
{
	std::string str;
	getline(f, str);
}

void ObjData::readFace(std::ifstream& f)
{
	Triangle tri;
	int num;
	char slash;

	for(int j = 2; j >= 0; j--)
	{
		for(int i = 0; i < 3; i++)
		{
			f >> num;
			tri.tr[j][i] = (float)num-1;

			f.get(slash);
		}
	}

	faces.push_back(tri);
}

void ObjData::readGroup(std::ifstream& f)
{
	std::string str;

	getline(f, str);
}

void ObjData::readMaterial(std::ifstream& f)
{

}

void ObjData::readMtllib(std::ifstream& f)
{
	getline(f, mtllib);
	mtllib.erase(mtllib.begin());
}

D3DXVECTOR3 ObjData::getVertex(int index)
{
	return vertex.at(index);
}

D3DXVECTOR3 ObjData::getNormal(int index)
{
	return normals.at(index);
}

D3DXVECTOR2 ObjData::getTex(int index)
{
	return tex.at(index);
}

Triangle ObjData::getFace(int index)
{
	return faces.at(index);
}

int ObjData::getNumberFaces()
{
	return (int)faces.size();
}

std::string ObjData::getTexturePath()
{
	return textureFilePath;
}