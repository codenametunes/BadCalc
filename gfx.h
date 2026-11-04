#pragma once
#include <windows.h>

void Gfx_FillRect(HDC dc, const RECT* r, COLORREF c);
void Gfx_Box(HDC dc, const RECT* r, COLORREF fill, COLORREF border, int thick);