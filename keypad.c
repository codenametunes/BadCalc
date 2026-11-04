#include "keypad.h"
#include "theme.h"

typedef struct KeyDef {
    int            id;
    const wchar_t* label;
    ButtonKind     kind;
    int            col, row, cspan;   // cspan = how many columns it eats
} KeyDef;

// the whole keypad in one table. if a button is wrong its this tables fault, not mine
static const KeyDef KEYS[KEYPAD_MAX] = {
    { 'C',       L"C",      BTN_ACTION, 0, 0, 1 },
    { 'B',       L"<-",     BTN_ACTION, 1, 0, 1 },
    { KEY_SQR,   L"x^2",    BTN_FUNC,   2, 0, 1 },
    { KEY_SQRT,  L"sqr",    BTN_FUNC,   3, 0, 1 },

    { '7',       L"7",      BTN_DIGIT,  0, 1, 1 },
    { '8',       L"8",      BTN_DIGIT,  1, 1, 1 },
    { '9',       L"9",      BTN_DIGIT,  2, 1, 1 },
    { '/',       L"/",      BTN_OP,     3, 1, 1 },

    { '4',       L"4",      BTN_DIGIT,  0, 2, 1 },
    { '5',       L"5",      BTN_DIGIT,  1, 2, 1 },
    { '6',       L"6",      BTN_DIGIT,  2, 2, 1 },
    { '*',       L"*",      BTN_OP,     3, 2, 1 },

    { '1',       L"1",      BTN_DIGIT,  0, 3, 1 },
    { '2',       L"2",      BTN_DIGIT,  1, 3, 1 },
    { '3',       L"3",      BTN_DIGIT,  2, 3, 1 },
    { '-',       L"-",      BTN_OP,     3, 3, 1 },

    { '~',       L"+/-",    BTN_FUNC,   0, 4, 1 },
    { '0',       L"0",      BTN_DIGIT,  1, 4, 1 },
    { '.',       L".",      BTN_DIGIT,  2, 4, 1 },
    { '+',       L"+",      BTN_OP,     3, 4, 1 },

    // equals takes the whole bottom row. it thinks it is better than you
    { '=',       L"=",      BTN_EQUALS, 0, 5, 4 },
};

void Keypad_Init(Keypad* k)
{
    int i;
    ZeroMemory(k, sizeof(*k));
    k->count = KEYPAD_MAX;
    k->hot = -1;
    k->pressed = -1;
    for (i = 0; i < k->count; i++)
        Button_Init(&k->btn[i], KEYS[i].id, KEYS[i].label, KEYS[i].kind);
}

// divide the area into a grid. its just division, we can do this
void Keypad_Layout(Keypad* k, const RECT* area)
{
    int gap = UI_GAP, i;
    int w = area->right - area->left;
    int h = area->bottom - area->top;
    int cw = (w - gap * (KEYPAD_COLS - 1)) / KEYPAD_COLS;
    int ch = (h - gap * (KEYPAD_ROWS - 1)) / KEYPAD_ROWS;

    // tiny window = negative sizes = bad time. clamp it
    if (cw < 0) cw = 0;
    if (ch < 0) ch = 0;

    for (i = 0; i < k->count; i++) {
        const KeyDef* d = &KEYS[i];
        RECT* r = &k->btn[i].rc;
        r->left = area->left + d->col * (cw + gap);
        r->right = r->left + d->cspan * cw + (d->cspan - 1) * gap;
        r->top = area->top + d->row * (ch + gap);
        r->bottom = r->top + ch;
    }
}

void Keypad_Draw(const Keypad* k, HDC dc, HFONT font)
{
    int i;
    HGDIOBJ old = SelectObject(dc, font);
    for (i = 0; i < k->count; i++)
        Button_Draw(&k->btn[i], dc);
    SelectObject(dc, old);
}

// which button is under the mouse? linear search, there are 21 of them, relax
static int HitIndex(const Keypad* k, int x, int y)
{
    int i;
    for (i = 0; i < k->count; i++)
        if (Button_HitTest(&k->btn[i], x, y)) return i;

    return -1;
}

int Keypad_MouseMove(Keypad* k, int x, int y)
{
    int h = HitIndex(k, x, y);
    if (h == k->hot) return 0;   // nothing changed, dont repaint, save the cpu
    if (k->hot >= 0) k->btn[k->hot].hot = 0;
    if (h >= 0)      k->btn[h].hot = 1;
    k->hot = h;

    return 1;
}

int Keypad_MouseLeave(Keypad* k)
{
    if (k->hot < 0) return 0;
    k->btn[k->hot].hot = 0;
    k->hot = -1;
    if (k->pressed >= 0) k->btn[k->pressed].down = 0;
    return 1;
}

int Keypad_MouseDown(Keypad* k, int x, int y)
{
    int h = HitIndex(k, x, y);
    if (h < 0) return 0;
    k->pressed = h;
    k->btn[h].down = 1;
    return 1;
}

int Keypad_MouseUp(Keypad* k, int x, int y)
{
    int idx = k->pressed, id = -1;
    if (idx < 0) return -1;
    k->btn[idx].down = 0;
    k->pressed = -1;

    // only counts as a click if you let go on the same button. rules are rules
    if (HitIndex(k, x, y) == idx) id = k->btn[idx].id;
    return id;
}