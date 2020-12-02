#pragma warning(push, 3)
#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include <emmintrin.h>
#pragma warning(pop)

#include <stdint.h>
#include "Main.h"

#pragma comment(lib, "Winmm.lib")

HWND gGameWindow;
BOOL gGameIsRunning;
GAMEBITMAP gBackBuffer;
GAMEBITMAP g6x7Font;
GAMEPERFDATA gPerformanceData;
HERO gPlayer;
BOOL gWindowHasForcus;
uint8_t charToPixelOffset[] = {
	//	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	
		93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93,
		//	..	..	..	..	..	..	..	..	..	..	..	..	' ' !	"	#	$	%	&	'	(	
			93, 93, 93, 93, 93, 93, 93, 93, 93, 93, 93,	93, 94, 64, 87, 66, 67, 68, 70, 85, 72,
			//	)	*	+	,	-	.	/	0	1	2	3	4	5	6	7	8	9	:	;	<
				73,	71,	77,	88,	74,	91, 92, 52,	53,	54,	55,	56,	57,	58,	59,	60,	61,	86,	84,	89,
				//	=	>	?	@	A	B	C	D	E	F	G	H	I	J	K	L	M	N	O	P
					75,	90,	93,	65,	0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	12,	13,	14,	15,
					//	Q	R	S	T	U	V	W	X	Y	Z	[	\	]	^	_	`	a	b	c	d
						16,	17,	18,	19,	20,	21,	22,	23,	24,	25,	80,	78,	81,	69,	76,	62,	26,	27,	28,	29,
						//	e	f	g	h	i	j	k	l	m	n	o	p	q	r	s	t	u	v	w	x
							30,	31,	32,	33,	34,	35,	36,	37,	38,	39,	40,	41,	42,	43,	44,	45,	46,	47,	48,	49,
							//	y	z	{	|	}	~	..	..	..	..	..	..	..	..	..	..	..	..	..	..
								50,	51,	82,	79,	83,	63,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,
								//	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..
									93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,
									//	..	..	..	..	..	..	..	..	..	..	�	..	..	..	..	..	..	..	..	..
										93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	96,	93, 93,	93,	93,	93,	93,	93,	93,	93,
										//	..	..	..	..	..	..	�	..	..	..	..	..	..	..	..	..	..	..	..	..
											93,	93,	93,	93,	93,	93,	95,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,
											//	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..
												93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,
												//	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..
													93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,
													//	..	F2	..	..	..	..	..	..	..	..	..	..	..	..	..	..	
														93,	97,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93
};

INT __stdcall WinMain(_In_ HINSTANCE Instance, _In_opt_ HINSTANCE PreviousInstance, _In_ PSTR CommandLine, _In_ INT CmdShow)
{

	UNREFERENCED_PARAMETER(PreviousInstance);
	UNREFERENCED_PARAMETER(CommandLine);
	UNREFERENCED_PARAMETER(CmdShow);

	MSG Message = { 0 };
	int64_t FrameStart = 0;
	int64_t FrameEnd = 0;
	int64_t ElapsedMicroseconds = 0;
	int64_t ElapsedMicrosecondsAccumulatorRaw = 0;
	int64_t ElapsedMicrosecondsAccumulatorCooked = 0;

	FILETIME ProcessCreationTime = { 0 };
	FILETIME ProcessExitTime = { 0 };
	int64_t CurrentUserCPUTime = 0;
	int64_t CurrentKernelCPUTime = 0;
	int64_t PreviousUserCPUTime = 0;
	int64_t PreviousKernelCPUTime = 0;

	HANDLE ProcessHandle = GetCurrentProcess();

	GetSystemInfo(&gPerformanceData.SystemInfo);

	if (GameIsAlreadyRunning() == TRUE)
	{
		MessageBoxA(NULL, "Another instance of this program is already running!", "Error", MB_ICONEXCLAMATION | MB_OK);

		goto Exit;
	}

	if (timeBeginPeriod(1) == TIMERR_NOCANDO)
	{
		MessageBoxA(NULL, "Failed to set global timer resolution!", "Error", MB_ICONEXCLAMATION | MB_OK);

		goto Exit;
	}

	if (SetPriorityClass(ProcessHandle, HIGH_PRIORITY_CLASS) == 0)
	{
		MessageBoxA(NULL, "Failed to set process priority!", "Error", MB_ICONEXCLAMATION | MB_OK);

		goto Exit;
	}

	if (SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST) == 0)
	{
		MessageBoxA(NULL, "Failed to set thread priority!", "Error", MB_ICONEXCLAMATION | MB_OK);

		goto Exit;
	}

	if (CreateMainGameWindow(Instance) != ERROR_SUCCESS)
	{
		goto Exit;
	}

	if (Load32BppBitmapFromFile("Assets\\6x7Font.bmpx", &g6x7Font) != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

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

	memset(gBackBuffer.Memory, 0xCC, GAME_DRAWING_AREA_MEMORY_SIZE);

	if (InitializeHero() != ERROR_SUCCESS) {
		MessageBoxA(NULL, "Failed to initialize hero!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

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
		ElapsedMicroseconds = FrameEnd - FrameStart;
		ElapsedMicroseconds *= 1000000;
		ElapsedMicroseconds /= gPerformanceData.PerfFrequency;
		gPerformanceData.TotalFramesRendered++;
		ElapsedMicrosecondsAccumulatorRaw += ElapsedMicroseconds;

		while (ElapsedMicroseconds < TARGET_MICROSECONDS_PER_FRAME)
		{
			ElapsedMicroseconds = FrameEnd - FrameStart;
			ElapsedMicroseconds *= 1000000;
			ElapsedMicroseconds /= gPerformanceData.PerfFrequency;
			QueryPerformanceCounter((LARGE_INTEGER*)&FrameEnd);

			if (ElapsedMicroseconds < (TARGET_MICROSECONDS_PER_FRAME * 0.80f))
			{
				Sleep(1); // Could be anywhere from 1ms to a full system timer tick? (~15.625ms)
			}
		}

		ElapsedMicrosecondsAccumulatorCooked += ElapsedMicroseconds;

		if (gPerformanceData.TotalFramesRendered % CALCULATE_AVG_FPS_EVERY_X_FRAMES == 0)
		{
			GetSystemTimeAsFileTime((FILETIME*)&gPerformanceData.CurrentSystemTime);

			GetProcessTimes(ProcessHandle,
				&ProcessCreationTime,
				&ProcessExitTime,
				(FILETIME*)&CurrentKernelCPUTime,
				(FILETIME*)&CurrentUserCPUTime);

			gPerformanceData.CPUPercent = (CurrentKernelCPUTime - PreviousKernelCPUTime) + \
				(double)(CurrentUserCPUTime - PreviousUserCPUTime);

			gPerformanceData.CPUPercent /= (gPerformanceData.CurrentSystemTime - gPerformanceData.PreviousSystemTime);
			gPerformanceData.CPUPercent /= gPerformanceData.SystemInfo.dwNumberOfProcessors;
			gPerformanceData.CPUPercent *= 100;

			GetProcessHandleCount(ProcessHandle, &gPerformanceData.HandleCount);
			K32GetProcessMemoryInfo(ProcessHandle, (PROCESS_MEMORY_COUNTERS*)&gPerformanceData.MemInfo, sizeof(gPerformanceData.MemInfo));

			gPerformanceData.RawFPSAverage = 1.0f / ((ElapsedMicrosecondsAccumulatorRaw / CALCULATE_AVG_FPS_EVERY_X_FRAMES) * 0.000001f);
			gPerformanceData.CookedFPSAverage = 1.0f / ((ElapsedMicrosecondsAccumulatorCooked / CALCULATE_AVG_FPS_EVERY_X_FRAMES) * 0.000001f);

			ElapsedMicrosecondsAccumulatorRaw = 0;
			ElapsedMicrosecondsAccumulatorCooked = 0;

			PreviousKernelCPUTime = CurrentKernelCPUTime;
			PreviousUserCPUTime = CurrentUserCPUTime;
			gPerformanceData.PreviousSystemTime = gPerformanceData.CurrentSystemTime;
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

	case WM_ACTIVATE:
	{
		if (WParam == 0)
		{
			// our window has lost focus
			gWindowHasForcus = FALSE;
		}
		else {
			// Our window has gained focus
			ShowCursor(FALSE);

			gWindowHasForcus = TRUE;
		}

		break;
	}

	default:
	{
		Result = DefWindowProcA(WindowHandler, Message, WParam, LParam);
	}

	}
	return(Result);
}

__forceinline DWORD CreateMainGameWindow(_In_ HINSTANCE Instance)
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

__forceinline BOOL GameIsAlreadyRunning(void)
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

__forceinline void ProcessPlayerInput(void)
{
	if (gWindowHasForcus == FALSE)
	{
		return;
	}

	int16_t EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);
	int16_t DebugKeyIsDown = GetAsyncKeyState(VK_F1);
	int16_t LeftKeyIsDown = GetAsyncKeyState(VK_LEFT) | GetAsyncKeyState('A');
	int16_t RightKeyIsDown = GetAsyncKeyState(VK_RIGHT) | GetAsyncKeyState('D');
	int16_t UpKeyIsDown = GetAsyncKeyState(VK_UP) | GetAsyncKeyState('W');
	int16_t DownKeyIsDown = GetAsyncKeyState(VK_DOWN) | GetAsyncKeyState('S');

	int16_t RightOrLeftIsDown = LeftKeyIsDown || RightKeyIsDown;

	static int16_t DebugKeyWasDown;
	static int16_t LeftKeyWasDown;
	static int16_t RightKeyWasDown;


	if (EscapeKeyIsDown) {
		SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
	}

	if (DebugKeyIsDown && !DebugKeyWasDown)
	{
		gPerformanceData.DisplayDebugInfo = !gPerformanceData.DisplayDebugInfo;
	}

	if (LeftKeyIsDown)
	{
		UpdateHeroMovement(&gPlayer, FACING_LEFT_0);
	}

	if (RightKeyIsDown)
	{
		UpdateHeroMovement(&gPlayer, FACING_RIGHT_0);
	}

	if (UpKeyIsDown && !RightOrLeftIsDown)
	{
		UpdateHeroMovement(&gPlayer, FACING_UPWARD_0);
	}

	if (DownKeyIsDown && !RightOrLeftIsDown)
	{
		UpdateHeroMovement(&gPlayer, FACING_DOWN_0);
	}

	if (gPlayer.AccumulatedMovements % 17) {
		UpdateHeroMovement(&gPlayer, gPlayer.Direction);
	}

	DebugKeyWasDown = DebugKeyIsDown;
}

void RenderFrameGraphics(void)
{
#ifdef SIMD
	__m128i QuadPixel = { 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff, 0x7f, 0x00, 0x00, 0xff };
	ClearScreen(&QuadPixel);
#else

	PIXEL32 Pixel = { 0 };

	Pixel.Blue = 0x7f;
	Pixel.Green = 0;
	Pixel.Red = 0;
	Pixel.Alpha = 0xFF;
	ClearScreen(&Pixel);
#endif
	char DebugTextBuffer[64] = { 0 };




	//	Blit32BppBitmapToBuffer(&g6x7Font, 0, 7);
	//	BlitStringToScreen("1234567890", &g6x7Font, 64, 64);

	if (gPerformanceData.DisplayDebugInfo == TRUE)
	{
		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPS RAW:  %.01f", gPerformanceData.RawFPSAverage);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, 0, 0);

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPS Cooked: %.01f", gPerformanceData.CookedFPSAverage);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, 0, 8);


		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Handles: %lu", gPerformanceData.HandleCount);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, 0, 16);

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Memory: %lluKB", gPerformanceData.MemInfo.PrivateUsage / 1024);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, 0, 24);

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "CPU: %0.2f%%", gPerformanceData.CPUPercent);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, 0, 32);

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "ScreenPos: (%d, %d)", gPlayer.ScreenPosX, gPlayer.ScreenPosY);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, 0, 40);

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Total Frames: %llu", gPerformanceData.TotalFramesRendered);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, 0, 48);



	}
	Blit32BppBitmapToBuffer(&gPlayer.Sprite[gPlayer.CurrentArmor][gPlayer.Direction + gPlayer.Step], gPlayer.ScreenPosX, gPlayer.ScreenPosY);

	//Blit32BppBitmapToBuffer(&g6x7Font, 0, 60);
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

	ReleaseDC(gGameWindow, DeviceContext);
}

DWORD Load32BppBitmapFromFile(_In_ char* FileName, _Inout_ GAMEBITMAP* GameBitmap)
{
	DWORD Result = ERROR_SUCCESS;

	HANDLE FileHandle = INVALID_HANDLE_VALUE;

	FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	WORD BitmapHeader = 0;
	DWORD PixelDataOffset = 0;
	DWORD NumberOfBytesRead = 0;
	DWORD FileLenght = 0;

	if (GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		Result = ERROR_FILE_NOT_FOUND;

		MessageBoxA(NULL, "Failded loading Bitmap from disk, file not found", "Error", MB_ICONEXCLAMATION | MB_OK);

		goto Exit;
	}

	if (ReadFile(FileHandle, &BitmapHeader, 2, &NumberOfBytesRead, NULL) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

	if (BitmapHeader != 0x4d42) // "BM" Backwards
	{
		Result = ERROR_FILE_INVALID;

		goto Exit;
	}

	if (ReadFile(FileHandle, &FileLenght, 4, &NumberOfBytesRead, NULL) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

	if (SetFilePointer(FileHandle, 0x0A, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		Result = GetLastError();

		goto Exit;
	}

	if (ReadFile(FileHandle, &PixelDataOffset, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

	if (SetFilePointer(FileHandle, 0xE, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		Result = GetLastError();

		goto Exit;
	}

	if (ReadFile(FileHandle, &GameBitmap->BitmapInfo.bmiHeader, sizeof(BITMAPINFOHEADER), &NumberOfBytesRead, NULL) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

	GameBitmap->Memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, GameBitmap->BitmapInfo.bmiHeader.biSizeImage);

	if (GameBitmap->Memory == NULL)
	{
		Result = ERROR_NOT_ENOUGH_MEMORY;

		goto Exit;
	}

	if (SetFilePointer(FileHandle, PixelDataOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		Result = GetLastError();

		goto Exit;
	}

	if (ReadFile(FileHandle, GameBitmap->Memory, GameBitmap->BitmapInfo.bmiHeader.biSizeImage, &NumberOfBytesRead, NULL) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

Exit:
	if (FileHandle && FileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(FileHandle);
	}

	return Result;
}

DWORD InitializeHero(void)
{
	DWORD Result = ERROR_SUCCESS;

	gPlayer.ScreenPosX = 32;
	gPlayer.ScreenPosY = 32;

	gPlayer.CurrentArmor = SUIT_0;
	gPlayer.Direction = FACING_DOWN_0;
	gPlayer.AccumulatedMovements = 0;

	Result = Load32BppBitmapFromFile("Assets\\Hero_Suit0_Down_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_DOWN_0]);

	if (Result != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	Result = Load32BppBitmapFromFile("Assets\\Hero_Suit0_Down_Walk1.bmpx", &gPlayer.Sprite[SUIT_0][FACING_DOWN_1]);

	if (Result != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	Result = Load32BppBitmapFromFile("Assets\\Hero_Suit0_Down_Walk2.bmpx", &gPlayer.Sprite[SUIT_0][FACING_DOWN_2]);

	if (Result != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	Result = Load32BppBitmapFromFile("Assets\\Hero_Suit0_Left_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_LEFT_0]);

	if (Result != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	Result = Load32BppBitmapFromFile("Assets\\Hero_Suit0_Left_Walk1.bmpx", &gPlayer.Sprite[SUIT_0][FACING_LEFT_1]);

	if (Result != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	Result = Load32BppBitmapFromFile("Assets\\Hero_Suit0_LEFT_Walk2.bmpx", &gPlayer.Sprite[SUIT_0][FACING_LEFT_2]);

	if (Result != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	Result = Load32BppBitmapFromFile("Assets\\Hero_Suit0_Right_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_RIGHT_0]);

	if (Result != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	Result = Load32BppBitmapFromFile("Assets\\Hero_Suit0_Right_Walk1.bmpx", &gPlayer.Sprite[SUIT_0][FACING_RIGHT_1]);

	if (Result != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	Result = Load32BppBitmapFromFile("Assets\\Hero_Suit0_Right_Walk2.bmpx", &gPlayer.Sprite[SUIT_0][FACING_RIGHT_2]);

	if (Result != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	Result = Load32BppBitmapFromFile("Assets\\Hero_Suit0_Up_Standing.bmpx", &gPlayer.Sprite[SUIT_0][FACING_UPWARD_0]);

	if (Result != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	Result = Load32BppBitmapFromFile("Assets\\Hero_Suit0_Up_Walk1.bmpx", &gPlayer.Sprite[SUIT_0][FACING_UPWARD_1]);

	if (Result != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	Result = Load32BppBitmapFromFile("Assets\\Hero_Suit0_Up_Walk2.bmpx", &gPlayer.Sprite[SUIT_0][FACING_UPWARD_2]);

	if (Result != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Load32BppBitmapFromFile failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}


Exit:
	return(Result);
}

void Blit32BppBitmapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ uint16_t x, _In_ uint16_t y)
{
	int32_t StartingScreenPixel = ((GAME_RES_WIDTH * GAME_RES_HEIGHT) - GAME_RES_WIDTH) - (GAME_RES_WIDTH * y) + x;

	int32_t StartingBitmapPixel = ((GameBitmap->BitmapInfo.bmiHeader.biWidth * GameBitmap->BitmapInfo.bmiHeader.biHeight) - \
		GameBitmap->BitmapInfo.bmiHeader.biWidth);

	int32_t MemoryOffset = 0;

	int32_t BitmapOffset = 0;

	PIXEL32 BitmapPixel = { 0 };

	for (int16_t YPixel = 0; YPixel < GameBitmap->BitmapInfo.bmiHeader.biHeight; YPixel++)
	{
		for (int16_t XPixel = 0; XPixel < GameBitmap->BitmapInfo.bmiHeader.biWidth; XPixel++)
		{
			MemoryOffset = StartingScreenPixel + XPixel - (GAME_RES_WIDTH * YPixel);

			BitmapOffset = StartingBitmapPixel + XPixel - (GameBitmap->BitmapInfo.bmiHeader.biWidth * YPixel);

			memcpy_s(&BitmapPixel, sizeof(PIXEL32), (PIXEL32*)GameBitmap->Memory + BitmapOffset, sizeof(PIXEL32));

			if (BitmapPixel.Alpha == 255)
			{
				memcpy_s((PIXEL32*)gBackBuffer.Memory + MemoryOffset, sizeof(PIXEL32), &BitmapPixel, sizeof(PIXEL32));
			}

		}
	}
}

void UpdateHeroMovement(_Inout_ HERO* Hero, _In_ uint8_t Direction)
{
	if (Hero->AccumulatedMovements % 8 == 0) {
		Hero->Step = ++Hero->Step % 3;
	}

	if (Hero->AccumulatedMovements % 17) {
		Hero->AccumulatedMovements++;

		switch (Hero->Direction)
		{
		case FACING_DOWN_0:
		{
			if (Hero->ScreenPosY < GAME_RES_HEIGHT - 16)
				Hero->ScreenPosY += 1;
			break;
		}
		case FACING_UPWARD_0:
		{
			if (Hero->ScreenPosY > 0)
				Hero->ScreenPosY -= 1;
			break;
		}
		case FACING_LEFT_0:
		{
			if (Hero->ScreenPosX > 0)
				Hero->ScreenPosX -= 1;
			break;
		}
		case FACING_RIGHT_0:
		{
			if (Hero->ScreenPosX < GAME_RES_WIDTH - 16)
				Hero->ScreenPosX += 1;
		}
		}
	}
	else
	{
		Hero->AccumulatedMovements = 1;
		Hero->Direction = Direction;
	}

}

void BlitStringToScreen(_In_ char* String, _In_ GAMEBITMAP* GameBitmap, _In_ uint16_t x, _In_ uint16_t y)
{
	uint32_t CharWidth = GameBitmap->BitmapInfo.bmiHeader.biWidth / FONT_SHEET_CHARACTERS_PER_ROW;
	uint32_t CharHeight = GameBitmap->BitmapInfo.bmiHeader.biHeight;

	uint32_t BytesPerCharacter = (CharWidth * CharHeight * (GameBitmap->BitmapInfo.bmiHeader.biBitCount / 8));
	uint64_t StringLength = strlen(String);

	GAMEBITMAP StringBitmap = { 0 };

	StringBitmap.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;
	StringBitmap.BitmapInfo.bmiHeader.biHeight = CharHeight;
	StringBitmap.BitmapInfo.bmiHeader.biWidth = CharWidth * (uint32_t)StringLength;
	StringBitmap.BitmapInfo.bmiHeader.biPlanes = 1;
	StringBitmap.BitmapInfo.bmiHeader.biCompression = BI_RGB;

	StringBitmap.Memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (uint64_t)BytesPerCharacter * (uint64_t)StringLength);

	for (uint32_t Character = 0; Character < StringLength; Character++)
	{
		uint32_t StartingFontSheetByte = 0;
		uint32_t FontSheetOffset = 0;
		uint32_t StringBitmapOffset;
		PIXEL32 FontSheetPixel = { 0 };
		uint8_t SelectedCharacter = String[Character];

		StartingFontSheetByte = (GameBitmap->BitmapInfo.bmiHeader.biWidth * GameBitmap->BitmapInfo.bmiHeader.biHeight) - GameBitmap->BitmapInfo.bmiHeader.biWidth + (CharWidth * charToPixelOffset[SelectedCharacter]);

		for (uint32_t YPixel = 0; YPixel < CharHeight; YPixel++)
		{
			for (uint32_t XPixel = 0; XPixel < CharWidth; XPixel++)
			{
				FontSheetOffset = StartingFontSheetByte + XPixel - (GameBitmap->BitmapInfo.bmiHeader.biWidth * YPixel);

				StringBitmapOffset = (Character * CharWidth) + ((StringBitmap.BitmapInfo.bmiHeader.biWidth * StringBitmap.BitmapInfo.bmiHeader.biHeight) - \
					StringBitmap.BitmapInfo.bmiHeader.biWidth) + XPixel - (StringBitmap.BitmapInfo.bmiHeader.biWidth) * YPixel;

				memcpy_s(&FontSheetPixel, sizeof(PIXEL32), (PIXEL32*)GameBitmap->Memory + FontSheetOffset, sizeof(PIXEL32));

				FontSheetPixel.Red = 0xFF;
				FontSheetPixel.Blue = 0x00;
				FontSheetPixel.Green = 0x00;

				memcpy_s((PIXEL32*)StringBitmap.Memory + StringBitmapOffset, sizeof(PIXEL32), &FontSheetPixel, sizeof(PIXEL32));
			}
		}

	}

	Blit32BppBitmapToBuffer(&StringBitmap, x, y);

	if (StringBitmap.Memory)
	{
		HeapFree(GetProcessHeap(), 0, StringBitmap.Memory);
	}
}

#ifdef SIMD
__forceinline void ClearScreen(_In_ __m128i* Color)
{
	for (int x = 0; x < ((GAME_RES_WIDTH * GAME_RES_HEIGHT) / 4); x++)
	{
		_mm_store_si128((__m128i*)gBackBuffer.Memory + x, *Color);
	}
}
#else

__forceinline void ClearScreen(_In_ PIXEL32* Pixel)
{
	for (int x = 0; x < GAME_RES_WIDTH * GAME_RES_HEIGHT; x++)
	{
		memcpy((PIXEL32*)gBackBuffer.Memory + x, Pixel, sizeof(PIXEL32));
	}
}
#endif