#include "app.h"
// btw this works on xp thanks to 141_xp 
// the entire entry point. it is very small and very proud
int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, PWSTR cmd, int show)
{
    App app;
    MSG msg;
    (void)prev; (void)cmd;

    if (!App_Create(&app, inst, show)) return 1;

    // pump messages until the window dies. forever is a strong word
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}