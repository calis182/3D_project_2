#include "WaterSimulation.h"

WaterSimulation::WaterSimulation()
{
	shader = NULL;
	renderShader = NULL;

	tessMesh = NULL;
	tessIndex = NULL;

	waterTexture = NULL;

	D3DXMatrixIdentity(&position);
}

WaterSimulation::~WaterSimulation()
{
	SAFE_RELEASE(shader);
	SAFE_DELETE(shader);
	SAFE_DELETE(renderShader);

	SAFE_DELETE(tessMesh);
	SAFE_DELETE(tessIndex);

	SAFE_DELETE(waterTexture);

	SAFE_RELEASE(buffers[0]);
	SAFE_RELEASE(buffers[1]);

	SAFE_RELEASE(UAV[0]);
	SAFE_RELEASE(UAV[1]);

	SAFE_RELEASE(SRV[0]);
	SAFE_RELEASE(SRV[1]);
}

bool WaterSimulation::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXVECTOR3 pos, int sizeX, int sizeY, int calcX, int calcY, int detailed)
{
	D3DXMatrixTranslation(&position, pos.x, pos.y, pos.z);

	if(!initShaders(device, deviceContext))
		return false;

	if(!initResources(device, deviceContext, pos, sizeX, sizeY, calcX, calcY, detailed))
		return false;

	return true;
}

bool WaterSimulation::initShaders(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	HRESULT hr = S_OK;
	ID3DBlob* pBlob = NULL;

	hr = CompileShaderFromFile("..\\Shaders\\waterSimulation.fx", "CSMain", "cs_5_0", &pBlob);
	if(FAILED(hr))
		return false;

	hr = device->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &shader);
	SAFE_RELEASE(pBlob);
	if(FAILED(hr))
		return false;

	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	renderShader = new Shader();
	hr = renderShader->Init(device, deviceContext, "..\\Shaders\\waterTesselator.fx", inputDesc, 2);
	if(FAILED(hr))
		return false;
	
	return true;
}

bool WaterSimulation::initResources(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXVECTOR3 pos, int sizeX, int sizeY, int calcX, int calcY, int detailed)
{
	this->sizeX = sizeX;
	this->sizeY = sizeY;

	this->calcX = calcX * 16;
	this->calcY = calcY * 16;

	threadGroupX = calcX;
	threadGroupY = calcY;

	totalCalcPoints = this->calcX * this->calcY;

	WaterData* waterData;
	waterData = new WaterData[totalCalcPoints];

	for(int i = 0; i < this->calcX; i++)
	{
		for(int j = 0; j < this->calcY; j++)
		{
			const float fFrequency = 0.1f;
			if ( i*i+j*j != 0.0f )
			{
				waterData[this->calcY * i + j].height = 80.0f * sinf( sqrt( (float)(i*i+j*j) ) * fFrequency ) / ( sqrt( (float)(i*i+j*j) ) * fFrequency );
				waterData[this->calcY * i + j].pipes = D3DXVECTOR4(0, 0, 0, 0);
			}
			else
			{
				waterData[this->calcY * i + j].height = 80.0f;
				waterData[this->calcY * i + j].pipes = D3DXVECTOR4(0, 0, 0, 0);
			}
		}
	}

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = totalCalcPoints * sizeof(WaterData);
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(WaterData);
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = waterData;

	HRESULT hr = S_OK;

	//Create buffers
	hr = device->CreateBuffer(&desc, &data, &buffers[0]);
	if(FAILED(hr))
		return false;

	hr = device->CreateBuffer(&desc, &data, &buffers[1]);
	if(FAILED(hr))
		return false;

	SAFE_DELETE_ARRAY(waterData);


	hr = device->CreateShaderResourceView(buffers[0], NULL, &SRV[0]);
	if(FAILED(hr))
		return false;

	hr = device->CreateShaderResourceView(buffers[1], NULL, &SRV[1]);
	if(FAILED(hr))
		return false;


	hr = device->CreateUnorderedAccessView(buffers[0], NULL, &UAV[0]);
	if(FAILED(hr))
		return false;

	hr = device->CreateUnorderedAccessView(buffers[1], NULL, &UAV[1]);
	if(FAILED(hr))
		return false;

	cBuffer.timeFactors = D3DXVECTOR4(0, 0, 0, 0);
	cBuffer.dispatchSize = D3DXVECTOR4(0, 0, 0, 0);

	D3D11_BUFFER_DESC desc2;
	desc2.ByteWidth = sizeof(ComputeShaderConstantBuffer);
	desc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc2.MiscFlags = 0;
	desc2.StructureByteStride = 0;
	desc2.Usage = D3D11_USAGE_DEFAULT;
	desc2.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = &cBuffer;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&desc2, &initData, &constantBuffer);
	if(FAILED(hr))
		return false;
		
	/*
		Tess buffrar
	*/
	tessVertexCount = detailed * detailed;
	PosUV* tessMeshData = new PosUV[tessVertexCount];

	float posJumpsX = (float)sizeX / (detailed-1);
	float posJumpsY = (float)sizeY / (detailed-1);
	float uvJumps = (float)this->calcX / (detailed-1);
	for(int i = 0; i < detailed; i++)
	{
		for(int j = 0; j < detailed; j++)
		{
			float uvX, uvY;
			uvX = i * uvJumps;
			uvY = j * uvJumps;

			if(uvX - 1 > 0)
				uvX -= 1;

			if(uvY - 1 > 0)
				uvY -= 1;

			tessMeshData[i * detailed + j].pos = D3DXVECTOR3(i * posJumpsX, j * posJumpsY, 0);
			//tessMeshData[i * detailed + j].normal = D3DXVECTOR3(0, 0, 1);
			tessMeshData[i * detailed + j].uv = D3DXVECTOR2(uvX, uvY);
		}
	}

	tessIndexCount = (detailed-1) * (detailed-1) * 6;
	int* tessIndexData = new int[tessIndexCount];

	int ind = 0;
	for(int i = 0; i < detailed-1; i++)
	{
		for(int j = 0; j < detailed-1; j++)
		{
			tessIndexData[ind] = i * detailed + j;
			tessIndexData[ind+1] = i * detailed + j+1;
			tessIndexData[ind+2] = (i+1) * detailed + j+1;

			tessIndexData[ind+3] = i * detailed + j;
			tessIndexData[ind+4] = (i+1) * detailed + j+1;
			tessIndexData[ind+5] = (i+1) * detailed + j;

			ind += 6;
		}
	}

	tessMesh = new Buffer();
	BUFFER_INIT_DESC meshDesc2;
	meshDesc2.InitData = tessMeshData;
	meshDesc2.ElementSize = sizeof(PosUV);
	meshDesc2.NumElements = tessVertexCount;
	meshDesc2.Usage = BUFFER_DEFAULT;
	meshDesc2.Type = VERTEX_BUFFER;
	
	hr = tessMesh->Init(device, deviceContext, meshDesc2);
	SAFE_DELETE_ARRAY(tessMeshData);
	if(FAILED(hr))
		return false;

	tessIndex = new Buffer();
	BUFFER_INIT_DESC meshDesc3;
	meshDesc3.Type = INDEX_BUFFER;
	meshDesc3.Usage = BUFFER_DEFAULT;
	meshDesc3.ElementSize = sizeof(int);
	meshDesc3.InitData = tessIndexData;
	meshDesc3.NumElements = tessIndexCount;

	hr = tessIndex->Init(device, deviceContext, meshDesc3);
	SAFE_DELETE_ARRAY(tessIndexData);
	if(FAILED(hr))
		return false;

	//create water texture
	waterTexture = new Texture();
	if(!waterTexture->init(device, "../shaders/water.jpg"))
		return false;

	return true;
}

void WaterSimulation::update(ID3D11DeviceContext* deviceContext, float dt)
{
	//Update constant buffer
	cBuffer.timeFactors.x = dt;
	cBuffer.dispatchSize = D3DXVECTOR4(threadGroupX, threadGroupY, calcX, calcY);
	deviceContext->UpdateSubresource(constantBuffer, 0, 0, &cBuffer, 0, 0);

	deviceContext->CSSetShaderResources(0, 1, &SRV[0]);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &UAV[1], 0);
	deviceContext->CSSetConstantBuffers(0, 1, &constantBuffer);
	deviceContext->CSSetShader(shader, NULL, 0);
	deviceContext->Dispatch(threadGroupX, threadGroupY, 1);

	ID3D11ShaderResourceView* nullSRV = NULL;
	ID3D11UnorderedAccessView* nullUAV = NULL;

	deviceContext->CSSetShaderResources(0, 1, &nullSRV);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, 0);
	deviceContext->CSSetShader(NULL, NULL, 0);

	//Swap resources
	ID3D11ShaderResourceView* tempSRV = SRV[0];
	SRV[0] = SRV[1];
	SRV[1] = tempSRV;

	ID3D11UnorderedAccessView* tempUAV = UAV[0];
	UAV[0] = UAV[1];
	UAV[1] = tempUAV;
}

void WaterSimulation::render(ID3D11DeviceContext* deviceContext, D3DXMATRIX wvp, float tessFactor, D3DXVECTOR4* frustrumPlaneEquation)
{
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	tessMesh->Apply();
	tessIndex->Apply();
	D3DXMATRIX world, trans;
	D3DXMatrixTranslation(&trans, 0, 0, 0);
	D3DXMatrixIdentity(&world);
	D3DXMatrixRotationX(&world, -3.14/2);
	world = position * world;

	renderShader->SetMatrix("gW", world);
	renderShader->SetMatrix("gVP", wvp);
	renderShader->SetFloat("dispatchSize", calcY);
	renderShader->SetFloat("tessFactor", tessFactor);
	renderShader->SetResource("currentState", SRV[0]);
	renderShader->SetResource("waterTexture", *waterTexture->getSRV());
	renderShader->SetRawData("frustrumPlaneEquation", frustrumPlaneEquation, sizeof(D3DXVECTOR4) * 4);
	renderShader->Apply(0);

	deviceContext->DrawIndexed(tessIndexCount, 0, 0);
	ID3D11ShaderResourceView* nullSRV[1] = {NULL};
	renderShader->SetResource("currentState", nullSRV[0]);
}

void WaterSimulation::setPosition(D3DXVECTOR3 pos)
{
	D3DXMatrixTranslation(&position, pos.x, pos.y, pos.z);
}