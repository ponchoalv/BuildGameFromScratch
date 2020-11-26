#include <stdio.h>

#pragma warning(push, 3)
#include <windows.h>
#pragma warning(pop)

#include <stdint.h>

#include "Main.h"

BOOL gGameIsRunning;
HWND gGameWindow;

GAMEBITMAP gBackBuffer;
MONITORINFO gMonitorInfo = { sizeof(MONITORINFO) };

int32_t gMonitorWidth;
int32_t gMonitorHeight;

INT WINAPI WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, INT CmdShow)
{

	UNREFERENCED_PARAMETER(PreviousInstance);
	UNREFERENCED_PARAMETER(CommandLine);
	UNREFERENCED_PARAMETER(CmdShow);

	if (GameIsAlreadyRunning() == TRUE)
	{
		MessageBoxA(NULL, "Another instance of this program is already running!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	if (CreateMainGameWindow(Instance) != ERROR_SUCCESS)
	{
		goto Exit;
	}

	gBackBuffer.BitmapInfo.bmiHeader.biSize = sizeof(gBackBuffer.BitmapInfo.bmiHeader);
	gBackBuffer.BitmapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;
	gBackBuffer.BitmapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
	gBackBuffer.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;
	gBackBuffer.BitmapInfo.bmiHeader.biCompression = BI_RGB;
	gBackBuffer.BitmapInfo.bmiHeader.biPlanes = 1;
	gBackBuffer.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if ((gBackBuffer.Memory) == NULL) {
		MessageBoxA(NULL, "Failded to allocate memory for drawing surface!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return(0);
	}

	memset(gBackBuffer.Memory, 0xFF, GAME_DRAWING_AREA_MEMORY_SIZE);

	MSG Message = { 0 };
	gGameIsRunning = TRUE;

	while (gGameIsRunning == TRUE)
	{
		while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE)) {
			TranslateMessage(&Message);
			DispatchMessageA(&Message);
		}

		ProcessPlayerInput();
		RenderFrameGraphics();

		Sleep(1);

	}

Exit:
	return(0);
}

LRESULT CALLBACK MainWindowProc(_In_ HWND WindowHandler, _In_ UINT Message, _In_ WPARAM WParam, _In_ LPARAM LParam)
{
	LRESULT Result = 0;

	switch (Message)
	{
	case WM_CLOSE:
	{
		gGameIsRunning = FALSE;
		PostQuitMessage(0);

		break;
	}
	default:
	{
		Result = DefWindowProcA(WindowHandler, Message, WParam, LParam);
	}

	}
	return(Result);
}

DWORD CreateMainGameWindow(_In_ HINSTANCE Instance)
{

	DWORD Result = ERROR_SUCCESS;
	WNDCLASSEXA WindowClass = { 0 };

	WindowClass.cbSize = sizeof(WNDCLASSEXA);
	WindowClass.style = 0;
	WindowClass.lpfnWndProc = MainWindowProc;
	WindowClass.cbClsExtra = 0;
	WindowClass.cbWndExtra = 0;
	WindowClass.hInstance = Instance; // same as GetModuleHandleA(NULL);
	WindowClass.hIcon = LoadIconA(NULL, IDI_APPLICATION);
	WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
	WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";
	WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);

	//SetProcessDpiAwarenessContext(DPI_AWARENESS_PER_MONITOR_AWARE_2);

	if (RegisterClassExA(&WindowClass) == 0)
	{
		Result = GetLastError();
		MessageBoxA(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	gGameWindow = CreateWindowEx(
		0,
		WindowClass.lpszClassName,
		"Game B title!! baby!!!",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, GAME_RES_WIDTH, GAME_RES_HEIGHT,
		NULL, NULL, WindowClass.hInstance, NULL);

	if (gGameWindow == NULL)
	{
		Result = GetLastError();

		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);

		goto Exit;
	}


	if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gMonitorInfo) == FALSE)
	{
		Result = ERROR_MONITOR_NO_DESCRIPTOR;

		goto Exit;
	}

	gMonitorWidth = gMonitorInfo.rcMonitor.right - gMonitorInfo.rcMonitor.left;
	gMonitorHeight = gMonitorInfo.rcMonitor.bottom - gMonitorInfo.rcMonitor.top;

	if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_OVERLAPPEDWINDOW) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

	if (SetWindowPos(gGameWindow, HWND_TOP,
		gMonitorInfo.rcMonitor.left, gMonitorInfo.rcMonitor.top,
		gMonitorWidth, gMonitorHeight,
		SWP_NOOWNERZORDER | SWP_FRAMECHANGED) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

Exit:
	return(Result);
}

BOOL GameIsAlreadyRunning(void)
{
	HANDLE Mutex = NULL;
	Mutex = CreateMutexA(NULL, FALSE, GAME_NAME "_GameMutex");

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}

void ProcessPlayerInput(void)
{
	BOOL EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE) & VK_ESCAPE;

	if (EscapeKeyIsDown == TRUE) {
		SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
	}
}

void RenderFrameGraphics(void)
{
	PIXEL32 Pixel = { 0 };
	PIXEL32 Pixel2 = { 0 };

	Pixel.Blue = 0x7F;
	Pixel.Green = 0;
	Pixel.Red = 0;

	Pixel.Alpha = 0xFF;


	Pixel2.Blue = 0;
	Pixel2.Green = 0;
	Pixel2.Red = 0xAF;

	Pixel2.Alpha = 0xFF;

	// memset(gBackBuffer.Memory, 0xFF, GAME_DRAWING_AREA_MEMORY_SIZE / 2);
	for (int x = 0; x < GAME_RES_WIDTH * GAME_RES_HEIGHT / 4; x++)
	{
		memcpy((PIXEL32*)gBackBuffer.Memory + x, &Pixel, sizeof(PIXEL32));
	}

	for (int x = (GAME_RES_WIDTH * GAME_RES_HEIGHT) - 1; x > GAME_RES_WIDTH * GAME_RES_HEIGHT / 1.481; x--)
	{
		memcpy((PIXEL32*)gBackBuffer.Memory + x, &Pixel2, sizeof(PIXEL32));
	}

	// memcpy(gBackBuffer.Memory, &Pixel, GAME_DRAWING_AREA_MEMORY_SIZE / 2);

	HDC DeviceContext = GetDC(gGameWindow);
	StretchDIBits(DeviceContext, 0, 0, gMonitorWidth, gMonitorHeight,
		0, 0, GAME_RES_WIDTH, GAME_RES_HEIGHT,
		gBackBuffer.Memory, &gBackBuffer.BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
	ReleaseDC(gGameWindow, DeviceContext);

}
