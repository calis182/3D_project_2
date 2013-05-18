#ifndef OBJ_DATA_H
#define OBJ_DATA_H

//Den ska ha all raw data.

#include "stdafx.h"

struct ObjMaterialData
{
	float ka[3];
	float kd[3];
	float ks[3];
	float ns;
	//..
	//..

};

struct ObjGroupData
{

};

struct Triangle
{
	float tr[3][3];

	Triangle()
	{
		for(int i = 0; i < 3; i++)
		{
			for(int j = 0; j < 3; j++)
				tr[i][j] = 0;
		}
	}
};

class ObjData
{
public:
	ObjData();
	~ObjData();

	bool load(char* filename);

	void readVertex(std::ifstream& f);
	void readNormal(std::ifstream& f);
	void readTex(std::ifstream& f);
	void readComment(std::ifstream& f);
	void readFace(std::ifstream& f);
	void readGroup(std::ifstream& f);
	void readMaterial(std::ifstream& f);
	void readMtllib(std::ifstream& f);

	D3DXVECTOR3 getVertex(int index);
	D3DXVECTOR3 getNormal(int index);
	D3DXVECTOR2 getTex(int index);
	Triangle getFace(int index);

	int getNumberFaces();

	std::string getTexturePath();

private:
	std::vector<D3DXVECTOR3> vertex;
	std::vector<D3DXVECTOR2> tex;
	std::vector<D3DXVECTOR3> normals;
	std::vector<Triangle> faces;

	std::string textureFilePath;
	std::string mtllib;

};

#endif