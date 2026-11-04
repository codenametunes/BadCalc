#pragma once
#include <windows.h>

// the vibe: chunky flat colors, thick black outlines, hard shadows. no gradients. gradients are dead to me
#define COL_BG        RGB(255, 244, 214)   // warm cream, like old paper that has seen things
#define COL_INK       RGB(24, 22, 30)      // not quite black. black with a personality

#define COL_DIGIT     RGB(255, 255, 255)
#define COL_OP        RGB(255, 208, 60)    // yellow
#define COL_ACTION    RGB(255, 120, 150)   // pink, for C and backspace
#define COL_FUNC      RGB(110, 215, 235)   // cyan, for the fancy math
#define COL_EQUALS    RGB(140, 230, 100)   // lime

#define COL_DISP_BG   RGB(24, 22, 30)
#define COL_DISP_TEXT RGB(140, 255, 120)   // terminal green, because we are hackers now

// the gap has to be bigger than the shadow or buttons start fighting
#define UI_MARGIN     16
#define UI_GAP        10
#define UI_SHADOW     4
#define UI_BORDER     2
#define DISPLAY_H     80

static void Fill(HDC dc, const RECT* r, COLORREF c)
{
    HBRUSH b = CreateSolidBrush(c);
    FillRect(dc, r, b);
    DeleteObject(b);   // forget this once and gdi slowly eats your ram. ask me how i know
}

// hard shadow + thick outline + flat fill. hands back the inner face so the text knows where to sit
static RECT Box(HDC dc, RECT r, COLORREF fill, int pressed)
{
    RECT s = r;
    if (pressed) OffsetRect(&r, UI_SHADOW, UI_SHADOW);   // pressed = sinks into its own shadow
    else { OffsetRect(&s, UI_SHADOW, UI_SHADOW); Fill(dc, &s, COL_INK); }
    Fill(dc, &r, COL_INK);
    InflateRect(&r, -UI_BORDER, -UI_BORDER);   // the border is just the background showing through. galaxy brain
    Fill(dc, &r, fill);
    return r;
}