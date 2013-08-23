//--------------------------------------------------------------------------------------
// File: TemplateMain.cpp
//
// BTH-D3D-Template
//
// Copyright (c) Stefan Petersson 2011. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"

#include <ctime>

#include "Shader.h"
#include "Buffer.h"

#include "HeightMap.h"
#include "Terrain.h"

#include "TextureClass.h"

#include "Camera.h"
#include "Input.h"

#include "ParticleSystem.h"
#include "SkyBox.h"
#include "ObjMesh.h"
#include "DynamicCubeMap.h"
#include "BlendState.h"

#include "WaterShader.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE				g_hInst					= NULL;  
HWND					g_hWnd					= NULL;

IDXGISwapChain*         g_SwapChain				= NULL;
ID3D11RenderTargetView* g_RenderTargetView		= NULL;
ID3D11Texture2D*        g_DepthStencil			= NULL;
ID3D11DepthStencilView* g_DepthStencilView		= NULL;
ID3D11Device*			g_Device				= NULL;
ID3D11DeviceContext*	g_DeviceContext			= NULL;

D3D11_VIEWPORT vp;

Shader* debugTextureShader;
Shader* r_Shader;
WaterShader* m_WaterShader;
float m_waterHeight;
float m_waterTranslation;
ID3D11ShaderResourceView* refractionTexture;
ID3D11ShaderResourceView* reflectionTexture;
ID3D11RenderTargetView* refractionTargetView;
ID3D11RenderTargetView* reflectionTargetView;

Terrain* g_Terrain = NULL;
Input* input = NULL;

Camera* camera = NULL;

ParticleSystem* particleSystem = NULL;

PointLight* light;

ObjMesh* object;

SkyBox* skyBox;
DynamicCubeMap* cubeMap;
const float waterHeight = 5.0f;        



__int64 frames = 0;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT             InitWindow( HINSTANCE hInstance, int nCmdShow );
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT				Render(float deltaTime);
HRESULT				Update(float deltaTime);
HRESULT				InitDirect3D();
char*				FeatureLevelToString(D3D_FEATURE_LEVEL featureLevel);
bool RenderRefractionToTexture();
bool RenderReflectonToTexture();
bool RenderScene();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	srand((int)time(NULL));

	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
		return 0;

	if( FAILED( InitDirect3D() ) )
		return 0;

	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	float secsPerCnt = 1.0f / (float)cntsPerSec;

	__int64 prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);

	// Main message loop
	MSG msg = {0};
	while(WM_QUIT != msg.message)
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			__int64 currTimeStamp = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
			float dt = (currTimeStamp - prevTimeStamp) * secsPerCnt;

			//render
			if(Update(dt) == 1)
				break;

			Render(dt);

			prevTimeStamp = currTimeStamp;
		}
	}

	delete particleSystem;
	delete g_Terrain;
	delete skyBox;
	delete input;
	delete camera;
	delete light;
	delete cubeMap;
	delete object;
	m_WaterShader->Shutdown();
	delete m_WaterShader;
	BlendState::getInstance()->shutdown();
	delete debugTextureShader;

	g_Device->Release();
	g_DeviceContext->Release();
	g_SwapChain->Release();
	g_RenderTargetView->Release();
	g_DepthStencil->Release();
	g_DepthStencilView->Release();

	return (int) msg.wParam;
}

//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = 0;
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = "BTH_D3D_Template";
	wcex.hIconSm        = 0;
	if( !RegisterClassEx(&wcex) )
		return E_FAIL;

	// Adjust and create window
	g_hInst = hInstance; 
	RECT rc = { 0, 0, 1024, 768 };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	
	if(!(g_hWnd = CreateWindow(
							"BTH_D3D_Template",
							"BTH - Direct3D 11.0 Template",
							WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							rc.right - rc.left,
							rc.bottom - rc.top,
							NULL,
							NULL,
							hInstance,
							NULL)))
	{
		return E_FAIL;
	}

	ShowWindow( g_hWnd, nCmdShow );

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDirect3D()
{
	srand((int)time(NULL));
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect( g_hWnd, &rc );
	int screenWidth = rc.right - rc.left;
	int screenHeight = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverType;

	D3D_DRIVER_TYPE driverTypes[] = 
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = sizeof(driverTypes) / sizeof(driverTypes[0]); //same as ARRAYSIZE(x) macro

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof(sd) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = screenWidth;
	sd.BufferDesc.Height = screenHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevelsToTry[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	D3D_FEATURE_LEVEL initiatedFeatureLevel;

	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,
			driverType,
			NULL,
			createDeviceFlags,
			featureLevelsToTry,
			ARRAYSIZE(featureLevelsToTry),
			D3D11_SDK_VERSION,
			&sd,
			&g_SwapChain,
			&g_Device,
			&initiatedFeatureLevel,
			&g_DeviceContext);

		if( SUCCEEDED( hr ) )
		{
			char title[256];
			sprintf_s(
				title,
				sizeof(title),
				"BTH - Direct3D 11.0 Template | Direct3D 11.0 device initiated with Direct3D %s feature level",
				FeatureLevelToString(initiatedFeatureLevel)
			);
			SetWindowText(g_hWnd, title);

			break;
		}
	}
	if( FAILED(hr) )
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer;
	hr = g_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer );
	if( FAILED(hr) )
		return hr;

	hr = g_Device->CreateRenderTargetView( pBackBuffer, NULL, &g_RenderTargetView );
	pBackBuffer->Release();
	if( FAILED(hr) )
		return hr;


	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = screenWidth;
	descDepth.Height = screenHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = g_Device->CreateTexture2D( &descDepth, NULL, &g_DepthStencil );
	if( FAILED(hr) )
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = g_Device->CreateDepthStencilView( g_DepthStencil, &descDSV, &g_DepthStencilView );
	if( FAILED(hr) )
		return hr;
	



	g_DeviceContext->OMSetRenderTargets( 1, &g_RenderTargetView, g_DepthStencilView );

	// Setup the viewport
	vp.Width = (float)screenWidth;
	vp.Height = (float)screenHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_DeviceContext->RSSetViewports( 1, &vp );

	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	debugTextureShader = new Shader();
	debugTextureShader->Init(g_Device, g_DeviceContext, "..\\Shaders\\DebugTexture.fx", inputDesc, 2);
	

	skyBox = new SkyBox(g_Device, g_DeviceContext);
	skyBox->init();

	object = new ObjMesh(g_Device, g_DeviceContext);
	object->initiate("..\\Shaders\\obj_mesh\\bth.obj");

	g_Terrain = new Terrain();
	if(!g_Terrain->init(g_Device, g_DeviceContext))
	{
		return E_FAIL;
	}
	
 	
	input = new Input();
	if(!input->init(g_hInst, g_hWnd, 1920, 1080))
	{
		MessageBox(NULL, "Could not init input", NULL, MB_OK);
	}

	float colorCorr = 1000;
	light = new PointLight(D3DXVECTOR4(252/colorCorr, 214/colorCorr, 103/colorCorr, 1), D3DXVECTOR4(252/colorCorr, 214/colorCorr, 103/colorCorr, 1), D3DXVECTOR4(252/colorCorr, 214/colorCorr, 103/colorCorr, 1), D3DXVECTOR3(-128, 128, 128), 500);
	
	camera = new Camera();
	camera->SetLens((float)D3DX_PI * 0.45f, 1024.0f/768.0f, 0.1f, 1000.0f);
	camera->UpdateViewMatrix();

	particleSystem = new ParticleSystem();
	particleSystem->Init(g_Device, g_DeviceContext);


	cubeMap = new DynamicCubeMap(g_Device);
	cubeMap->init();
	cubeMap->setUpCameras(D3DXVECTOR3(0, 20, 0));

	BlendState::getInstance()->createBlendState(g_Device);

	bool result;

	// Create the water shader object.
	m_WaterShader = new WaterShader;
	if(!m_WaterShader)
	{
		return false;
	}

	// Initialize the water shader object.
	result = m_WaterShader->Initialize(g_Device, g_DeviceContext, g_hWnd);
	if(!result)
	{
		MessageBox(g_hWnd, "Could not initialize the water shader object.", "Error", MB_OK);
		return false;
	}
	result = m_WaterShader->InitializeShader(g_Device, g_DeviceContext);
	// Set the height of the water.
	m_waterHeight = 2.75f;

	// Initialize the position of the water.
	m_waterTranslation = 0.0f;

	return S_OK;
}

HRESULT Update(float deltaTime)
{
	input->frame();

	D3DXVECTOR3 oldPos = camera->GetPosition();

	static bool flyModeOn = false;
	static float time = 0;
	time += deltaTime;
	
	if(input->isKeyPressed(DIK_W))	//W
		camera->Walk(30.0f * deltaTime);

	if(input->isKeyPressed(DIK_A))	//A
		camera->Strafe(-30.0f * deltaTime);

	if(input->isKeyPressed(DIK_S))	//S
		camera->Walk(-30.0f * deltaTime);

	if(input->isKeyPressed(DIK_D))	//D
		camera->Strafe(30.0f * deltaTime);

	if(input->isKeyPressed(DIK_ESCAPE))
		return 1;

	if(input->isKeyPressed(DIK_SPACE) && time > 0.50) //SPACE
	{
		time = 0;
		flyModeOn = !flyModeOn;
	}

	int x, y;
	input->getDiffMouseLocation(x, y);
	
	camera->Yaw((float)x*0.5f);
	camera->Pitch((float)y*0.5f);

	D3DXVECTOR3 pos = camera->GetPosition();
	if(!flyModeOn)
	{
		pos.y = g_Terrain->getY(pos.x, pos.z) + 10;
		oldPos.y = pos.y;
	}
	else
	{
		if(pos.y<g_Terrain->getY(pos.x, pos.z) + 10)
		{
			pos.y = g_Terrain->getY(pos.x, pos.z) + 10;
		}
	}

	if(!g_Terrain->isInBounds((int)pos.x, (int)pos.z))
		camera->SetPosition(oldPos);
	else
		camera->SetPosition(pos);

	camera->UpdateViewMatrix();

	particleSystem->Update(deltaTime, frames, *camera);
	skyBox->update(camera->GetPosition());

	// Update the position of the water to simulate motion.
	m_waterTranslation += 0.001f;
	if(m_waterTranslation > 1.0f)
	{
		m_waterTranslation -= 1.0f;
	}

	return S_OK;
}

HRESULT Render(float deltaTime)
{
	frames++;
	Camera cameraTemp;
	ID3D11RenderTargetView* renderTargets[1];
	D3DXMATRIX world, view = camera->View(), proj = camera->Proj(), wvp;
	D3DXMatrixIdentity(&world);
	wvp = world * view * proj;

	//clear render target
	static float ClearColor[4] = { 0, 0, 0, 0.0f };
	//set topology
	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_DeviceContext->RSSetViewports(1, &cubeMap->getViewPort());
	static bool Switch = true;

	skyBox->update(cubeMap->getPosition());


	BlendState::getInstance()->setState(0, g_DeviceContext);
	//Render for all 6 cameras 
	for(int i = 0; i < 6; i++)
	{
		//if(Switch)
		//{
			//Clear render target view and depth stencil view
			g_DeviceContext->ClearRenderTargetView(cubeMap->getRenderTargetView(i), ClearColor);
			g_DeviceContext->ClearDepthStencilView(cubeMap->getDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			//Bind render target view
			renderTargets[0] = cubeMap->getRenderTargetView(i);
			g_DeviceContext->OMSetRenderTargets(1, renderTargets, cubeMap->getDepthStencilView());

			cameraTemp = cubeMap->getCamera(i);
			view = cameraTemp.View();
			proj = cameraTemp.Proj();
			
			BlendState::getInstance()->setState(1, g_DeviceContext);
			skyBox->render(view * proj, skyBox->getCubeMap());
			BlendState::getInstance()->setState(0, g_DeviceContext);
			g_Terrain->render(g_DeviceContext, world, view, proj, cubeMap->getPosition(), *light, skyBox->getCubeMap());

			BlendState::getInstance()->setState(0, g_DeviceContext);
			particleSystem->Draw(g_DeviceContext, world, view, proj);
		//}
		Switch = !Switch;
	}
	Switch = !Switch;
	g_DeviceContext->GenerateMips(cubeMap->getCubeMap());

	//calculate WVP matrix
	view = camera->View();
	proj = camera->Proj();
	
	renderTargets[0] = g_RenderTargetView;
	
	//clear render target
	//clear depth info
	g_DeviceContext->ClearRenderTargetView( g_RenderTargetView, ClearColor );
	g_DeviceContext->ClearDepthStencilView( g_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

	g_DeviceContext->RSSetViewports(1, &vp);
	g_DeviceContext->OMSetRenderTargets(1, renderTargets, g_DepthStencilView);

	skyBox->update(camera->GetPosition());
	BlendState::getInstance()->setState(0, g_DeviceContext);
	skyBox->render(view * proj, skyBox->getCubeMap());
	
	g_Terrain->render(g_DeviceContext, world, view, proj, camera->GetPosition(), *light, skyBox->getCubeMap());
	
	//objMesh->billboard(camera->GetPosition());
	BlendState::getInstance()->setState(1, g_DeviceContext);
	object->render(view, proj, camera->GetPosition(), *light, cubeMap->getCubeMap());
	
	BlendState::getInstance()->setState(0, g_DeviceContext);
	particleSystem->Draw(g_DeviceContext, world, view, proj);

	RenderRefractionToTexture();
	RenderReflectonToTexture();
	RenderScene();


	char title[100];
	sprintf_s(title, sizeof(title),  "%f", 1/deltaTime);
	SetWindowText(g_hWnd, title);

	if(FAILED(g_SwapChain->Present(0, 0)))
		return E_FAIL;

	return S_OK;
}


bool RenderRefractionToTexture()
{
	D3DXVECTOR4 clipPlane;
	D3DXMATRIX world, view, proj;
	bool result;

	//Setup a clipping plane based haeight of the water to clip everything above it.
	clipPlane = D3DXVECTOR4(0.0f, -1.0f, 0.0f, waterHeight + 0.1f);
	
	//Set the render target to be the refraction render to texture.
	g_DeviceContext->OMSetRenderTargets( 1, &refractionTargetView, g_DepthStencilView );
	
	float color[4];
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;
	color[3] = 1;
	//Clear the  to texture.
	g_DeviceContext->ClearRenderTargetView( refractionTargetView, color);

	D3DXMatrixIdentity(&world);
	view = camera->View();
	proj = camera->Proj();

	D3DXMatrixTranslation(&world, 0, 2, 0);
	
	skyBox->render(view * proj, skyBox->getCubeMap());

	m_WaterShader->SetRefractionParameters(world, view, proj, clipPlane, light->getAmbient(), light->getDiffuse(), light->getPosition());
	m_WaterShader->RenderRefraction(g_Device, g_DeviceContext, refractionTexture);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	g_DeviceContext->OMSetRenderTargets(1, &refractionTargetView, g_DepthStencilView);

	return true;
}

bool RenderReflectonToTexture()
{
	D3DXMATRIX reflectionMatrix, world, proj, view;
	bool result;

	//Set the render target to be the reflection render to texture.
	g_DeviceContext->OMSetRenderTargets( 1, &reflectionTargetView, g_DepthStencilView );

	float color[4];
	color[0] = 0;
	color[1] = 0;
	color[2] = 0;
	color[3] = 1;
	//Clear the reflection to texture.
	g_DeviceContext->ClearRenderTargetView( reflectionTargetView, color);

	// Use the camera to render the reflection and create a reflection view matrix.
	// Get the camera reflection view matrix instead of the normal view matrix.
	reflectionMatrix = camera->RenderReflection(m_waterHeight);

	D3DXMatrixIdentity(&world);
	view = camera->View();
	proj = camera->Proj();

	D3DXMatrixTranslation(&world, 0, 6, 8);

	g_Terrain->render(g_DeviceContext, world, view, proj, camera->GetPosition(), *light, skyBox->getCubeMap());
	
	m_WaterShader->SetReflectionParameters(world, reflectionMatrix, proj, light->getAmbient(), light->getDiffuse(), light->getPosition());
	m_WaterShader->RenderReflection(g_Device, g_DeviceContext, reflectionTexture);
	
	// Reset the render target back to the original back buffer and not the render to texture anymore.
	g_DeviceContext->OMSetRenderTargets(1, &reflectionTargetView, g_DepthStencilView);

	return true;
}

bool RenderScene()
{
	D3DXMATRIX world, view, proj, reflectionMatrix;
	bool result;

	float color[4];
	
	// Setup the color to clear the buffer to.
	color[0] = 0.0f;
	color[1] = 0.0f;
	color[2] = 0.0f;
	color[3] = 1.0f;


	//Set the render target to be the reflection render to texture.
	g_DeviceContext->OMSetRenderTargets( 1, &g_RenderTargetView, g_DepthStencilView );
    
	D3DXMatrixIdentity(&world);
	view = camera->View();
	proj = camera->Proj();

	reflectionMatrix = camera->RenderReflection(m_waterHeight);

	D3DXMatrixTranslation(&world, 0, m_waterHeight, 0);
	
	BlendState::getInstance()->setState(0, g_DeviceContext);
	m_WaterShader->SetWaterParameters(world, reflectionMatrix, view, proj, light->getAmbient(), light->getDiffuse(), light->getPosition(), m_waterTranslation, 0.01f);
	m_WaterShader->RenderWater(g_Device, g_DeviceContext, refractionTexture, reflectionTexture);
	
	return true;
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:

		switch(wParam)
		{
			case VK_ESCAPE:
				PostQuitMessage(0);
				break;
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

char* FeatureLevelToString(D3D_FEATURE_LEVEL featureLevel)
{
	if(featureLevel == D3D_FEATURE_LEVEL_11_0)
		return "11.0";
	if(featureLevel == D3D_FEATURE_LEVEL_10_1)
		return "10.1";
	if(featureLevel == D3D_FEATURE_LEVEL_10_0)
		return "10.0";

	return "Unknown";
}