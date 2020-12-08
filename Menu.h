#pragma once

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
} MENU;

void PPI_Menu(_In_ MENU* Menu);

void MenuItem_TitleScreen_Resume(void);
void MenuItem_TitleScreen_StartNew(void);
void MenuItem_TitleScreen_Options(void);
void MenuItem_TitleScreen_Exit(void);

void DrawMenu(_In_ MENU* Menu, _In_ PIXEL32* Color);

// Title screen

MENUITEM gMI_ResumeGame = { "Resume", (GAME_RES_WIDTH / 2) - ((6 * 6) / 2), 100, MenuItem_TitleScreen_Resume };

MENUITEM gMI_StartNewGame = { "Start New Game", (GAME_RES_WIDTH / 2) - ((14 * 6) / 2), 120, MenuItem_TitleScreen_StartNew };

MENUITEM gMI_Options = { "Options", (GAME_RES_WIDTH / 2) - ((8 * 6) / 2), 140, MenuItem_TitleScreen_Options };

MENUITEM gMI_Exit = { "Exit", (GAME_RES_WIDTH / 2) - ((5 * 6) / 2), 160, MenuItem_TitleScreen_Exit };

MENUITEM* gMI_TitleScreenItem[] = {&gMI_StartNewGame, &gMI_Options, &gMI_Exit };

MENUITEM* gMI_TitleScreenItemWithResume[] = { &gMI_ResumeGame, &gMI_StartNewGame, &gMI_Options, &gMI_Exit };

MENU gMenu_TitleScreen = { GAME_NAME, 0, _countof(gMI_TitleScreenItem),  gMI_TitleScreenItem };

///



/// Exit Yes or No Screen
void MenuItem_ExitYesNo_Yes(void);
void MenuItem_ExitYesNo_No(void);

MENUITEM gMI_ExitYesNo_Yes = { "Yes", (GAME_RES_WIDTH / 2) - ((3 * 6) / 2), 100, MenuItem_ExitYesNo_Yes };
MENUITEM gMI_ExitYesNo_No = { "No", (GAME_RES_WIDTH / 2) - ((2 * 6) / 2), 115, MenuItem_ExitYesNo_No };

MENUITEM* gMI_ExitYesNoItem[] = { &gMI_ExitYesNo_Yes, &gMI_ExitYesNo_No };

MENU gMenu_ExitYesNoScreen = { "Are you sure you want to exit?", 1, _countof(gMI_ExitYesNoItem), gMI_ExitYesNoItem };