#include "app.h"
#include "theme.h"
#include "gfx.h"
#include "menu.h"
#include <windowsx.h>

#define APP_CLASS L"BadCalcWnd"

// recompute where everything lives. called on every resize
static void App_Layout(App* a)
{
    RECT rc, area;
    GetClientRect(a->hwnd, &rc);

    a->display.rc.left = UI_MARGIN;
    a->display.rc.top = UI_MARGIN;
    a->display.rc.right = rc.right - UI_MARGIN - UI_SHADOW;
    a->display.rc.bottom = UI_MARGIN + DISPLAY_H;

    area.left = UI_MARGIN;
    area.top = UI_MARGIN * 2 + DISPLAY_H;
    area.right = rc.right - UI_MARGIN - UI_SHADOW;     // leave room for the shadows or they get cut off. sad shadows
    area.bottom = rc.bottom - UI_MARGIN - UI_SHADOW;
    Keypad_Layout(&a->keypad, &area);
}

// draw everything into an offscreen bitmap, then blit once. no flicker, no tears
static void App_Paint(App* a)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(a->hwnd, &ps);
    RECT rc;
    GetClientRect(a->hwnd, &rc);

    // minimized window = 0 size = CreateCompatibleBitmap throws a fit
    if (rc.right > 0 && rc.bottom > 0) {
        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HGDIOBJ old = SelectObject(mem, bmp);

        Gfx_FillRect(mem, &rc, COL_BG);
        Display_Draw(&a->display, mem, a->fontDisplay);
        Keypad_Draw(&a->keypad, mem, a->fontButton);

        BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old);
        DeleteObject(bmp);
        DeleteDC(mem);
    }
    EndPaint(a->hwnd, &ps);
}

void App_OnButton(App* a, int id)
{
    switch (id) {
    case 'C': Display_Clear(&a->display); break;
    case 'B': Display_Backspace(&a->display); break;
    case '=': break;   // todo: evaluator + the bug engine. the fun part
    case '~': break;   // todo: negate. a number but spooky
    default:
        // sqr and sqrt are todo too. they just sit there looking smart
        if (id >= KEY_SPECIAL) break;

        // placeholder behavior: a lone 0 gets replaced by a digit, else we just append
        if (a->display.len == 1 && a->display.text[0] == L'0' && id >= '0' && id <= '9')
            Display_Set(&a->display, L"");
        Display_Append(&a->display, (wchar_t)id);
        break;
    }
}

static LRESULT CALLBACK App_WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    App* a = (App*)GetWindowLongPtrW(h, GWLP_USERDATA);

    // first message ever: stash our App pointer in the window so we can find it later
    if (m == WM_NCCREATE) {
        CREATESTRUCTW* cs = (CREATESTRUCTW*)l;
        a = (App*)cs->lpCreateParams;
        a->hwnd = h;
        SetWindowLongPtrW(h, GWLP_USERDATA, (LONG_PTR)a);
        return DefWindowProcW(h, m, w, l);
    }
    if (!a) return DefWindowProcW(h, m, w, l);

    switch (m) {
    case WM_SIZE:
        App_Layout(a);
        InvalidateRect(h, NULL, FALSE);
        return 0;

    case WM_ERASEBKGND:
        return 1;   // we paint everything ourselves, windows can keep its hands off

    case WM_PAINT:
        App_Paint(a);
        return 0;

    case WM_MOUSEMOVE:
        // windows wont tell us the mouse left unless we ask nicely. every time
        if (!a->tracking) {
            TRACKMOUSEEVENT t = { sizeof(t), TME_LEAVE, h, 0 };
            TrackMouseEvent(&t);
            a->tracking = 1;
        }
        if (Keypad_MouseMove(&a->keypad, GET_X_LPARAM(l), GET_Y_LPARAM(l)))
            InvalidateRect(h, NULL, FALSE);
        return 0;

    case WM_MOUSELEAVE:
        a->tracking = 0;
        if (Keypad_MouseLeave(&a->keypad))
            InvalidateRect(h, NULL, FALSE);
        return 0;

    case WM_LBUTTONDOWN:
        if (Keypad_MouseDown(&a->keypad, GET_X_LPARAM(l), GET_Y_LPARAM(l))) {
            SetCapture(h);   // keep getting mouse events even if you drag outside
            InvalidateRect(h, NULL, FALSE);
        }
        return 0;

    case WM_LBUTTONUP:
        if (GetCapture() == h) ReleaseCapture();
        {
            int id = Keypad_MouseUp(&a->keypad, GET_X_LPARAM(l), GET_Y_LPARAM(l));
            if (id >= 0) App_OnButton(a, id);
            InvalidateRect(h, NULL, FALSE);
        }
        return 0;

    //case WM_CHAR:
    //    // keyboard support, because clicking 21 buttons is for people with time
    //{
    //    wchar_t c = (wchar_t)w;
    //    int id = -1;
    //    if ((c >= L'0' && c <= L'9') || c == L'.' || c == L'+' || c == L'-' ||
    //        c == L'*' || c == L'/') id = c;
    //    else if (c == L'=' || c == L'\r') id = '=';
    //    else if (c == L'\b') id = 'B';
    //    else if (c == 27) id = 'C';   // escape
    //    if (id >= 0) { App_OnButton(a, id); InvalidateRect(h, NULL, FALSE); }
    //}

    // a little sweet treat to those who actually READ the code, congratulations, uncomment it and you have keyboard support

    return 0;

    case WM_COMMAND:
        // the one and only menu button lands here
        if (LOWORD(w) == IDM_CREDITS) Credits_Show(h);
        return 0;

    case WM_DESTROY:
        DeleteObject(a->fontDisplay);
        DeleteObject(a->fontButton);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

int App_Create(App* a, HINSTANCE inst, int show)
{
    WNDCLASSEXW wc;
    RECT rc = { 0, 0, 300, 450 };   // client size, the window will be bigger. borders and a menu exist
    // fixed size, no maximize button. calculators know their place
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    ZeroMemory(a, sizeof(*a));
    Display_Init(&a->display);
    Keypad_Init(&a->keypad);

    // negative height = pixel height. positive = something else entirely. thanks gdi
    a->fontDisplay = CreateFontW(-36, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, L"Consolas");
    a->fontButton = CreateFontW(-22, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");

    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = App_WndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.lpszClassName = APP_CLASS;
    if (!RegisterClassExW(&wc)) return 0;

    AdjustWindowRect(&rc, style, TRUE);   // TRUE = there is a menu bar eating some height
    CreateWindowExW(0, APP_CLASS, L"BadCalc", style, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top, NULL, Menu_Create(), inst, a);
    if (!a->hwnd) return 0;

    ShowWindow(a->hwnd, show);
    UpdateWindow(a->hwnd);
    return 1;
}