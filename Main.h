#pragma once

#define GAME_NAME		"GAME_B"
#define GAME_RES_WIDTH	384
#define GAME_RES_HEIGHT 240
#define GAME_BPP		32
#define GAME_DRAWING_AREA_MEMORY_SIZE (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))
#define CALCULATE_AVG_FPS_EVERY_X_FRAMES 120
#define TARGET_MICROSECONDS_PER_FRAME 16667ULL
#define SIMD
#define SUIT_0 0
#define SUIT_1 1
#define SUIT_2 2
#define FACING_DOWN_0		0
#define FACING_DOWN_1		1
#define FACING_DOWN_2		2
#define FACING_LEFT_0		3
#define FACING_LEFT_1		4
#define FACING_LEFT_2		5
#define FACING_RIGHT_0		6
#define FACING_RIGHT_1		7
#define FACING_RIGHT_2		8
#define FACING_UPWARD_0		9
#define FACING_UPWARD_1		10
#define FACING_UPWARD_2		11




#pragma warning(disable: 4820) // Disable warning about structure padding.
#pragma warning(disable: 5045) // Disable warning about Spectre/Meltdown CPU vulnerability.
#pragma warning(disable: 4710) // Disable warning about inline imposibility on _snprintf_S

typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG CurrentResolution);
_NtQueryTimerResolution NtQueryTimerResolution;

typedef struct GAMEBITMAP
{
	BITMAPINFO BitmapInfo;
	void* Memory;
} GAMEBITMAP;

typedef struct PIXEL32
{
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;
} PIXEL32;

typedef struct GAMEPERFDATA
{
	uint64_t TotalFramesRendered;

	float RawFPSAverage;
	float CookedFPSAverage;

	int64_t PerfFrequency;

	MONITORINFO MonitorInfo;
	int32_t MonitorWidth;
	int32_t MonitorHeight;

	BOOL DisplayDebugInfo;

	PROCESS_MEMORY_COUNTERS_EX MemInfo;
	DWORD HandleCount;
	
	SYSTEM_INFO SystemInfo;
	int64_t PreviousSystemTime;
	int64_t CurrentSystemTime;

	double CPUPercent;
} GAMEPERFDATA;

typedef struct HERO
{
	char Name[12];

	GAMEBITMAP Sprite[3][12];

	int8_t Direction;
	int16_t StepAccumulator;
	int8_t Step;
	int8_t Suit;

	int16_t ScreenPosX;
	int16_t ScreenPosY;

	int32_t HP;
	int32_t Strength;
	int32_t MP;
} HERO;

DWORD CreateMainGameWindow(_In_ HINSTANCE Instance);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RenderFrameGraphics(void);

DWORD Load32BppBitmapFromFile(_In_ char* FileName, _Inout_ GAMEBITMAP* GameBitmap);

DWORD InitializeHero(void);

void Load32BppBitmapOnScreen(_In_ GAMEBITMAP* GameBitmap, _In_ int16_t ScreenPosX, _In_ int16_t ScreenPosY);

void IncrementHeroSteps(_Inout_ HERO* Hero, _In_ int8_t Direction);

#ifdef SIMD
void ClearScreen(_In_ __m128i* Color);
#else
void ClearScreen(_In_ PIXEL32* Color);
#endif