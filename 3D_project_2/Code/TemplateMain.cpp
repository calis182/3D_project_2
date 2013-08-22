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
#include "GaussianBlur.h"

void renderCubeMap(void* param);
void extractPlanesFromFrustrum(D3DXVECTOR4* planeEquation, const D3DXMATRIX* viewProj, bool normalize = true);
void normalizePlane(D3DXVECTOR4* planeEquation);

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

ID3D11Texture2D*		mainTexture2D = NULL; 
ID3D11RenderTargetView* mainRTV		= NULL;
ID3D11ShaderResourceView* mainSRV = NULL;
ID3D11UnorderedAccessView* mainUAV = NULL;
Buffer* fullscreenQuad;

D3DXVECTOR4 frustrumPlaneEquation[6];

D3D11_VIEWPORT vp;

Shader* debugTextureShader;

Terrain* g_Terrain = NULL;
Input* input = NULL;

Camera* camera = NULL;

ParticleSystem* particleSystem = NULL;

PointLight* light;

ObjMesh* object;

SkyBox* skyBox;
DynamicCubeMap* cubeMap;
GaussianBlur* gaussianBlur;

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

#include <vld.h>
#include <fcntl.h>
#include <io.h>

void SetStdOutToNewConsole()
{
    // allocate a console for this app
    AllocConsole();

    // redirect unbuffered STDOUT to the console
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    int fileDescriptor = _open_osfhandle((intptr_t)consoleHandle, _O_TEXT);
    FILE *fp = _fdopen( fileDescriptor, "w" );
    *stdout = *fp;
    setvbuf( stdout, NULL, _IONBF, 0 );
	
    // give the console window a nicer title
	char str[256];
	sprintf_s(str, "Debug Output");

    SetConsoleTitle(str);

    // give the console window a bigger buffer size
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if ( GetConsoleScreenBufferInfo(consoleHandle, &csbi) )
    {
        COORD bufferSize;
        bufferSize.X = csbi.dwSize.X;
        bufferSize.Y = 50;
        SetConsoleScreenBufferSize(consoleHandle, bufferSize);
    }
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	srand((int)time(NULL));

	SetStdOutToNewConsole();

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

	delete gaussianBlur;
	delete particleSystem;
	delete g_Terrain;
	delete skyBox;
	delete input;
	delete camera;
	delete light;
	delete cubeMap;
	delete object;
	BlendState::getInstance()->shutdown();
	
	SAFE_DELETE(fullscreenQuad);

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
	RECT rc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
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
	hr = g_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer);
	if( FAILED(hr) )
		return hr;

	hr = g_Device->CreateRenderTargetView( pBackBuffer, NULL, &g_RenderTargetView );

	pBackBuffer->Release();
	if( FAILED(hr) )
		return hr;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = SCREEN_WIDTH;
	descDepth.Height = SCREEN_HEIGHT;
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

	//Create renderTarget, needed for the blur.
	D3D11_TEXTURE2D_DESC texDesc;	
	texDesc.Width				= SCREEN_WIDTH;
	texDesc.Height				= SCREEN_HEIGHT;
	texDesc.MipLevels			= 1;
	texDesc.ArraySize			= 1;
	texDesc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count	= 1;
	texDesc.SampleDesc.Quality	= 0;
	texDesc.Usage				= D3D11_USAGE_DEFAULT;
	texDesc.BindFlags			= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags		= 0;
	texDesc.MiscFlags			= 0;

	hr = g_Device->CreateTexture2D(&texDesc, NULL, &mainTexture2D);
	hr = g_Device->CreateRenderTargetView(mainTexture2D, NULL, &mainRTV);
	hr = g_Device->CreateShaderResourceView(mainTexture2D, NULL, &mainSRV);
	hr = g_Device->CreateUnorderedAccessView(mainTexture2D, NULL, &mainUAV);

	struct Vertex
	{
		D3DXVECTOR3 pos;
		D3DXVECTOR2 tex;

		Vertex()
		{
			pos = D3DXVECTOR3(0, 0, 0);
			tex = D3DXVECTOR2(0, 0);
		}

		Vertex(D3DXVECTOR3 p, D3DXVECTOR2 t)
		{
			pos = p;
			tex = t;
		}
	};

	//Quad for fullscreen quad
	Vertex quad[4];
	quad[0].pos = D3DXVECTOR3(1, 1, 0);
	quad[0].tex = D3DXVECTOR2(1, 0);
	quad[1].pos = D3DXVECTOR3(-1, 1, 0);
	quad[1].tex = D3DXVECTOR2(0, 0);
	quad[2].pos = D3DXVECTOR3(1, -1, 0);
	quad[2].tex = D3DXVECTOR2(1, 1);
	quad[3].pos = D3DXVECTOR3(-1, -1, 0);
	quad[3].tex = D3DXVECTOR2(0, 1);

	BUFFER_INIT_DESC bufferDesc;
	bufferDesc.ElementSize = sizeof(Vertex);
	bufferDesc.InitData = quad;
	bufferDesc.NumElements = 4;
	bufferDesc.Type = VERTEX_BUFFER;
	bufferDesc.Usage = BUFFER_DEFAULT;

	fullscreenQuad = new Buffer();
	fullscreenQuad->Init(g_Device, g_DeviceContext, bufferDesc);

	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	debugTextureShader = new Shader();
	debugTextureShader->Init(g_Device, g_DeviceContext, "..\\Shaders\\DebugTexture.fx", inputDesc, 2);

	gaussianBlur = new GaussianBlur();
	gaussianBlur->init(g_Device, g_DeviceContext);

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
	if(!input->init(g_hInst, g_hWnd, SCREEN_WIDTH, SCREEN_HEIGHT))
	{
		MessageBox(NULL, "Could not init input", NULL, MB_OK);
	}

	float colorCorr = 1000;
	light = new PointLight(D3DXVECTOR4(252/colorCorr, 214/colorCorr, 103/colorCorr, 1), D3DXVECTOR4(252/colorCorr, 214/colorCorr, 103/colorCorr, 1), D3DXVECTOR4(252/colorCorr, 214/colorCorr, 103/colorCorr, 1), D3DXVECTOR3(-128, 128, 128), 500);
	
	camera = new Camera();
	camera->SetLens((float)D3DX_PI * 0.45f, (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 1000.0f);
	camera->UpdateViewMatrix();

	particleSystem = new ParticleSystem();
	particleSystem->Init(g_Device, g_DeviceContext);

	cubeMap = new DynamicCubeMap(g_Device);
	cubeMap->init();
	cubeMap->setUpCameras(D3DXVECTOR3(0, 20, 0));

	BlendState::getInstance()->createBlendState(g_Device);

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

	return S_OK;
}

HRESULT Render(float deltaTime)
{
	frames++;
	Camera cameraTemp;
	ID3D11RenderTargetView* renderTargets[1];
	D3DXMATRIX world, view = camera->View(), proj = camera->Proj(), wvp, viewProj;
	D3DXMatrixIdentity(&world);
	wvp = world * view * proj;
	
	//clear render target
	static float ClearColor[4] = { 0, 0, 0, 0.0f };
	//set topology
	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	D3DXVECTOR3 diff = cubeMap->getPosition() - camera->GetPosition();
	float length = sqrt(D3DXVec3Dot(&diff, &diff));
	length = (1 - length / 150.0f) * 150.0f + length / 150.0f + 15;
	length *= 0.02f;

	//Clamp
	length = length > 3 ? 3 : length;
	length = length < 0 ? 0 : length;

	g_DeviceContext->RSSetViewports(1, &cubeMap->getViewPort(length));
	
	static int blurPasses = 0;
	float tessFactor = 3.0f;
	for(int i = 0; i <= 9; i++)
	{
		if(GetAsyncKeyState(char(i) + '0'))
			tessFactor = i;
	}

	if(GetAsyncKeyState('O'))
		blurPasses++;
	else if(GetAsyncKeyState('L'))
		blurPasses--;
	
	if(blurPasses < 0)
		blurPasses = 0;
	else if(blurPasses > 5)
		blurPasses = 5;

	if(GetAsyncKeyState('M'))
		tessFactor = 64;
	
	//Render for all 6 cameras 
	skyBox->update(cubeMap->getPosition());
	for(int i = 0; i < 6; i++)
	{
		//Clear render target view and depth stencil view
		g_DeviceContext->ClearRenderTargetView(cubeMap->getRenderTargetView(i, length), ClearColor);
		g_DeviceContext->ClearDepthStencilView(cubeMap->getDepthStencilView(length), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		//Bind render target view
		renderTargets[0] = cubeMap->getRenderTargetView(i, length);
		g_DeviceContext->OMSetRenderTargets(1, renderTargets, cubeMap->getDepthStencilView(length));

		cameraTemp = cubeMap->getCamera(i);
		view = cameraTemp.View();
		proj = cameraTemp.Proj();
		viewProj = view * proj;

		extractPlanesFromFrustrum(frustrumPlaneEquation, &viewProj);

		skyBox->render(view * proj, skyBox->getCubeMap());
		g_Terrain->render(g_DeviceContext, world, view, proj, cubeMap->getPosition(), *light, skyBox->getCubeMap(), tessFactor, frustrumPlaneEquation);

		particleSystem->Draw(g_DeviceContext, world, view, proj);
	}
	g_DeviceContext->GenerateMips(cubeMap->getCubeMap(length));
	
	//calculate WVP matrix
	view = camera->View();
	proj = camera->Proj();
	viewProj = view * proj;

	extractPlanesFromFrustrum(frustrumPlaneEquation, &viewProj);

	renderTargets[0] = mainRTV;
	
	//clear render target
	//clear depth info
	g_DeviceContext->ClearRenderTargetView( g_RenderTargetView, ClearColor );
	g_DeviceContext->ClearDepthStencilView( g_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );

	g_DeviceContext->RSSetViewports(1, &vp);
	g_DeviceContext->OMSetRenderTargets(1, renderTargets, g_DepthStencilView);

	skyBox->update(camera->GetPosition());
	skyBox->render(view * proj, skyBox->getCubeMap());

	ID3D11Query* query = NULL;
	D3D11_QUERY_DESC qd;
	qd.Query = D3D11_QUERY_PIPELINE_STATISTICS;
	qd.MiscFlags = 0;
	if(FAILED(g_Device->CreateQuery(&qd, &query)))
		return E_FAIL;

	g_DeviceContext->Begin(query);
	
	g_Terrain->render(g_DeviceContext, world, view, proj, camera->GetPosition(), *light, mainSRV, tessFactor, frustrumPlaneEquation);

	g_DeviceContext->End(query);

	D3D11_QUERY_DATA_PIPELINE_STATISTICS data;
	while(g_DeviceContext->GetData(query, &data, sizeof(data), 0) != S_OK);

	SAFE_RELEASE(query);
	
	BlendState::getInstance()->setState(1, g_DeviceContext);
	object->render(view, proj, camera->GetPosition(), *light, cubeMap->getCubeMap(length));
	
	BlendState::getInstance()->setState(0, g_DeviceContext);
	particleSystem->Draw(g_DeviceContext, world, view, proj);

	//Gaussian blur
	renderTargets[0] = g_RenderTargetView;
	g_DeviceContext->OMSetRenderTargets(1, renderTargets, g_DepthStencilView);
	gaussianBlur->blur(g_DeviceContext, blurPasses, mainSRV, mainUAV);

	//Fullscreen pass
	g_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	fullscreenQuad->Apply(0);
	debugTextureShader->SetResource("texture1", mainSRV);
	debugTextureShader->Apply(0);
	g_DeviceContext->Draw(4, 0);

	char title[100];
	int fps = 1/deltaTime;
	sprintf_s(title, sizeof(title),  "FPS: %d Dynamic cube: %f %d Triangles: %d", fps, length, particleSystem->getTotalNumOfParticles(), data.CInvocations);
	SetWindowText(g_hWnd, title);
	if(FAILED(g_SwapChain->Present(0, 0)))
		return E_FAIL;

	return S_OK;
}

void extractPlanesFromFrustrum(D3DXVECTOR4* planeEquation, const D3DXMATRIX* viewProj, bool normalize)
{
	//Left clipping plane
	planeEquation[0].x = viewProj->_14 + viewProj->_11;
	planeEquation[0].y = viewProj->_24 + viewProj->_21;
	planeEquation[0].z = viewProj->_34 + viewProj->_31;
	planeEquation[0].w = viewProj->_44 + viewProj->_41;

	//Right clipping plane
	planeEquation[1].x = viewProj->_14 - viewProj->_11;
	planeEquation[1].y = viewProj->_24 - viewProj->_21;
	planeEquation[1].z = viewProj->_34 - viewProj->_31;
	planeEquation[1].w = viewProj->_44 - viewProj->_41;

	//Top clipping plane
	planeEquation[2].x = viewProj->_14 - viewProj->_12;
	planeEquation[2].y = viewProj->_24 - viewProj->_22;
	planeEquation[2].z = viewProj->_34 - viewProj->_32;
	planeEquation[2].w = viewProj->_44 - viewProj->_42;

	//Bottom clipping plane
	planeEquation[3].x = viewProj->_14 + viewProj->_12;
	planeEquation[3].y = viewProj->_24 + viewProj->_22;
	planeEquation[3].z = viewProj->_34 + viewProj->_32;
	planeEquation[3].w = viewProj->_44 + viewProj->_42;

	//Near clipping plane
	planeEquation[4].x = viewProj->_13;
	planeEquation[4].y = viewProj->_23;
	planeEquation[4].z = viewProj->_33;
	planeEquation[4].w = viewProj->_43;

	//Far clipping plane
	planeEquation[5].x = viewProj->_14 - viewProj->_13;
	planeEquation[5].y = viewProj->_24 - viewProj->_23;
	planeEquation[5].z = viewProj->_34 - viewProj->_33;
	planeEquation[5].w = viewProj->_44 - viewProj->_43;

	if(normalize)
	{
		normalizePlane(&planeEquation[0]);
		normalizePlane(&planeEquation[1]);
		normalizePlane(&planeEquation[2]);
		normalizePlane(&planeEquation[3]);
		normalizePlane(&planeEquation[4]);
		normalizePlane(&planeEquation[5]);
	}
}

void normalizePlane(D3DXVECTOR4* planeEquation)
{
	float mag;
	mag = sqrt(planeEquation->x * planeEquation->x +
			   planeEquation->y * planeEquation->y + 
			   planeEquation->z * planeEquation->z);

	planeEquation->x = planeEquation->x / mag;
	planeEquation->y = planeEquation->y / mag;
	planeEquation->z = planeEquation->z / mag;
	planeEquation->w = planeEquation->w / mag;
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