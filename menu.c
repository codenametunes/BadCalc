#include "menu.h"

// the top bar has one button. one. it says About. We Are So Back.
HMENU Menu_Create(void)
{
    HMENU bar = CreateMenu();
    AppendMenuW(bar, MF_STRING, IDM_CREDITS, L"About");
    return bar;
}