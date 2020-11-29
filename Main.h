#pragma once

#define GAME_NAME		"GAME_B"
#define GAME_RES_WIDTH	384
#define GAME_RES_HEIGHT 240
#define GAME_BPP		32
#define GAME_DRAWING_AREA_MEMORY_SIZE (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))
#define CALCULATE_AVG_FPS_EVERY_X_FRAMES 120
#define TARGET_MICROSECONDS_PER_FRAME 16667
#define SIMD

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
	int64_t ProcessCreationTime;
	int64_t ProcessExitTime;
	int64_t CurrentUserCPUTime;
	int64_t CurrentKernelCPUTime;
	int64_t PreviousUserCPUTime;
	int64_t PreviousKernelCPUTime;

	double CPUPercent;
} GAMEPERFDATA;

typedef struct PLAYER
{
	char Name[12];

	int32_t WorldPosX;
	int32_t WorldPosY;

	int32_t HP;
	int32_t Strength;
	int32_t MP;
} PLAYER;

DWORD CreateMainGameWindow(_In_ HINSTANCE Instance);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RenderFrameGraphics(void);


#ifdef SIMD
void ClearScreen(_In_ __m128i* Color);
#else
void ClearScreen(_In_ PIXEL32* Color);
#endif