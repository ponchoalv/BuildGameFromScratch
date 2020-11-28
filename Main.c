#pragma warning(push, 3)
#include <stdio.h>
#include <windows.h>
#include <emmintrin.h>
#pragma warning(pop)

#include <stdint.h>
#include "Main.h"

HWND gGameWindow;
BOOL gGameIsRunning;
GAMEBITMAP gBackBuffer;
GAMEPERFDATA gPerformanceData;
BOOL gBackgroundDraw;

INT __stdcall WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, PSTR CommandLine, INT CmdShow)
{

	UNREFERENCED_PARAMETER(PreviousInstance);
	UNREFERENCED_PARAMETER(CommandLine);
	UNREFERENCED_PARAMETER(CmdShow);

	MSG Message = { 0 };
	int64_t FrameStart = 0;
	int64_t FrameEnd = 0;
	int64_t ElapsedMicrosecondsPerFrame = 0;
	int64_t ElapsedMicrosecondsPerFrameAccumulatorRaw = 0;
	int64_t ElapsedMicrosecondsPerFrameAccumulatorCooked = 0;
	HMODULE NtDllModuleHandle;

	if ((NtDllModuleHandle = GetModuleHandleA("ntdll.dll")) == NULL)
	{
		MessageBoxA(NULL, "Couldn't load ntdll!", "Error!", MB_ICONEXCLAMATION | MB_OK);

		goto Exit;
	}

	if ((NtQueryTimerResolution = (_NtQueryTimerResolution)GetProcAddress(NtDllModuleHandle, "NtQueryTimerResolution")) == NULL)
	{
		MessageBoxA(NULL, "Couldn't find the NtQueryTimerResolution function in ntdll.dll", "Error!", MB_ICONEXCLAMATION | MB_OK);

		goto Exit;
	}

	if (GameIsAlreadyRunning() == TRUE)
	{
		MessageBoxA(NULL, "Another instance of this program is already running!", "Error", MB_ICONEXCLAMATION | MB_OK);

		goto Exit;
	}

	if (CreateMainGameWindow(Instance) != ERROR_SUCCESS)
	{
		goto Exit;
	}

	NtQueryTimerResolution(&gPerformanceData.MinimumTimerResolution,
		&gPerformanceData.MaximumTimerResolution,
		&gPerformanceData.CurrentTimerResolution);

	QueryPerformanceFrequency((LARGE_INTEGER*)&gPerformanceData.PerfFrequency);

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
		goto Exit;
	}

	memset(gBackBuffer.Memory, 0xFF, GAME_DRAWING_AREA_MEMORY_SIZE);

	gGameIsRunning = TRUE;

	while (gGameIsRunning == TRUE)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&FrameStart);

		while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE)) {
			DispatchMessageA(&Message);
		}

		ProcessPlayerInput();
		RenderFrameGraphics();

		QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);
		ElapsedMicrosecondsPerFrame = FrameEnd - FrameStart;
		ElapsedMicrosecondsPerFrame *= 1000000;
		ElapsedMicrosecondsPerFrame /= gPerformanceData.PerfFrequency;
		gPerformanceData.TotalFramesRendered++;
		ElapsedMicrosecondsPerFrameAccumulatorRaw += ElapsedMicrosecondsPerFrame;

		while (ElapsedMicrosecondsPerFrame <= TARGET_MICROSECONDS_PER_FRAME)
		{
			ElapsedMicrosecondsPerFrame = FrameEnd - FrameStart;
			ElapsedMicrosecondsPerFrame *= 1000000;
			ElapsedMicrosecondsPerFrame /= gPerformanceData.PerfFrequency;
			QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);

			if (ElapsedMicrosecondsPerFrame <= ((int64_t)TARGET_MICROSECONDS_PER_FRAME - gPerformanceData.CurrentTimerResolution))
			{
				Sleep(1); // Could be anywhere from 1ms to a full system timer tick? (~15.625ms)
			}
		}

		ElapsedMicrosecondsPerFrameAccumulatorCooked += ElapsedMicrosecondsPerFrame;


		if (gPerformanceData.TotalFramesRendered % CALCULATE_AVG_FPS_EVERY_X_FRAMES == 0)
		{
			gPerformanceData.RawFPSAverage = 1.0f / ((ElapsedMicrosecondsPerFrameAccumulatorRaw / CALCULATE_AVG_FPS_EVERY_X_FRAMES) * 0.000001f);
			gPerformanceData.CookedFPSAverage = 1.0f / ((ElapsedMicrosecondsPerFrameAccumulatorCooked / CALCULATE_AVG_FPS_EVERY_X_FRAMES) * 0.000001f);

			ElapsedMicrosecondsPerFrameAccumulatorRaw = 0;
			ElapsedMicrosecondsPerFrameAccumulatorCooked = 0;
		}
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
	WindowClass.hIconSm = LoadIconA(NULL, IDI_APPLICATION);
	WindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
	WindowClass.hbrBackground = CreateSolidBrush(RGB(255, 0, 255));
	WindowClass.lpszMenuName = NULL;
	WindowClass.lpszClassName = GAME_NAME "_WINDOWCLASS";

	//SetProcessDpiAwarenessContext(DPI_AWARENESS_PER_MONITOR_AWARE_2);

	if (RegisterClassExA(&WindowClass) == 0)
	{
		Result = GetLastError();
		MessageBoxA(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	gGameWindow = CreateWindowExA(
		0,
		WindowClass.lpszClassName,
		"Window Title!",
		WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, GAME_RES_WIDTH, GAME_RES_HEIGHT,
		NULL, NULL, WindowClass.hInstance, NULL);

	if (gGameWindow == NULL)
	{
		Result = GetLastError();

		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);

		goto Exit;
	}

	gPerformanceData.MonitorInfo.cbSize = sizeof(MONITORINFO);

	if (GetMonitorInfoA(MonitorFromWindow(gGameWindow, MONITOR_DEFAULTTOPRIMARY), &gPerformanceData.MonitorInfo) == FALSE)
	{
		Result = ERROR_MONITOR_NO_DESCRIPTOR;

		goto Exit;
	}

	gPerformanceData.MonitorWidth = gPerformanceData.MonitorInfo.rcMonitor.right - gPerformanceData.MonitorInfo.rcMonitor.left;
	gPerformanceData.MonitorHeight = gPerformanceData.MonitorInfo.rcMonitor.bottom - gPerformanceData.MonitorInfo.rcMonitor.top;

	if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, WS_VISIBLE) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

	if (SetWindowPos(gGameWindow, HWND_TOP,
		gPerformanceData.MonitorInfo.rcMonitor.left,
		gPerformanceData.MonitorInfo.rcMonitor.top,
		gPerformanceData.MonitorWidth,
		gPerformanceData.MonitorHeight,
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
	int16_t EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);
	int16_t DebugKeyIsDown = GetAsyncKeyState(VK_F1);

	static int16_t DebugKeyWasDown;

	if (EscapeKeyIsDown) {
		SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
	}

	if (DebugKeyIsDown && !DebugKeyWasDown)
	{
		gPerformanceData.DisplayDebugInfo = !gPerformanceData.DisplayDebugInfo;
	}

	DebugKeyWasDown = DebugKeyIsDown;
}

void RenderFrameGraphics(void)
{
	__m128i QuadPixel = { 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff };
	ClearScreen(QuadPixel);

	PIXEL32 Pixel2 = { 0 };
	char DebugTextBuffer[64] = { 0 };

	Pixel2.Blue = 0;
	Pixel2.Green = 0;
	Pixel2.Red = 0xAF;
	Pixel2.Alpha = 0xFF;

	// memset(gBackBuffer.Memory, 0xFF, GAME_DRAWING_AREA_MEMORY_SIZE / 2);
	//if (!gBackgroundDraw)
	//{


		//for (int x = (GAME_RES_WIDTH * GAME_RES_HEIGHT) - 1; x > GAME_RES_WIDTH * GAME_RES_HEIGHT / (1.50000001); x-=4)
		//{
		//	_mm_store_si128((PIXEL32*)gBackBuffer.Memory + x, QuadPixel);
		//	//memcpy_s((PIXEL32*)gBackBuffer.Memory + x, sizeof(PIXEL32), &Pixel2, sizeof(PIXEL32));
		//}

		int32_t ScreenX = 25;
		int32_t ScreenY = 25;

		int32_t StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH) - \
			(GAME_RES_WIDTH * ScreenY) + ScreenX;

		for (int32_t y = 0; y < 16; y++)
		{
			for (int32_t x = 0; x < 16; x++)
			{
				memset((PIXEL32*)gBackBuffer.Memory + (uintptr_t)StartingScreenPixel + x - ((uintptr_t)GAME_RES_WIDTH * y), 0xFF, sizeof(PIXEL32));
			}
		}

	/*	gBackgroundDraw = !gBackgroundDraw;
	}*/

	HDC DeviceContext = GetDC(gGameWindow);

	StretchDIBits(DeviceContext,
		0,
		0,
		gPerformanceData.MonitorWidth,
		gPerformanceData.MonitorHeight,
		0,
		0,
		GAME_RES_WIDTH,
		GAME_RES_HEIGHT,
		gBackBuffer.Memory,
		&gBackBuffer.BitmapInfo,
		DIB_RGB_COLORS,
		SRCCOPY);

	if (gPerformanceData.DisplayDebugInfo == TRUE)
	{
		SelectObject(DeviceContext, (HFONT)GetStockObject(ANSI_FIXED_FONT));
		SetTextColor(DeviceContext, RGB(255, 0, 255));

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPS RAW:    %.01f", gPerformanceData.RawFPSAverage);
		TextOutA(DeviceContext, 0, 0, DebugTextBuffer, (int)strlen(DebugTextBuffer));

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPS Cooked: %.01f", gPerformanceData.CookedFPSAverage);
		TextOutA(DeviceContext, 0, 13, DebugTextBuffer, (int)strlen(DebugTextBuffer));

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Min Timer Res: %.02f", gPerformanceData.MinimumTimerResolution / 10000.0f);
		TextOutA(DeviceContext, 0, 26, DebugTextBuffer, (int)strlen(DebugTextBuffer));

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Max Timer Res: %.02f", gPerformanceData.MaximumTimerResolution / 10000.0f);
		TextOutA(DeviceContext, 0, 39, DebugTextBuffer, (int)strlen(DebugTextBuffer));

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Cur Timer Res: %.02f", gPerformanceData.CurrentTimerResolution / 10000.0f);
		TextOutA(DeviceContext, 0, 52, DebugTextBuffer, (int)strlen(DebugTextBuffer));

	}

	ReleaseDC(gGameWindow, DeviceContext);
}

__forceinline void ClearScreen(__m128i Color)
{
	for (int x = 0; x < GAME_RES_WIDTH * GAME_RES_HEIGHT; x += 4)
	{
		_mm_store_si128((PIXEL32*)gBackBuffer.Memory + x, Color);
	}
}
