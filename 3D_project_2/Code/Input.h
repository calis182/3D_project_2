#ifndef INPUT_H
#define INPUT_H

#define DIRECTINPUT_VERSION 0x0800

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

class Input
{
public:
	Input();
	~Input();

	bool init(HINSTANCE hInstance, HWND hwnd, int screenWidth, int screenHeight);
	void shutdown();
	bool frame();

	bool isEscapedPressed();

	bool isWPressed();

	bool isKeyPressed(char key);

	void getMouseLocation(int &x, int &y);
	void getDiffMouseLocation(int &x, int &y);


private:
	bool readKeyBoard();
	bool readMouse();
	void processInput();

private:
	IDirectInput8* directInput;
	IDirectInputDevice8* keyboard;
	IDirectInputDevice8* mouse;

	unsigned char keyboardState[256];
	DIMOUSESTATE mouseState;

	int screenWidth, screenHeight;
	int mouseX, mouseY;

};

#endif