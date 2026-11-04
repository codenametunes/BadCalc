#pragma once
#include <windows.h>

#define DISPLAY_MAX 32

typedef struct Display {
    RECT    rc;
    wchar_t text[DISPLAY_MAX];
    int     len;
} Display;

void Display_Init(Display* d);
void Display_Set(Display* d, const wchar_t* s);
void Display_Append(Display* d, wchar_t ch);
void Display_Backspace(Display* d);
void Display_Clear(Display* d);
void Display_Draw(const Display* d, HDC dc, HFONT font);