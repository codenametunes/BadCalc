#pragma once
#include <windows.h>
#include "display.h"
#include "keypad.h"

typedef struct App {
    HWND    hwnd;
    HFONT   fontDisplay;
    HFONT   fontButton;
    Display display;
    Keypad  keypad;
    int     tracking;   // 1 = we asked windows to tell us when the mouse leaves
} App;

int  App_Create(App* a, HINSTANCE inst, int show);

// every click and keypress ends up here. the calculator brain goes here later
void App_OnButton(App* a, int id);