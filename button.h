#pragma once
#include <windows.h>

typedef enum ButtonKind { BTN_DIGIT, BTN_OP, BTN_ACTION, BTN_FUNC, BTN_EQUALS } ButtonKind;

typedef struct Button {
    RECT           rc;
    const wchar_t* label;
    int            id;     // the char it means: '7', '+', 'C'. yes chars. its fine
    ButtonKind     kind;
    int            hot;    // mouse is hovering, it knows
    int            down;   // being pressed, it feels it
} Button;

void Button_Init(Button* b, int id, const wchar_t* label, ButtonKind kind);
void Button_Draw(const Button* b, HDC dc);
int  Button_HitTest(const Button* b, int x, int y);