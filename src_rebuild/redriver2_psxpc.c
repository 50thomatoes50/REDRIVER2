﻿// redriver2_psxpc.cpp
//

#include "THISDUST.H"
#include "GAME/C/MAIN.H"
#include "GAME/C/SYSTEM.H"



#include "EMULATOR.H"
#include "EMULATOR_PRIVATE.H"

#include <SDL_scancode.h>

// eq engine console output
typedef enum
{
	SPEW_NORM,
	SPEW_INFO,
	SPEW_WARNING,
	SPEW_ERROR,
	SPEW_SUCCESS,
}SpewType_t;


#ifdef _WIN32
#include <Windows.h>
#include <conio.h>
#include <sys/types.h>
#include <sys/stat.h>

static unsigned short g_InitialColor = 0xFFFF;
static unsigned short g_LastColor = 0xFFFF;
static unsigned short g_BadColor = 0xFFFF;
static WORD g_BackgroundFlags = 0xFFFF;

static void GetInitialColors()
{
	// Get the old background attributes.
	CONSOLE_SCREEN_BUFFER_INFO oldInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &oldInfo);
	g_InitialColor = g_LastColor = oldInfo.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	g_BackgroundFlags = oldInfo.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);

	g_BadColor = 0;
	if (g_BackgroundFlags & BACKGROUND_RED)
		g_BadColor |= FOREGROUND_RED;
	if (g_BackgroundFlags & BACKGROUND_GREEN)
		g_BadColor |= FOREGROUND_GREEN;
	if (g_BackgroundFlags & BACKGROUND_BLUE)
		g_BadColor |= FOREGROUND_BLUE;
	if (g_BackgroundFlags & BACKGROUND_INTENSITY)
		g_BadColor |= FOREGROUND_INTENSITY;
}

static WORD SetConsoleTextColor(int red, int green, int blue, int intensity)
{
	WORD ret = g_LastColor;

	g_LastColor = 0;
	if (red)	g_LastColor |= FOREGROUND_RED;
	if (green) g_LastColor |= FOREGROUND_GREEN;
	if (blue)  g_LastColor |= FOREGROUND_BLUE;
	if (intensity) g_LastColor |= FOREGROUND_INTENSITY;

	// Just use the initial color if there's a match...
	if (g_LastColor == g_BadColor)
		g_LastColor = g_InitialColor;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), g_LastColor | g_BackgroundFlags);
	return ret;
}

static void RestoreConsoleTextColor(WORD color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color | g_BackgroundFlags);
	g_LastColor = color;
}

CRITICAL_SECTION g_SpewCS;
bool g_bSpewCSInitted = false;

void fnConDebugSpew(SpewType_t type, char* text)
{
	// Hopefully two threads won't call this simultaneously right at the start!
	if (!g_bSpewCSInitted)
	{
		GetInitialColors();
		InitializeCriticalSection(&g_SpewCS);
		g_bSpewCSInitted = true;
	}

	WORD old;
	EnterCriticalSection(&g_SpewCS);
	{
		if (type == SPEW_NORM)
		{
			old = SetConsoleTextColor(1, 1, 1, 0);
		}
		else if (type == SPEW_WARNING)
		{
			old = SetConsoleTextColor(1, 1, 0, 1);
		}
		else if (type == SPEW_SUCCESS)
		{
			old = SetConsoleTextColor(0, 1, 0, 1);
		}
		else if (type == SPEW_ERROR)
		{
			old = SetConsoleTextColor(1, 0, 0, 1);
		}
		else if (type == SPEW_INFO)
		{
			old = SetConsoleTextColor(0, 1, 1, 1);
		}
		else
		{
			old = SetConsoleTextColor(1, 1, 1, 1);
		}

		OutputDebugStringA(text);
		printf("%s", text);

		RestoreConsoleTextColor(old);
	}
	LeaveCriticalSection(&g_SpewCS);
}
#endif

void SpewMessageToOutput(SpewType_t spewtype, char const* pMsgFormat, va_list args)
{
	static char pTempBuffer[4096];
	int len = 0;
	vsprintf(&pTempBuffer[len], pMsgFormat, args);

#ifdef WIN32
	fnConDebugSpew(spewtype, pTempBuffer);
#else
	printf(pTempBuffer);
#endif
}

void printMsg(char *fmt, ...)
{
	va_list		argptr;

	va_start(argptr, fmt);
	SpewMessageToOutput(SPEW_NORM, fmt, argptr);
	va_end(argptr);
}

void printInfo(char *fmt, ...)
{
	va_list		argptr;

	va_start(argptr, fmt);
	SpewMessageToOutput(SPEW_INFO, fmt, argptr);
	va_end(argptr);
}

void printWarning(char *fmt, ...)
{
	va_list		argptr;

	va_start(argptr, fmt);
	SpewMessageToOutput(SPEW_WARNING, fmt, argptr);
	va_end(argptr);
}

void printError(char *fmt, ...)
{
	va_list		argptr;

	va_start(argptr, fmt);
	SpewMessageToOutput(SPEW_ERROR, fmt, argptr);
	va_end(argptr);
}

int(*GPU_printf)(const char *fmt, ...);

extern int g_texturelessMode;
extern int g_wireframeMode;
int gShowCollisionDebug = 0;
extern int gDrawDistance;
extern int gDisplayPosition;
extern int gDisplayDrawStats;

extern void FunkUpDaBGMTunez(int funk);

void GameDebugKeys(int nKey, bool down)
{
	if (!down)
		return;

	if (nKey == SDL_SCANCODE_F1)
	{
		gDrawDistance -= 100;

		if (gDrawDistance < 441)
			gDrawDistance = 441;

		printf("gDrawDistance = %d\n", gDrawDistance);

	}
	else if (nKey == SDL_SCANCODE_F2)
	{
		gDrawDistance += 100;

		if (gDrawDistance > 6000)
			gDrawDistance = 6000;

		printf("gDrawDistance = %d\n", gDrawDistance);
	}
	else if (nKey == SDL_SCANCODE_F3)
	{
		gDisplayPosition ^= 1;
		printf("Position display %s\n", gDisplayPosition ? "ON" : "OFF");
	}
	else if (nKey == SDL_SCANCODE_F4)
	{
		gShowCollisionDebug ^= 1;
		printf("Collision debug %s\n", gShowCollisionDebug ? "ON" : "OFF");
	}
	else if (nKey == SDL_SCANCODE_F5)
	{
		gDisplayDrawStats ^= 1;
		printf("Stats %s\n", gDisplayDrawStats ? "ON" : "OFF");
	}
	else if (nKey == SDL_SCANCODE_KP_DIVIDE)
	{
		FunkUpDaBGMTunez(0);
	}
	else if (nKey == SDL_SCANCODE_KP_MULTIPLY)
	{
		FunkUpDaBGMTunez(1);
	}
}

#ifndef USE_CRT_MALLOC
char g_Overlay_buffer[0x50000];		// 0x1C0000
char g_Frontend_buffer[0x50000];	// 0xFB400
char g_Other_buffer[0x50000];		// 0xF3000
char g_Other_buffer2[0x50000];		// 0xE7000
OTTYPE g_OT1[OTSIZE];				// 0xF3000
OTTYPE g_OT2[OTSIZE];				// 0xF7200
char g_PrimTab1[0x1a180];			// 0xFB400
char g_PrimTab2[0x1a180];			// 0x119400
char g_Replay_buffer[0x50000];		// 0x1fabbc
#endif

int main()
{
#ifdef USE_CRT_MALLOC
	_overlay_buffer = (char*)malloc(0x50000);			// 0x1C0000
	_frontend_buffer = (char*)malloc(0x50000);			// 0xFB400
	_other_buffer = (char*)malloc(0x50000);				// 0xF3000
	_other_buffer2 = (char*)malloc(0x50000);			// 0xE7000
	_OT1 = (OTTYPE*)malloc(OTSIZE * sizeof(OTTYPE));	// 0xF3000
	_OT2 = (OTTYPE*)malloc(OTSIZE * sizeof(OTTYPE));	// 0xF7200
	_primTab1 = (char*)malloc(0x1a180);					// 0xFB400
	_primTab2 = (char*)malloc(0x1a180);					// 0x119400
	_replay_buffer = (char*)malloc(0x50000);			// 0x1fabbc
#else
	_overlay_buffer = g_Overlay_buffer;		// 0x1C0000
	_frontend_buffer = g_Frontend_buffer;	// 0xFB400
	_other_buffer = g_Other_buffer;			// 0xF3000
	_other_buffer2 = g_Other_buffer2;		// 0xE7000
	_OT1 = g_OT1;							// 0xF3000
	_OT2 = g_OT2;							// 0xF7200
	_primTab1 = g_PrimTab1;					// 0xFB400
	_primTab2 = g_PrimTab2;					// 0x119400
	_replay_buffer = g_Replay_buffer;		// 0x1fabbc
#endif

	//g_texturelessMode = 1;
	//g_wireframeMode = 1;
	gameDebugKeys = GameDebugKeys;
	gDrawDistance = 600;					// best distance

	GPU_printf = printf;

	Emulator_Initialise("REDRIVER2", 800, 600);

	redriver2_main();

	Emulator_ShutDown();
}
