#pragma warning(push, 3)
#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include <xaudio2.h>
#pragma warning(pop)

#include <Xinput.h>
#include <stdint.h>
#include "Main.h"
#include "Menu.h"

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "XAudio2.lib")
#pragma comment(lib, "Xinput.lib")

HWND gGameWindow;
BOOL gGameIsRunning;
GAMEBITMAP gBackBuffer;
GAMEBITMAP g6x7Font;
GAMEPERFDATA gPerformanceData;
HERO gPlayer;
BOOL gWindowHasForcus;
uint8_t charToPixelOffset[256] = {
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
									//	..	..	..	..	..	..	..	..	..	..	«	..	..	..	..	..	..	..	..	..
										93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	96,	93, 93,	93,	93,	93,	93,	93,	93,	93,
										//	..	..	..	..	..	..	»	..	..	..	..	..	..	..	..	..	..	..	..	..
											93,	93,	93,	93,	93,	93,	95,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,
											//	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..
												93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,
												//	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..	..
													93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,
													//	..	F2	..	..	..	..	..	..	..	..	..	..	..	..	..		
														93,	97,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93,	93
};

REGISTRYPARAMS gRegistryParams;

XINPUT_STATE gGamepadState;
int8_t gGamepadID = -1;

GAMESTATE gCurrentGameState = GAMESTATE_OPENINGSPLASHSCREEN;
GAMESTATE gPreviousGameState = GAMESTATE_TITLESCREEN;
GAMESTATE gDesiredGameState;

GAMEINPUT gGameInput = { 0 };

IXAudio2* gXAudio;
IXAudio2MasteringVoice* gXAudioMasteringVoice;
IXAudio2SourceVoice* gXAudioSFXSourceVoice[NUMBER_OF_SFX_SOURCE_VOICES];
IXAudio2SourceVoice* gXAudioMusicSourceVoice;
uint8_t gSFXSourceVoiceSelector;
float gSFXVolume = 0.5f;
float gMusicVolume = 0.5f;

GAMESOUND gSound_MenuNavigate;
GAMESOUND gSound_MenuChoose;
GAMESOUND gSound_SplashScreen;

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


	if (LoadRegistryParameters() != ERROR_SUCCESS)
	{
		goto Exit;
	}

	LogMessageA(LL_INFO, "[%s] %s %s is starting...", __FUNCTION__, GAME_NAME, GAME_VERSION);

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

	if (InitializeSoundEngine() != S_OK)
	{
		MessageBoxA(NULL, "InitializeSoundEngine failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	if (FAILED(LoadWavFromFile("Assets\\MenuSelect.wav", &gSound_MenuNavigate)))
	{
		MessageBoxA(NULL, "Initialize gMenuMoveSound failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	if (FAILED(LoadWavFromFile("Assets\\MenuReturn.wav", &gSound_MenuChoose)))
	{
		MessageBoxA(NULL, "Initialize gMenuMoveSound failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	if (FAILED(LoadWavFromFile("Assets\\SplashScreen.wav", &gSound_SplashScreen)))
	{
		MessageBoxA(NULL, "Initialize gMenuMoveSound failded!", "Error", MB_ICONEXCLAMATION | MB_OK);
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

	if ((gBackBuffer.Memory) == NULL)
	{
		MessageBoxA(NULL, "Failded to allocate memory for drawing surface!", "Error!",
					MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

	if (InitializeHero() != ERROR_SUCCESS)
	{
		MessageBoxA(NULL, "Failed to initialize hero!", "Error!",
					MB_ICONEXCLAMATION | MB_OK);
		goto Exit;
	}

	gGameIsRunning = TRUE;

	while (gGameIsRunning == TRUE)
	{
		QueryPerformanceCounter((LARGE_INTEGER*)&FrameStart);

		while (PeekMessageA(&Message, gGameWindow, 0, 0, PM_REMOVE))
		{
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

			FindFirstConnectedGamepad();

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
			else
			{
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

	if (gPerformanceData.MonitorWidth / GAME_RES_WIDTH > gPerformanceData.MonitorHeight / GAME_RES_HEIGHT)
	{
		gPerformanceData.CurrentScaleFactor = (uint8_t)(gPerformanceData.MonitorHeight / GAME_RES_HEIGHT);
		gPerformanceData.MaxScaleFactor = gPerformanceData.CurrentScaleFactor;
	}
	else
	{
		gPerformanceData.CurrentScaleFactor = (uint8_t)(gPerformanceData.MonitorWidth / GAME_RES_WIDTH);
		gPerformanceData.MaxScaleFactor = gPerformanceData.CurrentScaleFactor;
	}


	if (gPerformanceData.WindowWidth == 0 ||
		gPerformanceData.WindowHeight == 0 ||
		gPerformanceData.WindowWidth > gPerformanceData.MonitorWidth ||
		gPerformanceData.WindowHeight > gPerformanceData.MonitorHeight)
	{
		LogMessageA(LL_INFO, "[%s] The window size was setup to the monitor maximun resolution.", __FUNCTION__);
		gPerformanceData.WindowHeight = gPerformanceData.CurrentScaleFactor * GAME_RES_HEIGHT;
		gPerformanceData.WindowWidth = gPerformanceData.CurrentScaleFactor * GAME_RES_WIDTH;
	}

	gPerformanceData.WindowsPosX = (uint16_t)(gPerformanceData.MonitorWidth / 2) - (uint16_t)(gPerformanceData.WindowWidth / 2);
	gPerformanceData.WindowsPosY = (uint16_t)(gPerformanceData.MonitorHeight / 2) - (uint16_t)(gPerformanceData.WindowHeight / 2);

	if (SetWindowLongPtrA(gGameWindow, GWL_STYLE, WS_VISIBLE) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

	if (SetWindowPos(gGameWindow, HWND_TOP,
					 gPerformanceData.WindowsPosX,
					 gPerformanceData.WindowsPosY,
					 gPerformanceData.WindowWidth,
					 gPerformanceData.WindowHeight,
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

	gGameInput.EscapeKeyIsDown = GetAsyncKeyState(VK_ESCAPE);
	gGameInput.DebugKeyIsDown = GetAsyncKeyState(VK_F1);
	gGameInput.LeftKeyIsDown = GetAsyncKeyState(VK_LEFT) | GetAsyncKeyState('A');
	gGameInput.RightKeyIsDown = GetAsyncKeyState(VK_RIGHT) | GetAsyncKeyState('D');
	gGameInput.UpKeyIsDown = GetAsyncKeyState(VK_UP) | GetAsyncKeyState('W');
	gGameInput.DownKeyIsDown = GetAsyncKeyState(VK_DOWN) | GetAsyncKeyState('S');
	gGameInput.EnterKeyIsDown = GetAsyncKeyState(VK_RETURN);


	if (gGamepadID > -1)
	{
		if (XInputGetState(gGamepadID, &gGamepadState) == ERROR_SUCCESS)
		{
			gGameInput.EscapeKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;
			gGameInput.LeftKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
			gGameInput.RightKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
			gGameInput.UpKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
			gGameInput.DownKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
			gGameInput.EnterKeyIsDown |= gGamepadState.Gamepad.wButtons & XINPUT_GAMEPAD_A;
		}
		else
		{
			// Gamepad unplugged?
			gGamepadID = -1;
			gPreviousGameState = gCurrentGameState;
			gCurrentGameState = GAMESTATE_GAMEPADUNPLUGGED;
		}
	}

	if (gGameInput.DebugKeyIsDown && !gGameInput.DebugKeyWasDown)
	{
		gPerformanceData.DisplayDebugInfo = !gPerformanceData.DisplayDebugInfo;
	}

	if (gGameInput.EscapeKeyIsDown && !gGameInput.EscapeKeyWasDown)
	{
		GoBack();
	}

	switch (gCurrentGameState)
	{

		case GAMESTATE_OPENINGSPLASHSCREEN:
		{
			PPI_OpeningSplashScreen();

			break;
		}

		case GAMESTATE_TITLESCREEN:
		{
			PPI_Menu(&gMenu_TitleScreen);

			break;
		}

		case  GAMESTATE_OVERWORLD:
		{
			PPI_Overworld();

			break;
		}

		case GAMESTATE_EXITYESNOSCREEN:
		{
			PPI_Menu(&gMenu_ExitYesNoScreen);

			break;
		}

		case GAMESTATE_BATTLE:
		{
			break;
		}

		case GAMESTATE_OPTIONS:
		{
			PPI_Menu(&gMenu_OptionScreen);
			break;
		}

		case GAMESTATE_GAMEPADUNPLUGGED:
		{
			PPI_Menu(&gMenu_GamepadUnpluggedScreen);
			if (gGamepadID > -1)
			{
				gCurrentGameState = gPreviousGameState;
				gPreviousGameState = GAMESTATE_TITLESCREEN;
			}
			break;
		}

		default:
		{
			ASSERT(FALSE, "Unknown game state!");
		}
	}

	gGameInput.EscapeKeyWasDown = gGameInput.EscapeKeyIsDown;
	gGameInput.DebugKeyWasDown = gGameInput.DebugKeyIsDown;
	gGameInput.LeftKeyWasDown = gGameInput.LeftKeyIsDown;
	gGameInput.RightKeyWasDown = gGameInput.RightKeyIsDown;
	gGameInput.UpKeyWasDown = gGameInput.UpKeyIsDown;
	gGameInput.DownKeyWasDown = gGameInput.DownKeyIsDown;
	gGameInput.EnterKeyWasDown = gGameInput.EnterKeyIsDown;

Exit:
	return;
}

void RenderFrameGraphics(void)
{

	switch (gCurrentGameState)
	{

		case  GAMESTATE_OVERWORLD:
		{

			//  ClearScreenEx
			PIXEL32 Pixel = { 0xff, 0x00, 0x00, 0xff };

			ClearScreenEx(&Pixel);
			Blit32BppBitmapToBuffer(&gPlayer.Sprite[gPlayer.CurrentArmor][gPlayer.Direction + gPlayer.Step], gPlayer.ScreenPosX, gPlayer.ScreenPosY);

			break;
		}

		case GAMESTATE_OPENINGSPLASHSCREEN:
		{
			DrawOpeningSplashScreen();
			break;
		}

		case GAMESTATE_TITLESCREEN:
		{
			DrawMenu(&gMenu_TitleScreen);
			break;
		}
		case GAMESTATE_EXITYESNOSCREEN:
		{
			DrawMenu(&gMenu_ExitYesNoScreen);
			break;
		}

		case GAMESTATE_GAMEPADUNPLUGGED:
		{
			DrawMenu(&gMenu_GamepadUnpluggedScreen);
			break;
		}

		case GAMESTATE_BATTLE:
		{
			break;
		}

		case GAMESTATE_OPTIONS:
		{
			DrawMenu(&gMenu_OptionScreen);
			DrawOptionsValues();
			break;
		}

		default:
		{
			ASSERT(FALSE, "Gamestate not implemented!");
		}
	}

	ShowDebugInformation();

	HDC DeviceContext = GetDC(gGameWindow);

	StretchDIBits(DeviceContext,
				  0,
				  0,
				  gPerformanceData.WindowWidth,
				  gPerformanceData.WindowHeight,
				  0,
				  0,
				  GAME_RES_WIDTH,
				  GAME_RES_HEIGHT,
				  gBackBuffer.Memory,
				  &gBackBuffer.BitmapInfo,
				  DIB_RGB_COLORS,
				  SRCCOPY);

	ReleaseDC(gGameWindow, DeviceContext);
Exit:
	return;
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

	if (Result == ERROR_SUCCESS)
	{
		LogMessageA(LL_INFO, "[%s]Succesfully loaded: [%s]", __FUNCTION__, FileName);
	}
	else
	{
		LogMessageA(LL_ERROR, "[%s]Failed to load bitmap asset: [%s] with error 0x%0blx!", __FUNCTION__, FileName, Result);
	}


	if (FileHandle && FileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(FileHandle);
	}

	return(Result);
}

DWORD InitializeHero(void)
{
	DWORD Result = ERROR_SUCCESS;

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

void UpdateHeroMovement(_Inout_ HERO* Hero, _In_ DIRECTION Direction)
{
	Hero->CurrentFrames++;

	if (Hero->CurrentFrames == Hero->SkipFrames)
	{
		Hero->CurrentFrames = 0;

		switch (Hero->PendingMovements)
		{
			case 0:
			{
				Hero->Step = 1;
				break;
			}

			case 4:
			{
				Hero->Step = 0;
				break;
			}

			case 8:
			{
				Hero->Step = 2;
				break;
			}

			case 12:
			{
				Hero->Step = 0;
				break;
			}

			case 16:
			{
				Hero->Step = 1;
				break;
			}

			default:
				break;
		}

		if (Hero->PendingMovements--)
		{

			switch (Hero->Direction)
			{
				case DIR_DOWN:
				{
					if (Hero->ScreenPosY < GAME_RES_HEIGHT - 16)
						Hero->ScreenPosY += 1;
					break;
				}
				case DIR_UP:
				{
					if (Hero->ScreenPosY > 0)
						Hero->ScreenPosY -= 1;
					break;
				}
				case DIR_LEFT:
				{
					if (Hero->ScreenPosX > 0)
						Hero->ScreenPosX -= 1;
					break;
				}
				case DIR_RIGHT:
				{
					if (Hero->ScreenPosX < GAME_RES_WIDTH - 16)
						Hero->ScreenPosX += 1;
					break;
				}
			}
		}
		else
		{
			Hero->PendingMovements = 16;
			Hero->Direction = Direction;
		}
	}
}

void BlitStringToScreen(_In_ char* String, _In_ GAMEBITMAP* FontSheet, _In_ PIXEL32* Color, _In_ uint16_t x, _In_ uint16_t y)
{
	uint32_t CharWidth = FontSheet->BitmapInfo.bmiHeader.biWidth / FONT_SHEET_CHARACTERS_PER_ROW;
	uint32_t CharHeight = FontSheet->BitmapInfo.bmiHeader.biHeight;

	uint32_t BytesPerCharacter = (CharWidth * CharHeight * (FontSheet->BitmapInfo.bmiHeader.biBitCount / 8));
	size_t StringLength = strlen(String);

	GAMEBITMAP StringBitmap = { 0 };

	StringBitmap.BitmapInfo.bmiHeader.biBitCount = GAME_BPP;
	StringBitmap.BitmapInfo.bmiHeader.biHeight = CharHeight;
	StringBitmap.BitmapInfo.bmiHeader.biWidth = CharWidth * (uint32_t)StringLength;
	StringBitmap.BitmapInfo.bmiHeader.biPlanes = 1;
	StringBitmap.BitmapInfo.bmiHeader.biCompression = BI_RGB;

	StringBitmap.Memory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (size_t)(BytesPerCharacter * StringLength));

	for (uint32_t Character = 0; Character < StringLength; Character++)
	{
		uint32_t StartingFontSheetPixel = 0;
		uint32_t FontSheetOffset = 0;
		uint32_t StringBitmapOffset;
		PIXEL32 FontSheetPixel = { 0 };
		uint8_t SelectedCharacter = String[Character];

		StartingFontSheetPixel = (FontSheet->BitmapInfo.bmiHeader.biWidth * FontSheet->BitmapInfo.bmiHeader.biHeight) - FontSheet->BitmapInfo.bmiHeader.biWidth + (CharWidth * charToPixelOffset[SelectedCharacter]);

		for (uint32_t YPixel = 0; YPixel < CharHeight; YPixel++)
		{
			for (uint32_t XPixel = 0; XPixel < CharWidth; XPixel++)
			{
				FontSheetOffset = StartingFontSheetPixel + XPixel - (FontSheet->BitmapInfo.bmiHeader.biWidth * YPixel);

				StringBitmapOffset = (Character * CharWidth) + ((StringBitmap.BitmapInfo.bmiHeader.biWidth * StringBitmap.BitmapInfo.bmiHeader.biHeight) - \
																StringBitmap.BitmapInfo.bmiHeader.biWidth) + XPixel - (StringBitmap.BitmapInfo.bmiHeader.biWidth) * YPixel;

				memcpy_s(&FontSheetPixel, sizeof(PIXEL32), (PIXEL32*)FontSheet->Memory + FontSheetOffset, sizeof(PIXEL32));

				if (FontSheetPixel.Alpha == 255)
				{
					memcpy_s((PIXEL32*)StringBitmap.Memory + StringBitmapOffset, sizeof(PIXEL32), Color, sizeof(PIXEL32));
				}


			}
		}

	}

	Blit32BppBitmapToBuffer(&StringBitmap, x, y);

	if (StringBitmap.Memory)
	{
		HeapFree(GetProcessHeap(), 0, StringBitmap.Memory);
	}
}

DWORD LoadRegistryParameters(void)
{
	DWORD Result = ERROR_SUCCESS;

	HKEY RegKey = NULL;
	DWORD RegDisposition = 0;
	DWORD RegBytesRead = sizeof(DWORD);

	Result = RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\" GAME_NAME, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &RegKey, &RegDisposition);

	if (Result != ERROR_SUCCESS)
	{
		LogMessageA(LL_ERROR, "[%s] RegCreateKey failed with error code 0x%08lx!", __FUNCTION__, Result);
		goto Exit;
	}

	if (RegDisposition == REG_CREATED_NEW_KEY)
	{
		LogMessageA(LL_INFO, "[%s] Registry key did not exists; created new key HKCU\\SOFTWARE\\%s", __FUNCTION__, GAME_NAME);
	}
	else
	{
		LogMessageA(LL_INFO, "[%s] Openened exisiting registry key HKCU\\SOFTWARE\\%s", __FUNCTION__, GAME_NAME);

	}

	Result = RegGetValueA(RegKey, NULL, "LogLevel", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.LogLevel, &RegBytesRead);

	if (Result != ERROR_SUCCESS)
	{
		if (Result == ERROR_FILE_NOT_FOUND)
		{
			Result = ERROR_SUCCESS;
			LogMessageA(LL_INFO, "[%s] Registry value 'LogLevel' not found. Using default of 0. (LOG_LEVEL_NONE)", __FUNCTION__);
			gRegistryParams.LogLevel = LL_NONE;
		}
		else
		{
			LogMessageA(LL_ERROR, "[%s] Failed to read the 'LogLevel' registry value! Error 0x%08lx!", __FUNCTION__, Result);

			goto Exit;
		}

	}

	Result = RegGetValueA(RegKey, NULL, "WindowWidth", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.WindowWidth, &RegBytesRead);

	if (Result != ERROR_SUCCESS)
	{
		if (Result == ERROR_FILE_NOT_FOUND)
		{
			Result = ERROR_SUCCESS;
			LogMessageA(LL_INFO, "[%s] Registry value 'WindowWidth' not found. Using default of 0.", __FUNCTION__);
			gRegistryParams.WindowWidth = 0;
		}
		else
		{
			LogMessageA(LL_ERROR, "[%s] Failed to read the 'WindowWidth' registry value! Error 0x%08lx!", __FUNCTION__, Result);

			goto Exit;
		}
	}

	gPerformanceData.WindowWidth = gRegistryParams.WindowWidth;

	Result = RegGetValueA(RegKey, NULL, "WindowHeight", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.WindowHeight, &RegBytesRead);

	if (Result != ERROR_SUCCESS)
	{
		if (Result == ERROR_FILE_NOT_FOUND)
		{
			Result = ERROR_SUCCESS;
			LogMessageA(LL_INFO, "[%s] Registry value 'WindowHeight' not found. Using default of 0.", __FUNCTION__);
			gRegistryParams.WindowHeight = 0;
		}
		else
		{
			LogMessageA(LL_ERROR, "[%s] Failed to read the 'WindowHeight' registry value! Error 0x%08lx!", __FUNCTION__, Result);

			goto Exit;
		}

	}

	gPerformanceData.WindowHeight = gRegistryParams.WindowHeight;

	Result = RegGetValueA(RegKey, NULL, "SFXVolume", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.SFXVolume, &RegBytesRead);

	if (Result != ERROR_SUCCESS)
	{
		if (Result == ERROR_FILE_NOT_FOUND)
		{
			Result = ERROR_SUCCESS;
			LogMessageA(LL_INFO, "[%s] Registry value 'SFXVolume' not found. Using default of 50.", __FUNCTION__);
			gRegistryParams.SFXVolume = 50;
		}
		else
		{
			LogMessageA(LL_ERROR, "[%s] Failed to read the 'SFXVolume' registry value! Error 0x%08lx!", __FUNCTION__, Result);

			goto Exit;
		}

	}

	Result = RegGetValueA(RegKey, NULL, "MusicVolume", RRF_RT_DWORD, NULL, (BYTE*)&gRegistryParams.MusicVolume, &RegBytesRead);

	if (Result != ERROR_SUCCESS)
	{
		if (Result == ERROR_FILE_NOT_FOUND)
		{
			Result = ERROR_SUCCESS;
			LogMessageA(LL_INFO, "[%s] Registry value 'MusicVolume' not found. Using default of 50.", __FUNCTION__);
			gRegistryParams.MusicVolume = 50;
		}
		else
		{
			LogMessageA(LL_ERROR, "[%s] Failed to read the 'MusicVolume' registry value! Error 0x%08lx!", __FUNCTION__, Result);

			goto Exit;
		}

	}

	LogMessageA(LL_INFO, "[%s] LogLevel is %d.", __FUNCTION__, gRegistryParams.LogLevel);

	gSFXVolume = (float)gRegistryParams.SFXVolume / 100.0f;
	gMusicVolume = (float)gRegistryParams.MusicVolume / 100.0f;

Exit:
	if (RegKey)
	{
		RegCloseKey(RegKey);
	}

	return(Result);
}

void LogMessageA(_In_ LOGLEVEL LogLevel, _In_ char* Message, _In_ ...)
{
	size_t MessageLength = strlen(Message);
	SYSTEMTIME Time = { 0 };
	HANDLE LogFileHandle = INVALID_HANDLE_VALUE;
	DWORD EndOfFile = 0;
	DWORD NumberOfBytesWritten = 0;
	char DateTimeString[96] = { 0 };
	char SeverityString[9] = { 0 };
	char FormattedString[4096] = { 0 };
	int Error = 0;

	if (gRegistryParams.LogLevel < (DWORD)LogLevel)
	{
		return;
	}


	if (MessageLength < 1 || MessageLength >= 4096)
	{
		ASSERT(FALSE, "[%s][%s][%s]The length of the message is to short or to long", __FILE__, __FUNCTION__, __LINE__);
		return;
	}

	switch (LogLevel)
	{
		case LL_NONE:
		{
			return;
		}
		case LL_INFO:
		{
			strcpy_s(SeverityString, sizeof(SeverityString), "[INFO] ");
			break;
		}
		case LL_WARN:
		{
			strcpy_s(SeverityString, sizeof(SeverityString), "[WARN] ");
			break;
		}
		case LL_ERROR:
		{
			strcpy_s(SeverityString, sizeof(SeverityString), "[ERROR] ");
			break;
		}
		case LL_DEBUG:
		{
			strcpy_s(SeverityString, sizeof(SeverityString), "[DEBUG] ");
			break;
		}
		default:
		{
			ASSERT(FALSE, "Invalid Log level value.");
		}
	}

	GetLocalTime(&Time);
	va_list ArgPointer = NULL;
	va_start(ArgPointer, Message);

	_vsnprintf_s(FormattedString, sizeof(FormattedString), _TRUNCATE, Message, ArgPointer);
	va_end(ArgPointer);

	Error = _snprintf_s(DateTimeString, sizeof(DateTimeString), _TRUNCATE, "\r\n[%02u/%02u/%u %02u:%02u:%02u.%03u]",
						Time.wYear, Time.wDay, Time.wMonth, Time.wHour, Time.wMinute, Time.wSecond, Time.wMilliseconds);

	if ((LogFileHandle = CreateFileA(LOG_FILE_NAME, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
	{
		return;
	}

	EndOfFile = SetFilePointer(LogFileHandle, 0, NULL, FILE_END);

	WriteFile(LogFileHandle, DateTimeString, (DWORD)strlen(DateTimeString), &NumberOfBytesWritten, NULL);
	WriteFile(LogFileHandle, SeverityString, (DWORD)strlen(SeverityString), &NumberOfBytesWritten, NULL);
	WriteFile(LogFileHandle, FormattedString, (DWORD)strlen(FormattedString), &NumberOfBytesWritten, NULL);
Exit:
	if (LogFileHandle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(LogFileHandle);
	}
}

void FindFirstConnectedGamepad(void)
{
	gGamepadID = -1;

	for (int8_t GamepadIndex = 0; GamepadIndex < XUSER_MAX_COUNT && gGamepadID == -1; GamepadIndex++)
	{
		XINPUT_STATE State = { 0 };

		if (XInputGetState(GamepadIndex, &State) == ERROR_SUCCESS)
		{
			gGamepadID = GamepadIndex;
		}
	}
}

void ShowDebugInformation(void)
{
	char DebugTextBuffer[64] = { 0 };

	if (gPerformanceData.DisplayDebugInfo == TRUE)
	{
		PIXEL32 Green = { 0x54, 0xF2, 0xFF, 0xFF };

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPS RAW:  %.01f", gPerformanceData.RawFPSAverage);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, &Green, 0, 0);

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "FPS Cooked: %.01f", gPerformanceData.CookedFPSAverage);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, &Green, 0, 8);

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Handles: %lu", gPerformanceData.HandleCount);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, &Green, 0, 16);

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Memory: %lluKB", (uint64_t)gPerformanceData.MemInfo.PrivateUsage / 1024);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, &Green, 0, 24);

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "CPU: %0.2f%%", gPerformanceData.CPUPercent);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, &Green, 0, 32);

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "ScreenPos: (%d, %d)", gPlayer.ScreenPosX, gPlayer.ScreenPosY);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, &Green, 0, 40);

		sprintf_s(DebugTextBuffer, sizeof(DebugTextBuffer), "Total Frames: %llu", gPerformanceData.TotalFramesRendered);
		BlitStringToScreen(DebugTextBuffer, &g6x7Font, &Green, 0, 48);
	}
}

void DrawOpeningSplashScreen(void)
{
	static uint64_t LocalFrameCounter;

	static uint64_t LastFrameSeen;

	const static uint8_t BrightnessModifier = 32;

	static PIXEL32 Color = { 0xFF, 0xFF, 0xFF, 0xFF };

	if (gPerformanceData.TotalFramesRendered > (LastFrameSeen + 1))
	{
		LocalFrameCounter = 0;
	}

	memset(gBackBuffer.Memory, 0, GAME_DRAWING_AREA_MEMORY_SIZE);

	if (LocalFrameCounter >= 60)
	{

		if (LocalFrameCounter == 60)
		{
			PlayGameSound(&gSound_SplashScreen);
		}

		if (LocalFrameCounter >= 120 && LocalFrameCounter % 10 == 0)
		{
			Color.Blue -= BrightnessModifier;
			Color.Red -= BrightnessModifier;
			Color.Green -= BrightnessModifier;
		}

		if (LocalFrameCounter >= 190)
		{
			Color.Blue = 0;
			Color.Red = 0;
			Color.Green = 0;
		}

		if (LocalFrameCounter == 200)
		{
			gPreviousGameState = GAMESTATE_TITLESCREEN;
			gCurrentGameState = GAMESTATE_TITLESCREEN;
		}


		BlitStringToScreen("-Poncho's Studio-", &g6x7Font, &Color, CENTER(18), 100);
		BlitStringToScreen("Presents", &g6x7Font, &Color, CENTER(9), 115);
	}

	LocalFrameCounter++;
	LastFrameSeen = gPerformanceData.TotalFramesRendered;
}

void DrawVolumenBar(_In_ float* Volume, _In_ uint16_t x, _In_ uint16_t y)
{
	PIXEL32 White = { 0xFF, 0xFF, 0xFF, 0xFF };
	PIXEL32 Grey = { 0x60, 0x60, 0x60, 0xFF };

	for (uint8_t Counter = 0; Counter < 10; Counter++)
	{

		if ((Counter * 10) < (uint8_t)(*Volume * 100))
		{
			BlitStringToScreen(
				"\xF2",
				&g6x7Font,
				&White,
				x + ((uint16_t)Counter * 6),
				y);
		}
		else
		{
			BlitStringToScreen(
				"\xF2",
				&g6x7Font,
				&Grey,
				x + ((uint16_t)Counter * 6),
				y);
		}
	}
}

void DrawOptionsValues(void)
{
	PIXEL32 White = { 0xFF, 0xFF, 0xFF, 0xFF };

	// Draw The values to the screen
	char Buffer[12];

	DrawVolumenBar(&gSFXVolume, RIGHT_STR(gMenu_OptionScreen.Items[0]->Name, gMenu_OptionScreen.Items[0]->x, 15), gMenu_OptionScreen.Items[0]->y);
	DrawVolumenBar(&gMusicVolume, RIGHT_STR(gMenu_OptionScreen.Items[1]->Name, gMenu_OptionScreen.Items[1]->x, 3), gMenu_OptionScreen.Items[1]->y);

	sprintf_s(Buffer, 11, "%ux%u", gPerformanceData.WindowWidth, gPerformanceData.WindowHeight);
	BlitStringToScreen(
		Buffer,
		&g6x7Font,
		&White,
		RIGHT_STR(gMenu_OptionScreen.Items[2]->Name, gMenu_OptionScreen.Items[2]->x, 15),
		gMenu_OptionScreen.Items[2]->y);
}

void DrawMenu(_Inout_ MENU* Menu)
{
	if (gPerformanceData.TotalFramesRendered > (Menu->LastFrameSeen + 1))
	{
		Menu->LastFrameSeen = 0;
		Menu->LocalFrameCounter = 0;
		Menu->SelectedItem = 0;

		Menu->ActiveForegroundColor->Blue = Menu->BackgroundColor->Blue;
		Menu->ActiveForegroundColor->Red = Menu->BackgroundColor->Red;
		Menu->ActiveForegroundColor->Green = Menu->BackgroundColor->Green;
		Menu->ActiveForegroundColor->Alpha = Menu->BackgroundColor->Alpha;
	}


	//	  AARRGGBB
	//	0xFFFF0000 -> Red
	//	0xFF00FF00 -> Green
	//	0xFF0000FF -> Blue
	DWORD BackColor = (Menu->BackgroundColor->Blue << 0) | (Menu->BackgroundColor->Green << 8) | (Menu->BackgroundColor->Red << 16) | (Menu->BackgroundColor->Alpha << 24);

	__stosd(gBackBuffer.Memory, BackColor, GAME_DRAWING_AREA_MEMORY_SIZE / sizeof(DWORD));


	if (Menu->LocalFrameCounter < 90 && !Menu->HaveBeenDraw)
	{
		if (Menu->LocalFrameCounter == 0)
		{
			Menu->ActiveForegroundColor->Blue = Menu->BackgroundColor->Blue;
			Menu->ActiveForegroundColor->Red = Menu->BackgroundColor->Red;
			Menu->ActiveForegroundColor->Green = Menu->BackgroundColor->Green;
			Menu->ActiveForegroundColor->Alpha = Menu->BackgroundColor->Alpha;
		}

		if (Menu->LocalFrameCounter >= 10 && Menu->LocalFrameCounter % 10 == 0)
		{
			Menu->ActiveForegroundColor->Blue = (Menu->ForegroundColor->Blue  * (uint8_t)Menu->LocalFrameCounter / 80);
			Menu->ActiveForegroundColor->Red = (Menu->ForegroundColor->Red * (uint8_t)Menu->LocalFrameCounter / 80);
			Menu->ActiveForegroundColor->Green = (Menu->ForegroundColor->Green * (uint8_t)Menu->LocalFrameCounter / 80);
		}

		if (Menu->LocalFrameCounter > 80)
		{
			Menu->ActiveForegroundColor->Blue = Menu->ForegroundColor->Blue;
			Menu->ActiveForegroundColor->Red = Menu->ForegroundColor->Red;
			Menu->ActiveForegroundColor->Green = Menu->ForegroundColor->Green;
			Menu->ActiveForegroundColor->Alpha = Menu->ForegroundColor->Alpha;
			Menu->HaveBeenDraw = TRUE;
		}

		BlitStringToScreen(Menu->Name, &g6x7Font, Menu->ActiveForegroundColor, CENTER(strlen(Menu->Name)), 60);
		
		for (uint8_t MenuItem = 0; MenuItem < Menu->ItemCount; MenuItem++)
		{
			BlitStringToScreen(
				Menu->Items[MenuItem]->Name,
				&g6x7Font,
				Menu->ActiveForegroundColor,
				Menu->Items[MenuItem]->x,
				Menu->Items[MenuItem]->y);
		}

		BlitStringToScreen(
			"\xBB",
			&g6x7Font,
			Menu->ActiveForegroundColor,
			Menu->Items[Menu->SelectedItem]->x - 6,
			Menu->Items[Menu->SelectedItem]->y);
	}
	else
	{
		BlitStringToScreen(Menu->Name, &g6x7Font, Menu->ForegroundColor, CENTER(strlen(Menu->Name)), 60);
		for (uint8_t MenuItem = 0; MenuItem < Menu->ItemCount; MenuItem++)
		{
			BlitStringToScreen(
				Menu->Items[MenuItem]->Name,
				&g6x7Font,
				Menu->ForegroundColor,
				Menu->Items[MenuItem]->x,
				Menu->Items[MenuItem]->y);
		}

		BlitStringToScreen(
			"\xBB",
			&g6x7Font,
			Menu->ForegroundColor,
			Menu->Items[Menu->SelectedItem]->x - 6,
			Menu->Items[Menu->SelectedItem]->y);
	}


	Menu->LocalFrameCounter++;
	Menu->LastFrameSeen = gPerformanceData.TotalFramesRendered;

}

void MenuItem_TitleScreen_Resume(void)
{
	gPreviousGameState = gCurrentGameState;
	gCurrentGameState = GAMESTATE_OVERWORLD;
}

void MenuItem_TitleScreen_StartNew(void)
{
	gPlayer.ScreenPosX = 32;
	gPlayer.ScreenPosY = 32;

	gPlayer.CurrentArmor = SUIT_0;
	gPlayer.Direction = FACING_DOWN_0;
	gPlayer.PendingMovements = 0;
	gPlayer.SkipFrames = 3;
	gPlayer.CurrentFrames = 0;

	gPlayer.Active = TRUE;

	gMenu_TitleScreen.Items = gMI_TitleScreenItemWithResume;
	gMenu_TitleScreen.ItemCount = _countof(gMI_TitleScreenItemWithResume);

	gPreviousGameState = gCurrentGameState;
	gCurrentGameState = GAMESTATE_OVERWORLD;
}

void MenuItem_TitleScreen_Options(void)
{
	gPreviousGameState = gCurrentGameState;
	gCurrentGameState = GAMESTATE_OPTIONS;
}

void MenuItem_TitleScreen_Exit(void)
{
	gPreviousGameState = gCurrentGameState;
	gCurrentGameState = GAMESTATE_EXITYESNOSCREEN;
}

void MenuItem_ExitYesNo_Yes(void)
{
	SendMessageA(gGameWindow, WM_CLOSE, 0, 0);
}

void MenuItem_ExitYesNo_No(void)
{
	GoBack();
}

void MenuItem_OptionsScren_SFXVolume(void)
{
	gSFXVolume += 0.1f;
	if (gSFXVolume >= 1.1f)
	{
		gSFXVolume = 0.0f;
	}

	UpdateSoundVolume(gXAudioSFXSourceVoice, NUMBER_OF_SFX_SOURCE_VOICES, gSFXVolume);
}

void MenuItem_OptionsScren_MusicVolume(void)
{

	gMusicVolume += 0.1f;
	if (gMusicVolume >= 1.1f)
	{
		gMusicVolume = 0.0f;
	}

	UpdateSoundVolume(&gXAudioMusicSourceVoice, 1, gMusicVolume);
}

void MenuItem_OptionsScren_Resolution(void)
{

	gPerformanceData.CurrentScaleFactor -= 1;

	if (gPerformanceData.CurrentScaleFactor == 0)
	{
		gPerformanceData.CurrentScaleFactor = gPerformanceData.MaxScaleFactor;
	}

	gPerformanceData.WindowWidth = gPerformanceData.CurrentScaleFactor * GAME_RES_WIDTH;
	gPerformanceData.WindowHeight = gPerformanceData.CurrentScaleFactor * GAME_RES_HEIGHT;

	gPerformanceData.WindowsPosX = (uint16_t)(gPerformanceData.MonitorWidth / 2) - (uint16_t)(gPerformanceData.WindowWidth / 2);
	gPerformanceData.WindowsPosY = (uint16_t)(gPerformanceData.MonitorHeight / 2) - (uint16_t)(gPerformanceData.WindowHeight / 2);

	SetWindowPos(gGameWindow, HWND_TOP,
				 gPerformanceData.WindowsPosX,
				 gPerformanceData.WindowsPosY,
				 gPerformanceData.WindowWidth,
				 gPerformanceData.WindowHeight,
				 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

}

void PPI_Menu(_In_ MENU* Menu)
{
	if (gGameInput.DownKeyIsDown && !gGameInput.DownKeyWasDown)
	{
		if (++Menu->SelectedItem >= Menu->ItemCount)
		{
			Menu->SelectedItem = 0;
		}

		PlayGameSound(&gSound_MenuNavigate);
	}
	else if (gGameInput.UpKeyIsDown && !gGameInput.UpKeyWasDown)
	{
		if (--Menu->SelectedItem >= Menu->ItemCount)
		{
			Menu->SelectedItem = Menu->ItemCount - 1;
		}

		PlayGameSound(&gSound_MenuNavigate);
	}
	else if (gGameInput.EnterKeyIsDown && !gGameInput.EnterKeyWasDown)
	{
		Menu->Items[Menu->SelectedItem]->Action();
		PlayGameSound(&gSound_MenuChoose);
	}
}

void PPI_OpeningSplashScreen(void)
{

}

void PPI_Overworld(void)
{
	int16_t RightOrLeftIsDown = gGameInput.LeftKeyIsDown || gGameInput.RightKeyIsDown;

	if (gGameInput.LeftKeyIsDown)
	{
		UpdateHeroMovement(&gPlayer, DIR_LEFT);
	}

	if (gGameInput.RightKeyIsDown)
	{
		UpdateHeroMovement(&gPlayer, DIR_RIGHT);
	}

	if (gGameInput.UpKeyIsDown && !RightOrLeftIsDown)
	{
		UpdateHeroMovement(&gPlayer, DIR_UP);
	}

	if (gGameInput.DownKeyIsDown && !RightOrLeftIsDown)
	{
		UpdateHeroMovement(&gPlayer, DIR_DOWN);
	}

	if (gPlayer.PendingMovements)
	{
		UpdateHeroMovement(&gPlayer, gPlayer.Direction);
	}
}

HRESULT InitializeSoundEngine(void)
{
	HRESULT Result = S_OK;

	WAVEFORMATEX SfxWaveFormat = { 0 };
	WAVEFORMATEX MusicWaveFormat = { 0 };


	Result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

	if (Result != S_OK)
	{
		LogMessageA(LL_ERROR, "[%s] CoInitializeEx failed with 0x%08lx!", __FUNCTION__, Result);
		goto Exit;
	}

	if (FAILED(Result = XAudio2Create(&gXAudio, 0, XAUDIO2_ANY_PROCESSOR)))
	{
		LogMessageA(LL_ERROR, "[%s] XAudio2Create failed with 0x%08lx!", __FUNCTION__, Result);
		goto Exit;
	}

	if (FAILED(Result = gXAudio->lpVtbl->CreateMasteringVoice(gXAudio, &gXAudioMasteringVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, NULL, 0)))
	{
		LogMessageA(LL_ERROR, "[%s] CreateMasteringVoice failed with 0x%08lx!", __FUNCTION__, Result);
		goto Exit;
	}

	SfxWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	SfxWaveFormat.nChannels = 1; // Mono
	SfxWaveFormat.nSamplesPerSec = 44100;
	SfxWaveFormat.nAvgBytesPerSec = SfxWaveFormat.nSamplesPerSec * SfxWaveFormat.nChannels * 2;
	SfxWaveFormat.nBlockAlign = SfxWaveFormat.nChannels * 2;
	SfxWaveFormat.wBitsPerSample = 16;
	SfxWaveFormat.cbSize = 0x6164;

	for (uint8_t Counter = 0; Counter < NUMBER_OF_SFX_SOURCE_VOICES; Counter++)
	{
		Result = gXAudio->lpVtbl->CreateSourceVoice(gXAudio, &gXAudioSFXSourceVoice[Counter], &SfxWaveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL);

		if (FAILED(Result))
		{
			LogMessageA(LL_ERROR, "[%s] CreateSourceVoice with Counter %i failed with 0x%08lx!", __FUNCTION__, Counter, Result);
			goto Exit;
		}

		gXAudioSFXSourceVoice[Counter]->lpVtbl->SetVolume(gXAudioSFXSourceVoice[Counter], gSFXVolume, XAUDIO2_COMMIT_NOW);
	}

	MusicWaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	MusicWaveFormat.nChannels = 2; // Stereo
	MusicWaveFormat.nSamplesPerSec = 44100;
	MusicWaveFormat.nAvgBytesPerSec = MusicWaveFormat.nSamplesPerSec * MusicWaveFormat.nChannels * 2;
	MusicWaveFormat.nBlockAlign = MusicWaveFormat.nChannels * 2;
	MusicWaveFormat.wBitsPerSample = 16;
	MusicWaveFormat.cbSize = 0;

	Result = gXAudio->lpVtbl->CreateSourceVoice(gXAudio, &gXAudioMusicSourceVoice, &MusicWaveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL);
	if (FAILED(Result))
	{
		LogMessageA(LL_ERROR, "[%s] CreateSourceVoice for  gXAudioMusicSourceVoice failed with 0x%08lx!", __FUNCTION__, Result);
		goto Exit;
	}

	gXAudioMusicSourceVoice->lpVtbl->SetVolume(gXAudioMusicSourceVoice, gMusicVolume, XAUDIO2_COMMIT_NOW);

Exit:
	return(Result);
}

DWORD LoadWavFromFile(_In_ char* FileName, _Inout_ GAMESOUND* GameSound)
{
	DWORD Result = ERROR_SUCCESS;
	DWORD NumberOfBytesRead = 0;
	DWORD Riff = 0;
	uint16_t DataChunkOffset = 0;
	DWORD DataChunkSearcher = 0;
	BOOL DataChunkFound = FALSE;
	DWORD DataChunkSize = 0;
	void* AudioData = NULL;

	HANDLE FileHandle = INVALID_HANDLE_VALUE;
	FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (GetLastError() == ERROR_FILE_NOT_FOUND)
	{
		Result = ERROR_FILE_NOT_FOUND;

		LogMessageA(LL_ERROR, "[%s]Failded loading Wav from disk, file not found", __FUNCTION__);

		goto Exit;
	}

	if (ReadFile(FileHandle, &Riff, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
	{
		Result = GetLastError();

		goto Exit;
	}

	if (Riff != 0x46464952) // "RIFF" Backwards
	{
		Result = ERROR_FILE_INVALID;

		LogMessageA(LL_ERROR, "[%s]First four bytes are not 'RIFF'", __FUNCTION__);

		goto Exit;
	}

	if (SetFilePointer(FileHandle, 20, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		Result = GetLastError();

		LogMessageA(LL_ERROR, "[%s]SetFilePointer failed with 0x%08lx!", __FUNCTION__, Result);

		goto Exit;
	}

	if (ReadFile(FileHandle, &GameSound->WaveFormat, sizeof(WAVEFORMATEX), &NumberOfBytesRead, NULL) == 0)
	{
		Result = GetLastError();
		LogMessageA(LL_ERROR, "[%s]ReadFile failed with 0x%08lx!", __FUNCTION__, Result);

		goto Exit;
	}

	if (GameSound->WaveFormat.nBlockAlign != ((GameSound->WaveFormat.nChannels * GameSound->WaveFormat.wBitsPerSample) / 8) ||
		(GameSound->WaveFormat.wFormatTag != WAVE_FORMAT_PCM) ||
		(GameSound->WaveFormat.wBitsPerSample != 16))
	{
		Result = ERROR_DATATYPE_MISMATCH;

		LogMessageA(LL_ERROR, "[%s]This wav file dis not meet the format requirements: Only PCM, 44.1KHz, 16 bit bits per sample wav files are supported. Error 0x%08lx!", __FUNCTION__, Result);

		goto Exit;
	}

	while (DataChunkFound == FALSE)
	{

		if (SetFilePointer(FileHandle, DataChunkOffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			Result = GetLastError();

			LogMessageA(LL_ERROR, "[%s]SetFilePointer failed with 0x%08lx!", __FUNCTION__, Result);

			goto Exit;
		}

		if (ReadFile(FileHandle, &DataChunkSearcher, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
		{
			Result = GetLastError();
			LogMessageA(LL_ERROR, "[%s]ReadFile failed with 0x%08lx!", __FUNCTION__, Result);

			goto Exit;
		}

		if (DataChunkSearcher == 0x61746164) // 'data', backwards
		{
			DataChunkFound = TRUE;
		}
		else
		{
			DataChunkOffset += 4;
		}

		if (DataChunkOffset > 256)
		{
			Result = ERROR_DATATYPE_MISMATCH;

			LogMessageA(LL_ERROR, "[%s]Data chunk not found within first 256 bytes of this file 0x%08lx!", __FUNCTION__, Result);

			goto Exit;
		}
	}

	if (SetFilePointer(FileHandle, DataChunkOffset + 4, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		Result = GetLastError();

		LogMessageA(LL_ERROR, "[%s]SetFilePointer failed with 0x%08lx!", __FUNCTION__, Result);

		goto Exit;
	}

	if (ReadFile(FileHandle, &DataChunkSize, sizeof(DWORD), &NumberOfBytesRead, NULL) == 0)
	{
		Result = GetLastError();

		LogMessageA(LL_ERROR, "[%s]ReadFile failed with 0x%08lx!", __FUNCTION__, Result);

		goto Exit;
	}

	AudioData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DataChunkSize);
	if (AudioData == NULL)
	{
		Result = ERROR_NOT_ENOUGH_MEMORY;

		LogMessageA(LL_ERROR, "[%s]HeapAlloc failed with 0x%08lx!", __FUNCTION__, Result);

		goto Exit;
	}

	if (SetFilePointer(FileHandle, DataChunkOffset + 8, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{
		Result = GetLastError();

		LogMessageA(LL_ERROR, "[%s]SetFilePointer failed with 0x%08lx!", __FUNCTION__, Result);

		goto Exit;
	}

	if (ReadFile(FileHandle, AudioData, DataChunkSize, &NumberOfBytesRead, NULL) == 0)
	{
		Result = GetLastError();

		LogMessageA(LL_ERROR, "[%s]ReadFile failed with 0x%08lx!", __FUNCTION__, Result);

		goto Exit;
	}

	GameSound->Buffer.pAudioData = AudioData;

	GameSound->Buffer.Flags = XAUDIO2_END_OF_STREAM;
	GameSound->Buffer.AudioBytes = DataChunkSize;

Exit:

	if (Result == ERROR_SUCCESS)
	{
		LogMessageA(LL_INFO, "[%s]Succesfully loaded asset: [%s]", __FUNCTION__, FileName);
	}
	else
	{
		LogMessageA(LL_ERROR, "[%s]Failed to load wav asset: [%s] with error 0x%0blx!", __FUNCTION__, FileName, Result);
	}

	if (FileHandle && (FileHandle != INVALID_HANDLE_VALUE))
	{
		CloseHandle(FileHandle);
	}

	return(Result);
}

void PlayGameSound(_In_ GAMESOUND* GameSound)
{
	gXAudioSFXSourceVoice[gSFXSourceVoiceSelector]->lpVtbl->SubmitSourceBuffer(gXAudioSFXSourceVoice[gSFXSourceVoiceSelector], &GameSound->Buffer, NULL);

	gXAudioSFXSourceVoice[gSFXSourceVoiceSelector]->lpVtbl->Start(gXAudioSFXSourceVoice[gSFXSourceVoiceSelector], 0, XAUDIO2_COMMIT_NOW);

	if (++gSFXSourceVoiceSelector >= NUMBER_OF_SFX_SOURCE_VOICES)
	{
		gSFXSourceVoiceSelector = 0;
	}
}

void GoBack(void)
{
	gDesiredGameState = gPreviousGameState;
	gPreviousGameState = gCurrentGameState;
	gCurrentGameState = gDesiredGameState;
	PlayGameSound(&gSound_MenuChoose);
}

void  UpdateSoundVolume(_Inout_ IXAudio2SourceVoice** SoundVoice, _In_ uint8_t Count, _In_ float Volume)
{
	for (uint8_t Counter = 0; Counter < Count; Counter++)
	{
		SoundVoice[Counter]->lpVtbl->SetVolume(SoundVoice[Counter], Volume, XAUDIO2_COMMIT_NOW);
	}
}

#ifdef AVX2
__forceinline void ClearScreen(_In_ __m512i* Color)
{
	for (int x = 0; x < ((GAME_RES_WIDTH * GAME_RES_HEIGHT) / 16); x++)
	{
		_mm512_store_si512((__m512i*)gBackBuffer.Memory + x, *Color);
	}
}
#elif defined AVX
__forceinline void ClearScreen(_In_ __m256i* Color)
{
	for (int x = 0; x < ((GAME_RES_WIDTH * GAME_RES_HEIGHT) / 8); x++)
	{
		_mm256_store_si256((__m256i*)gBackBuffer.Memory + x, *Color);
	}
}
#elif defined SSE2
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

__forceinline void ClearScreenEx(_In_ PIXEL32* Pixel)
{
#ifdef AVX2
	__m512i SexiPixel = {
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha
	};

	ClearScreen(&SexiPixel);
#elif defined AVX

	__m256i OctaPixel = {
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha
	};

	ClearScreen(&OctaPixel);

#elif defined SSE2
	__m128i QuadPixel = {
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha,
		Pixel->Blue, Pixel->Green, Pixel->Red, Pixel->Alpha
	};
	ClearScreen(&QuadPixel);
#else
	ClearScreen(Pixel);
#endif
}
