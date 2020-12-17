#pragma once

#define AVX

#ifdef AVX2
#include <immintrin.h>
#endif

#ifdef AVX
#include <immintrin.h>
#endif

#ifdef SSE2
#include <emmintrin.h>
#endif


#ifdef _DEBUG
#define ASSERT(Expression, Message, ...)								\
				if(!(Expression))										\
				{														\
					LogMessageA(LL_ERROR, Message, ##__VA_ARGS__ );		\
					goto Exit;											\
				}
#else
#define ASSERT(Expresion, Message, ...)
#endif                                                             \

#define GAME_NAME		"Game_B"
#define GAME_VERSION	"0.27"
#define GAME_RES_WIDTH	384
#define GAME_RES_HEIGHT 240
#define GAME_BPP		32
#define GAME_DRAWING_AREA_MEMORY_SIZE (GAME_RES_WIDTH * GAME_RES_HEIGHT * (GAME_BPP / 8))
#define CALCULATE_AVG_FPS_EVERY_X_FRAMES 120
#define TARGET_MICROSECONDS_PER_FRAME 16667ULL
#define NUMBER_OF_SFX_SOURCE_VOICES 4


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

#define FONT_SHEET_CHARACTERS_PER_ROW 98
#define LOG_FILE_NAME	GAME_NAME ".log"

#pragma warning(disable: 4820) // Disable warning about structure padding.
#pragma warning(disable: 5045) // Disable warning about Spectre/Meltdown CPU vulnerability.
#pragma warning(disable: 4710) // Disable warning about inline imposibility on _snprintf_S

typedef LONG(NTAPI* _NtQueryTimerResolution) (OUT PULONG MinimumResolution, OUT PULONG MaximumResolution, OUT PULONG CurrentResolution);
_NtQueryTimerResolution NtQueryTimerResolution;

typedef enum DIRECTION
{
	DIR_DOWN = 0,
	DIR_LEFT = 3,
	DIR_RIGHT = 6,
	DIR_UP = 9
} DIRECTION;

typedef struct GAMEINPUT
{
	int16_t EscapeKeyIsDown;
	int16_t DebugKeyIsDown;
	int16_t LeftKeyIsDown;
	int16_t RightKeyIsDown;
	int16_t UpKeyIsDown;
	int16_t DownKeyIsDown;
	int16_t EnterKeyIsDown;

	int16_t EscapeKeyWasDown;
	int16_t DebugKeyWasDown;
	int16_t LeftKeyWasDown;
	int16_t RightKeyWasDown;
	int16_t UpKeyWasDown;
	int16_t DownKeyWasDown;
	int16_t EnterKeyWasDown;
} GAMEINPUT;

typedef enum LOGLEVEL
{
	LL_NONE = 0,
	LL_ERROR = 1,
	LL_INFO = 2,
	LL_WARN = 3,
	LL_DEBUG = 4
} LOGLEVEL;

typedef enum GAMESTATE
{
	GAMESTATE_OPENINGSPLASHSCREEN,
	GAMESTATE_TITLESCREEN,
	GAMESTATE_OVERWORLD,
	GAMESTATE_BATTLE,
	GAMESTATE_OPTIONS,
	GAMESTATE_EXITYESNOSCREEN,
	GAMESTATE_GAMEPADUNPLUGGED
} GAMESTATE;

typedef struct GAMEBITMAP
{
	BITMAPINFO BitmapInfo;
	void* Memory;
} GAMEBITMAP;


typedef struct GAMESOUND
{
	WAVEFORMATEX WaveFormat;
	XAUDIO2_BUFFER Buffer;
} GAMESOUND;

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
	uint32_t MonitorWidth;
	uint32_t MonitorHeight;

	uint32_t WindowWidth;
	uint32_t WindowHeight;

	BOOL DisplayDebugInfo;

	PROCESS_MEMORY_COUNTERS_EX MemInfo;
	DWORD HandleCount;

	SYSTEM_INFO SystemInfo;
	int64_t PreviousSystemTime;
	int64_t CurrentSystemTime;

	double CPUPercent;

	uint8_t MaxScaleFactor;
	uint8_t CurrentScaleFactor;

	uint16_t WindowsPosX;
	uint16_t WindowsPosY;

} GAMEPERFDATA;

typedef struct HERO
{
	char Name[12];

	GAMEBITMAP Sprite[3][12];

	int16_t ScreenPosX;
	int16_t ScreenPosY;

	uint8_t SkipFrames;
	uint8_t CurrentFrames;

	BOOL Active;

	uint8_t PendingMovements;
	DIRECTION Direction;
	uint8_t CurrentArmor;
	uint8_t Step;

	int32_t HP;
	int32_t Strength;
	int32_t MP;
} HERO;

typedef struct REGISTRYPARAMS
{
	DWORD LogLevel;
	
	DWORD SFXVolume;
	DWORD MusicVolume;

	DWORD WindowWidth;
	DWORD WindowHeight;
} REGISTRYPARAMS;

DWORD CreateMainGameWindow(_In_ HINSTANCE Instance);

BOOL GameIsAlreadyRunning(void);

void ProcessPlayerInput(void);

void RenderFrameGraphics(void);

DWORD Load32BppBitmapFromFile(_In_ char* FileName, _Inout_ GAMEBITMAP* GameBitmap);

DWORD InitializeHero(void);

void Blit32BppBitmapToBuffer(_In_ GAMEBITMAP* GameBitmap, _In_ uint16_t x, _In_ uint16_t y);

void UpdateHeroMovement(_Inout_ HERO* Hero, _In_ DIRECTION Direction);

void BlitStringToScreen(_In_ char* String, _In_ GAMEBITMAP* FontSheet, _In_ PIXEL32* Color, _In_ uint16_t x, _In_ uint16_t y);

DWORD LoadRegistryParameters(void);

void LogMessageA(_In_ LOGLEVEL LogLevel, _In_ char* Message, _In_ ...);

void FindFirstConnectedGamepad(void);

void ShowDebugInformation(void);

void DrawOpeningSplashScreen(void);

void DrawVolumenBar(_In_ float* Volume, _In_ uint16_t x, _In_ uint16_t y);

void DrawOptionsValues(void);

void PPI_OpeningSplashScreen(void);

void PPI_Overworld(void);

HRESULT InitializeSoundEngine(void);

DWORD LoadWavFromFile(_In_ char* FileName, _Inout_ GAMESOUND* GameSound);

void PlayGameSound(_In_ GAMESOUND* GameSound);

void GoBack(void);

void UpdateSoundVolume(_Inout_ IXAudio2SourceVoice** SoundVoice, _In_ uint8_t Count, _In_ float Volume);

#ifdef AVX2
void ClearScreen(_In_ __m512i* Color);
#elif defined AVX
void ClearScreen(_In_ __m256i* Color);
#elif defined SSE2
void ClearScreen(_In_ __m128i* Color);
#else
void ClearScreen(_In_ PIXEL32* Color);
#endif

void ClearScreenEx(_In_ PIXEL32* Pixel);