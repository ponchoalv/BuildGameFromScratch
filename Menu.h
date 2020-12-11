#pragma once

#define CENTER(Width) ((GAME_RES_WIDTH / 2) - (((uint16_t)Width * 6) / 2))
#define RIGHT(Width, LeftMargin, RightMargin) ((uint16_t)LeftMargin + ((uint16_t)Width * 6) + (uint16_t)RightMargin)
#define RIGHT_STR(String, LeftMargin, RightMargin) RIGHT(strlen(String), LeftMargin, RightMargin)

typedef struct MENUITEM
{
	char* Name;
	int16_t x;
	int16_t y;
	void(*Action)(void);
} MENUITEM;

typedef struct MENU
{
	char* Name;
	uint8_t SelectedItem;
	uint8_t ItemCount;
	MENUITEM** Items;
	uint64_t LocalFrameCounter;
	uint64_t LastFrameSeen;
	PIXEL32* BackgroundColor;
	PIXEL32* ForegroundColor;
	PIXEL32* ActiveForegroundColor;
	BOOL HaveBeenDraw;
} MENU;

void PPI_Menu(_In_ MENU* Menu);

void MenuItem_TitleScreen_Resume(void);
void MenuItem_TitleScreen_StartNew(void);
void MenuItem_TitleScreen_Options(void);
void MenuItem_TitleScreen_Exit(void);

void DrawMenu(_Inout_ MENU* Menu);

PIXEL32 gMenu_TitleActiveColor = { 0x00, 0x00, 0x00, 0xFF };
PIXEL32 gMenu_Black = { 0x00, 0x00, 0x00, 0xFF };
PIXEL32 gMenu_Yellow = { 0x00, 0xB6, 0xFF, 0xFF };
PIXEL32 gMenu_Red = { 0x00, 0x00, 0xFF, 0xFF };
PIXEL32 gMenu_White = { 0xFF, 0xFF, 0xFF, 0xFF };

// Title screen
MENUITEM gMI_ResumeGame = { "Resume", CENTER(14), 100, MenuItem_TitleScreen_Resume };
MENUITEM gMI_StartNewGame = { "Start New Game", CENTER(14), 120, MenuItem_TitleScreen_StartNew };
MENUITEM gMI_Options = { "Options", CENTER(14), 140, MenuItem_TitleScreen_Options };
MENUITEM gMI_Exit = { "Exit", CENTER(14), 160, MenuItem_TitleScreen_Exit };

MENUITEM* gMI_TitleScreenItem[] = {&gMI_StartNewGame, &gMI_Options, &gMI_Exit }; // Without Resume
MENUITEM* gMI_TitleScreenItemWithResume[] = { &gMI_ResumeGame, &gMI_StartNewGame, &gMI_Options, &gMI_Exit }; // With Resume

MENU gMenu_TitleScreen = { "The Turing Adventure", 0, _countof(gMI_TitleScreenItem),  gMI_TitleScreenItem, 0, 0, &gMenu_Black, &gMenu_White, &gMenu_TitleActiveColor, FALSE };

/// Exit Yes or No Screen
PIXEL32 gMenu_YesNoActiveColor = { 0x00, 0x00, 0x00, 0xFF };
void MenuItem_ExitYesNo_Yes(void);
void MenuItem_ExitYesNo_No(void);

MENUITEM gMI_ExitYesNo_Yes = { "Yes", CENTER(3), 100, MenuItem_ExitYesNo_Yes };
MENUITEM gMI_ExitYesNo_No = { "No", CENTER(3), 115, MenuItem_ExitYesNo_No };

MENUITEM* gMI_ExitYesNoItem[] = { &gMI_ExitYesNo_No , &gMI_ExitYesNo_Yes };

MENU gMenu_ExitYesNoScreen = { "Are you sure you want to exit?", 1, _countof(gMI_ExitYesNoItem), gMI_ExitYesNoItem, 0, 0,  &gMenu_Black, &gMenu_Red, &gMenu_YesNoActiveColor, FALSE };


/// GamePad Unplugged
PIXEL32 gMenu_GamepadUnpluggedActiveColor = { 0x00, 0x00, 0x00, 0xFF };
MENUITEM gMI_GamepadUnplugged_Continue = { "Continue", CENTER(9), 100, MenuItem_ExitYesNo_No };

MENUITEM* gMI_GamepadUnplugged_ContinueItems[] = { &gMI_GamepadUnplugged_Continue };

MENU gMenu_GamepadUnpluggedScreen = { "The gamepad was unpplugged...", 0, _countof(gMI_GamepadUnplugged_ContinueItems), gMI_GamepadUnplugged_ContinueItems, 0, 0, &gMenu_Black, &gMenu_Yellow, &gMenu_GamepadUnpluggedActiveColor, TRUE };

// Options Screen
PIXEL32 gMenu_OptionsScreenActiveColor = { 0x00, 0x00, 0x00, 0xFF };

void MenuItem_OptionsScren_SFXVolume(void);
void MenuItem_OptionsScren_MusicVolume(void);
void MenuItem_OptionsScren_Resolution(void);

MENUITEM gMI_OptionScreen_SFXVolume = {"SFX Volume:", CENTER(24), 100, MenuItem_OptionsScren_SFXVolume};
MENUITEM gMI_OptionScreen_MusicVolume = {"Music Volume:", CENTER(24), 115, MenuItem_OptionsScren_MusicVolume };
MENUITEM gMI_OptionScreen_Resolution = {"Resolution:", CENTER(24), 130, MenuItem_OptionsScren_Resolution };
MENUITEM gMI_OptionScreen_Back = {"Back", CENTER(24), 145, MenuItem_ExitYesNo_No };

MENUITEM* gMI_OptionScreen_Items[] = {&gMI_OptionScreen_SFXVolume, &gMI_OptionScreen_MusicVolume, &gMI_OptionScreen_Resolution, &gMI_OptionScreen_Back };

MENU gMenu_OptionScreen = { "Options", 0, _countof(gMI_OptionScreen_Items), gMI_OptionScreen_Items, 0, 0, &gMenu_Black, &gMenu_White, &gMenu_OptionsScreenActiveColor, TRUE };
