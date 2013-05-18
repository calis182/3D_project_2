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


Terrain* g_Terrain = NULL;
Input* input = NULL;

Camera* camera = NULL;

ParticleSystem* particleSystem = NULL;

PointLight* light;

ObjMesh* objMesh;

SkyBox* skyBox;

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
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
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
	D3D11_VIEWPORT vp;
	vp.Width = (float)screenWidth;
	vp.Height = (float)screenHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_DeviceContext->RSSetViewports( 1, &vp );


	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	skyBox = new SkyBox(g_Device, g_DeviceContext);
	skyBox->init();

	objMesh = new ObjMesh(g_Device, g_DeviceContext);
	objMesh->initiate("..\\Shaders\\obj_mesh\\bth.obj");

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

	light = new PointLight(D3DXVECTOR4(0.2, 0.2, 0.2, 0.2), D3DXVECTOR4(0.2, 0.2, 0.2, 0.2), D3DXVECTOR4(0.2, 0.2, 0.2, 0.2), D3DXVECTOR3(-128, 128, 128), 500);

	camera = new Camera();

	particleSystem = new ParticleSystem();
	particleSystem->Init(g_Device, g_DeviceContext);

	camera->SetLens((float)D3DX_PI * 0.45f, 1024.0f/768.0f, 0.1f, 1000.0f);

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
			if(flyModeOn)
			{
				flyModeOn=false;
					
			}
			else
			{
				flyModeOn=true;
			}
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

	particleSystem->Update(deltaTime, frames, *camera);
	skyBox->update(camera->GetPosition());

	return S_OK;
}

HRESULT Render(float deltaTime)
{
	frames++;

	//clear render target
	static float ClearColor[4] = { 0, 0, 0, 0.0f };
	g_DeviceContext->ClearRenderTargetView( g_RenderTargetView, ClearColor );

	//clear depth info
	g_DeviceContext->ClearDepthStencilView( g_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

	//set topology
	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//calculate WVP matrix
	static float rotY = 0.0f;
	D3DXMATRIX world, view = camera->View(), proj = camera->Proj(), wvp;
	D3DXMatrixRotationY(&world, 0);


	camera->UpdateViewMatrix();
	
	g_Terrain->render(g_DeviceContext, world, view, proj, camera->GetPosition(), *light);
	particleSystem->Draw(g_DeviceContext, world, view, proj);
	objMesh->billboard(camera->GetPosition());
	objMesh->render(view, proj, camera->GetPosition(), *light);
	skyBox->render(view*proj);
	

	char title[100];
	sprintf_s(title, sizeof(title),  "%f", 1/deltaTime);
	SetWindowText(g_hWnd, title);

	if(FAILED(g_SwapChain->Present(0, 0)))
		return E_FAIL;

	return S_OK;
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