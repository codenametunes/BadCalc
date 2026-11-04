#include "gfx.h"

void Gfx_FillRect(HDC dc, const RECT* r, COLORREF c)
{
    HBRUSH br = CreateSolidBrush(c);
    FillRect(dc, r, br);
    DeleteObject(br);   // forget this once and gdi slowly eats your ram. ask me how i know
	// PLEASE don't delete it you'll cause a white screen. gdi is a cruel mistress
    // my mistress' eyes are nothing like the sun
}

// no actual border drawing here. the border is just the background showing through. galaxy brain
void Gfx_Box(HDC dc, const RECT* r, COLORREF fill, COLORREF border, int thick)
{
    RECT in = *r;
    Gfx_FillRect(dc, r, border);
    InflateRect(&in, -thick, -thick);
    Gfx_FillRect(dc, &in, fill);
}