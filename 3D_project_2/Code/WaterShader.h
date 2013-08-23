#ifndef WATERSHADER_H
#define WATERSHADER_H

#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <fstream>
#include "Shader.h"
using namespace std;

class WaterShader
{
private:
	struct MatrixBufferType
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	struct ReflectionBufferType
	{
		D3DXMATRIX reflection;
	};

	struct WaterBufferType
	{
		float waterTranslation;
		float reflectRefractScale;
		D3DXVECTOR2 padding;
	};

	struct Vertex
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR3 normal;
		D3DXVECTOR2 uv;
	};
public:
	WaterShader();
	WaterShader(const WaterShader&);
	~WaterShader();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, HWND);
	void Shutdown();
	bool InitializeShader(ID3D11Device*, ID3D11DeviceContext*);
	bool SetRefractionParameters(D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR3);
	bool SetReflectionParameters(D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR3);
	bool SetWaterParameters(D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX, D3DXVECTOR4, D3DXVECTOR4, D3DXVECTOR3, float, float);
	void RenderRefraction(ID3D11Device*, ID3D11DeviceContext*, ID3D11ShaderResourceView*);
	void RenderReflection(ID3D11Device*, ID3D11DeviceContext*, ID3D11ShaderResourceView*);
	void RenderWater(ID3D11Device*, ID3D11DeviceContext*, ID3D11ShaderResourceView*, ID3D11ShaderResourceView*);
private:
	
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, char*);

	

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;

	//new
	Shader* refractionShader;
	Shader* reflectionShader;
	Shader* waterShader;
	Buffer* refraction;
	Buffer* reflection;
	Buffer* water;
	D3DXMATRIX world;
	D3DXMATRIX view;
	D3DXMATRIX reflectionMatrix;
	D3DXMATRIX proj;
	D3DXVECTOR4 clipPlane;
	D3DXVECTOR4 ambient;
	D3DXVECTOR4 diffuse;
	D3DXVECTOR3 lightPos;
	float waterTranslation; 
	float reflectRefractScale;
	TextureClass* normalMap;
};

#endif