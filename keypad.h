#pragma once
#include <windows.h>
#include "button.h"

#define KEYPAD_COLS 4
#define KEYPAD_ROWS 6
#define KEYPAD_MAX  21

// ids below 256 are plain chars ('7', '+'). the fancy ones live above that
enum {
    KEY_SPECIAL = 256,
    KEY_SQR,     // x squared
    KEY_SQRT     // square root
};

typedef struct Keypad {
    Button btn[KEYPAD_MAX];
    int    count;
    int    hot;       // hovered button index, -1 = nothing
    int    pressed;   // button held down with the mouse, -1 = nothing
} Keypad;

void Keypad_Init(Keypad* k);
void Keypad_Layout(Keypad* k, const RECT* area);
void Keypad_Draw(const Keypad* k, HDC dc, HFONT font);

int  Keypad_MouseMove(Keypad* k, int x, int y);
int  Keypad_MouseLeave(Keypad* k);
int  Keypad_MouseDown(Keypad* k, int x, int y);

int  Keypad_MouseUp(Keypad* k, int x, int y);