#include <windows.h>

void Credits_Show(HWND owner)
{
    MessageBoxW(owner, L"horrid calculator\n", L"BadCalc by @codenametunes", MB_OK | MB_ICONINFORMATION);
}