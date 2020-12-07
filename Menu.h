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

void MenuItem_TitleScreen_Resume(void);
void MenuItem_TitleScreen_StartNew(void);
void MenuItem_TitleScreen_Options(void);
void MenuItem_TitleScreen_Exit(void);


// Title screen

MENUITEM gMI_ResumeGame = { "Resume", (GAME_RES_WIDTH / 2) - ((6 * 6) / 2), 100, MenuItem_TitleScreen_Resume };

MENUITEM gMI_StartNewGame = { "Start New Game", (GAME_RES_WIDTH / 2) - ((14 * 6) / 2), 120, MenuItem_TitleScreen_StartNew };

MENUITEM gMI_Options = { "Options", (GAME_RES_WIDTH / 2) - ((8 * 6) / 2), 140, MenuItem_TitleScreen_Options };

MENUITEM gMI_Exit = { "Exit", (GAME_RES_WIDTH / 2) - ((5 * 6) / 2), 160, MenuItem_TitleScreen_Exit };

MENUITEM* gMI_TitleScreenItem[] = { &gMI_ResumeGame, &gMI_StartNewGame, &gMI_Options, &gMI_Exit };

MENU gMenu_TitleScreen = { "Title Screen Menu", 0, _countof(gMI_TitleScreenItem),  gMI_TitleScreenItem };

///