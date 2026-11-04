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

// sizes. the gap has to be bigger than the shadow or buttons start fighting
#define UI_MARGIN     16
#define UI_GAP        10
#define UI_SHADOW     4
#define UI_BORDER     2
#define DISPLAY_H     80

static __inline COLORREF Theme_Shade(COLORREF c, int d)
{
    int r = GetRValue(c) + d, g = GetGValue(c) + d, b = GetBValue(c) + d;
    r = r < 0 ? 0 : (r > 255 ? 255 : r);
    g = g < 0 ? 0 : (g > 255 ? 255 : g);
    b = b < 0 ? 0 : (b > 255 ? 255 : b);
    return RGB(r, g, b);
}