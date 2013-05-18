#ifndef OBJ_MESH_H
#define OBJ_MESH_H

#include "ObjData.h"
#include "TextureClass.h"
#include "Light.h"

	struct Vertex
	{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 tex;

	Vertex()
	{
		pos = D3DXVECTOR3(0, 0, 0);
		normal = D3DXVECTOR3(0, 0, 0);
		tex = D3DXVECTOR2(0, 0);
	}
	};

class ObjMesh
{
public:
	ObjMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~ObjMesh();

	bool initiate(char* filename);
	bool initShader();
	bool createBuffers();

	void render(D3DXMATRIX& view, D3DXMATRIX& proj, D3DXVECTOR3& cameraPos, PointLight& light);

	void billboard(const D3DXVECTOR3& cameraPos);

private:
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	Shader* shader;

	Buffer* vertexBuffer;
	Buffer* indexBuffer;
	int numOfVertices;

	TextureClass* texture;

	ObjData* data;

	D3DXMATRIX world, translate;
	D3DXVECTOR3 pos;


};

#endif