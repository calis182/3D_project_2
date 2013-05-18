#include "Input.h"

Input::Input()
{
}

Input::~Input()
{
}

bool Input::init(HINSTANCE hInstance, HWND hwnd, int screenWidth, int screenHeight)
{
	this->screenHeight = screenHeight;
	this->screenWidth = screenWidth;

	mouseX = 0;
	mouseY = 0;

	HRESULT result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, NULL);
	if(FAILED(result))
		return false;

	//Setup keyboard

	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	if(FAILED(result))
		return false;

	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	if(FAILED(result))
		return false;

	/** Sätt cooperative level DISCL_EXCLUSIVE sätter så att bara ditt program får 
		inputen från tangentbordet och inte alla program i systemet.*/
	result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if(FAILED(result))
		return false;

	//Acquire the keyboard
	result = keyboard->Acquire();
	if(FAILED(result))
		return false;

	//Setup mouse

	result = directInput->CreateDevice(GUID_SysMouse, &mouse, NULL);
	if(FAILED(result))
		return false;

	result = mouse->SetDataFormat(&c_dfDIMouse);
	if(FAILED(result))
		return false;

	result = mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if(FAILED(result))
		return false;

	result = mouse->Acquire();
	if(FAILED(result))
		return false;

	return true;
}

void Input::shutdown()
{
	if(mouse)
	{
		mouse->Unacquire();
		mouse->Release();
	}

	if(keyboard)
	{
		keyboard->Unacquire();
		keyboard->Release();
	}

	if(directInput)
		directInput->Release();
}

bool Input::frame()
{
	bool result = readKeyBoard();
	if(!result)
		return false;

	result = readMouse();
	if(!result)
		return false;

	processInput();

	return true;
}

bool Input::isEscapedPressed()
{
	if(keyboardState[DIK_ESCAPE] & 0x80)
		return true;

	return false;
}

bool Input::isWPressed()
{
	if(keyboardState[DIK_W] & 0x80)
		return true;
	return false;
}

bool Input::isKeyPressed(char key)
{
	if(keyboardState[key] & 0x80)
		return true;
	return false;
}

void Input::getMouseLocation(int &x, int &y)
{
	x = mouseX;
	y = mouseY;
}

void Input::getDiffMouseLocation(int &x, int &y)
{
	x = mouseState.lX;
	y = mouseState.lY;
}

bool Input::readKeyBoard()
{
	HRESULT result = keyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
	if(FAILED(result))
	{
		if((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
			keyboard->Acquire();
		else
			return false;
	}

	return true;
}

bool Input::readMouse()
{
	HRESULT result = mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mouseState);
	if(FAILED(result))
	{
		if((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
			mouse->Acquire();
		else
			return false;
	}
	return true;
}

void Input::processInput()
{
	mouseX += mouseState.lX;
	mouseY += mouseState.lY;

	if(mouseX < 0) mouseX = 0;
	if(mouseY < 0) mouseY = 0;

	if(mouseX > screenWidth) { mouseX = screenWidth; }
	if(mouseY > screenHeight) { mouseY = screenHeight; }
}