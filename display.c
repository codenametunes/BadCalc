#include "display.h"
#include "theme.h"
#include "gfx.h"

void Display_Init(Display* d)
{
    ZeroMemory(d, sizeof(*d));
    Display_Clear(d);
}

// copy a string in, chop it if its too long. no mercy
void Display_Set(Display* d, const wchar_t* s)
{
    int i = 0;
    while (s[i] && i < DISPLAY_MAX - 1) { d->text[i] = s[i]; i++; }
    d->text[i] = 0;
    d->len = i;
}

void Display_Append(Display* d, wchar_t ch)
{
    // buffer full? sorry, no more numbers today
    if (d->len >= DISPLAY_MAX - 1) return;
    d->text[d->len++] = ch;
    d->text[d->len] = 0;
}

void Display_Backspace(Display* d)
{
    // never let it go empty, an empty display is just sad
    if (d->len <= 1) { Display_Clear(d); return; }
    d->text[--d->len] = 0;
}

void Display_Clear(Display* d)
{
    Display_Set(d, L"0");
}

void Display_Draw(const Display* d, HDC dc, HFONT font)
{
    HGDIOBJ oldFont;
    RECT sh = d->rc;
    RECT r = d->rc;

    // same hard shadow as the buttons. consistency. we have standards (low ones)
    OffsetRect(&sh, UI_SHADOW, UI_SHADOW);
    Gfx_FillRect(dc, &sh, COL_INK);
    Gfx_Box(dc, &d->rc, COL_DISP_BG, COL_INK, UI_BORDER);

    r.left += 12; r.right -= 12;
    oldFont = SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, COL_DISP_TEXT);
    DrawTextW(dc, d->text, -1, &r, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, oldFont);
}