#include "button.h"
#include "theme.h"
#include "gfx.h"

// kind -> color. a switch, nothing to see here
static COLORREF BaseColor(ButtonKind k)
{
    switch (k) {
    case BTN_OP:     return COL_OP;
    case BTN_ACTION: return COL_ACTION;
    case BTN_FUNC:   return COL_FUNC;
    case BTN_EQUALS: return COL_EQUALS;
    default:         return COL_DIGIT;
    }
}

void Button_Init(Button* b, int id, const wchar_t* label, ButtonKind kind)
{
    ZeroMemory(b, sizeof(*b));
    b->id = id;
    b->label = label;
    b->kind = kind;
}

void Button_Draw(const Button* b, HDC dc)
{
    RECT face = b->rc;
    COLORREF c = BaseColor(b->kind);

    if (b->down) 
        OffsetRect(&face, UI_SHADOW, UI_SHADOW);
    else 
    {
        RECT sh = b->rc;
        OffsetRect(&sh, UI_SHADOW, UI_SHADOW);
        Gfx_FillRect(dc, &sh, COL_INK);
    }

    Gfx_Box(dc, &face, c, COL_INK, UI_BORDER);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, COL_INK);
    DrawTextW(dc, b->label, -1, &face, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

int Button_HitTest(const Button* b, int x, int y)
{
    POINT p = { x, y };
    return PtInRect(&b->rc, p);
}