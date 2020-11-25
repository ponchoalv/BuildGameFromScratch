#include <stdio.h>

#pragma warning(push, 3)
#include <windows.h>
#pragma warning(pop)

#include "Main.h"

BOOL gGameIsRunning;
HWND gGameWindow;

GAMEBITMAP gDrawingSurface;

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

	gDrawingSurface.BitmapInfo.bmiHeader.biSize = sizeof(gDrawingSurface.BitmapInfo.bmiHeader);
	gDrawingSurface.BitmapInfo.bmiHeader.biWidth = GAME_RES_WIDTH;
	gDrawingSurface.BitmapInfo.bmiHeader.biHeight = GAME_RES_HEIGHT;
	gDrawingSurface.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;
	gDrawingSurface.BitmapInfo.bmiHeader.biCompression = BI_RGB;
	gDrawingSurface.BitmapInfo.bmiHeader.biPlanes = 1;
	gDrawingSurface.Memory = VirtualAlloc(NULL, GAME_DRAWING_AREA_MEMORY_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if ((gDrawingSurface.Memory) == NULL) {
		MessageBoxA(NULL, "Failded to allocate memory for drawing surface!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return(0);
	}

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
	WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";
	WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);

	if (RegisterClassExA(&WindowClass) == 0)
	{
		Result = GetLastError();
		MessageBoxA(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	gGameWindow = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		WindowClass.lpszClassName,
		"Game B title!! baby!!!",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
		NULL, NULL, WindowClass.hInstance, NULL);

	if (gGameWindow == NULL)
	{
		Result = GetLastError();

		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);

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
	SHORT EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);

	if (EscapeKeyIsDown) {
		SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
	}
}

void RenderFrameGraphics(void)
{

}
