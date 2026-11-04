#define _CRT_SECURE_NO_WARNINGS   // microsoft wants _s everything. we have a 12 char cap and a dream
#include <windows.h>
#include <windowsx.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include "theme.h"

// btw this works on xp thanks to 141_xp
#define IDM_ABOUT 1001
#define COLS  4
#define ROWS  6
#define CAP   12   // max typed chars. the display font is big and the display is not
#define CHAOS 15   // after this many calculations the floating point goes feral

enum { SQR = 256, SQRT };   // ids below 256 are plain chars. the fancy ones live above

typedef struct { int id; const wchar_t* l; COLORREF c; int col, row, span; RECT rc; } Key;

// the whole keypad in one table. if a button is wrong its this tables fault, not mine
static Key K[] = {
    { 'C', L"C",       COL_ACTION, 0, 0, 1 }, { 'B', L"<-",      COL_ACTION, 1, 0, 1 },
    { SQR, L"x\u00B2", COL_FUNC,   2, 0, 1 }, { SQRT, L"\u221A", COL_FUNC,   3, 0, 1 },
    { '7', L"7",       COL_DIGIT,  0, 1, 1 }, { '8', L"8",       COL_DIGIT,  1, 1, 1 },
    { '9', L"9",       COL_DIGIT,  2, 1, 1 }, { '/', L"\u00F7",  COL_OP,     3, 1, 1 },
    { '4', L"4",       COL_DIGIT,  0, 2, 1 }, { '5', L"5",       COL_DIGIT,  1, 2, 1 },
    { '6', L"6",       COL_DIGIT,  2, 2, 1 }, { '*', L"\u00D7",  COL_OP,     3, 2, 1 },
    { '1', L"1",       COL_DIGIT,  0, 3, 1 }, { '2', L"2",       COL_DIGIT,  1, 3, 1 },
    { '3', L"3",       COL_DIGIT,  2, 3, 1 }, { '-', L"\u2212",  COL_OP,     3, 3, 1 },
    { '~', L"\u00B1",  COL_FUNC,   0, 4, 1 }, { '0', L"0",       COL_DIGIT,  1, 4, 1 },
    { '.', L".",       COL_DIGIT,  2, 4, 1 }, { '+', L"+",       COL_OP,     3, 4, 1 },
    { '=', L"=",       COL_EQUALS, 0, 5, 4 },   // equals eats the whole bottom row. it thinks it is better than you
};
#define NK ((int)(sizeof(K) / sizeof(K[0])))

static HFONT   fD, fS, fE, fB;   // display, display-but-smaller, expression line, buttons
static RECT    disp;
static int     hot = -1, pressed = -1;   // hovered / mouse-held button index, -1 = nothing

// calculator brain. globals, because there is exactly one calculator and it knows it
static wchar_t txt[32] = L"0", expr[96];   // what you see big / the little "5 + 3 =" line
static double  acc, rep;                   // running total / the operand that = keeps reusing
static int     op, fresh, eq, wait;        // pending op / next digit starts over / just hit = / just hit an op
static int     calcs, presses;             // lifetime counters. clear does NOT forgive you
static int     sulk;                       // 1 = display is showing an excuse, the real number is parked in keep
static wchar_t keep[32];

// the calculator has opinions about work
static const wchar_t* QUIPS[] = { L"i don't feel like it", L"ask me later", L"no", L"maybe tomorrow", L"busy rn", L"google it", L"eehhhh", L"not my job" };

static wchar_t Sym(int o)
{
    return o == '*' ? L'\u00D7' : o == '/' ? L'\u00F7' : o == '-' ? L'\u2212' : (wchar_t)o;
}

static void Fmt(wchar_t* o, double v)
{
    if (v - v != 0) wcscpy(o, L"nope");   // inf and nan both make v-v nan. one check, zero shame
    else _snwprintf(o, 32, calcs >= CHAOS ? L"%.17g" : L"%.8g", v + 0.0);   // + 0.0 turns -0 into 0. yes really
}

static void Show(double v) { Fmt(txt, v); }

// chaos mode: a few ulps of random garbage on every result. the digits you did not ask for
static double Noise(double v)
{
    if (calcs < CHAOS) return v;
    return v * (1 + (rand() % 7 - 3) * DBL_EPSILON);
}

// every bug lives in here (the other two are in Press). you can stop reading now
static double Calc(double a, int o, double b)
{
    double r;
    calcs++;
    switch (o) {
    case '+': r = a + b; break;
    case '-': r = a - b; break;
    case '*': r = a * b; break;
    default:  r = a / b;
    }
    if (o == '*' && a == 6 && b == 9) r = 42;   // the answer. obviously
    if (o == '-' && calcs % 4 == 0) r = fabs(r);   // every 4th calculation, subtraction forgets negatives exist
    return Noise(r);
}

// every click ends up here
static void Press(int id)
{
    double v;
    wchar_t a[32], b[32];
    int n, w;

    if (sulk) { wcscpy(txt, keep); sulk = 0; }   // excuse over, the number comes back from wherever it hid
    if ((id >= SQR || id == '*' || wcschr(L"=+-/~", id)) && rand() % 9 == 0) {   // 1 in 9 real work requests gets turned down
        wcscpy(keep, txt); wcscpy(txt, QUIPS[rand() % (sizeof(QUIPS) / sizeof(QUIPS[0]))]); sulk = 1;
        return;
    }
    v = _wtof(txt);
    w = wait;
    wait = 0;

    if (id == 'C') { wcscpy(txt, L"0"); expr[0] = 0; acc = 0; op = eq = fresh = 0; }
    else if (id == 'B') {
        n = (int)wcslen(txt);
        if (!fresh) { if (n <= 1 || (n == 2 && txt[0] == L'-')) wcscpy(txt, L"0"); else txt[n - 1] = 0; }
    }
    else if (id == '~') { if (v != 0) Show(-v); }
    else if (id == SQR || id == SQRT) { Show(Noise(id == SQR ? v * v : sqrt(v))); fresh = 1; }
    else if (id == '=') {
        if (op) {
            if (!eq) rep = v; else rep *= 1.01;   // drift: every extra = nudges the operand 1%. it is not a bug, it is character
            Fmt(a, acc); Fmt(b, rep);
            _snwprintf(expr, 96, L"%s %c %s =", a, Sym(op), b);
            acc = Calc(acc, op, rep); Show(acc); fresh = eq = 1;
        }
    }
    else if (id == '+' || id == '-' || id == '*' || id == '/') {
        if (op && !eq && !w) { acc = Calc(acc, op, v); Show(acc); }   // chained: 1+2+ shows 3
        else acc = v;
        op = id; fresh = wait = 1; eq = 0;
        Fmt(a, acc); _snwprintf(expr, 96, L"%s %c", a, Sym(op));
    }
    else {   // digits and the dot
        if (id != '.' && ++presses % 13 == 0) id = '0' + (id - '0' + 1) % 10;   // every 13th digit is unlucky and types its neighbor
        if (eq) op = eq = expr[0] = 0;   // typing after = starts a whole new sum
        if (fresh) { fresh = 0; txt[0] = 0; if (id == '.') wcscpy(txt, L"0"); }
        if (!wcscmp(txt, L"0") && id != '.') txt[0] = 0;   // a lone 0 gets replaced
        n = (int)wcslen(txt);
        if (n >= CAP || (id == '.' && wcschr(txt, L'.'))) return;
        txt[n] = (wchar_t)id; txt[n + 1] = 0;
    }
    if (eq && (id == '~' || id >= SQR)) acc = _wtof(txt);   // unary after = edits the result, not the old total
}

// divide the window into a grid. its just division, we can do this
static void Layout(int w, int h)
{
    int i, y0 = UI_MARGIN * 2 + DISPLAY_H;
    int cw = (w - UI_MARGIN * 2 - UI_SHADOW - UI_GAP * (COLS - 1)) / COLS;   // leave room for the shadows. sad shadows
    int ch = (h - y0 - UI_MARGIN - UI_SHADOW - UI_GAP * (ROWS - 1)) / ROWS;
    SetRect(&disp, UI_MARGIN, UI_MARGIN, w - UI_MARGIN - UI_SHADOW, UI_MARGIN + DISPLAY_H);
    if (cw < 0) cw = 0;   // tiny window = negative sizes = bad time
    if (ch < 0) ch = 0;
    for (i = 0; i < NK; i++) {
        K[i].rc.left = UI_MARGIN + K[i].col * (cw + UI_GAP);
        K[i].rc.right = K[i].rc.left + K[i].span * cw + (K[i].span - 1) * UI_GAP;
        K[i].rc.top = y0 + K[i].row * (ch + UI_GAP);
        K[i].rc.bottom = K[i].rc.top + ch;
    }
}

// linear search, there are 21 buttons, relax
static int Hit(int x, int y)
{
    POINT p; int i;
    p.x = x; p.y = y;
    for (i = 0; i < NK; i++) if (PtInRect(&K[i].rc, p)) return i;
    return -1;
}

// draw into an offscreen bitmap, blit once. no flicker, no tears
static void Paint(HWND h)
{
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(h, &ps), m;
    HBITMAP b; HGDIOBJ o;
    RECT rc, r, re; int i;
    GetClientRect(h, &rc);

    if (rc.right > 0 && rc.bottom > 0) {   // minimized = 0 size = CreateCompatibleBitmap throws a fit
        m = CreateCompatibleDC(dc);
        b = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
        o = SelectObject(m, b);
        Fill(m, &rc, COL_BG);
        SetBkMode(m, TRANSPARENT);

        r = Box(m, disp, COL_DISP_BG, 0);
        r.left += 12; r.right -= 12;
        re = r; re.top += 4;
        SelectObject(m, fE);
        SetTextColor(m, COL_FUNC);
        DrawTextW(m, expr, -1, &re, DT_RIGHT | DT_TOP | DT_SINGLELINE);
        SelectObject(m, wcslen(txt) > CAP ? fS : fD);   // long chaos numbers and long excuses get the small font
        SetTextColor(m, COL_DISP_TEXT);
        DrawTextW(m, txt, -1, &r, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

        SelectObject(m, fB);
        SetTextColor(m, COL_INK);
        for (i = 0; i < NK; i++) {
            r = Box(m, K[i].rc, K[i].c, i == pressed && i == hot);
            DrawTextW(m, K[i].l, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        BitBlt(dc, 0, 0, rc.right, rc.bottom, m, 0, 0, SRCCOPY);
        SelectObject(m, o); DeleteObject(b); DeleteDC(m);
    }
    EndPaint(h, &ps);
}

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    int i;
    switch (m) {
    case WM_SIZE: Layout(LOWORD(l), HIWORD(l)); InvalidateRect(h, NULL, FALSE); return 0;
    case WM_ERASEBKGND: return 1;   // we paint everything ourselves, windows can keep its hands off
    case WM_PAINT: Paint(h); return 0;

    case WM_MOUSEMOVE:
        i = Hit(GET_X_LPARAM(l), GET_Y_LPARAM(l));
        if (i != hot) { hot = i; InvalidateRect(h, NULL, FALSE); }
        return 0;

    case WM_LBUTTONDOWN:
        pressed = hot = Hit(GET_X_LPARAM(l), GET_Y_LPARAM(l));
        if (pressed >= 0) { SetCapture(h); InvalidateRect(h, NULL, FALSE); }   // capture = keep mouse events when you drag outside
        return 0;

    case WM_LBUTTONUP:
        if (pressed >= 0) {
            ReleaseCapture();
            if (Hit(GET_X_LPARAM(l), GET_Y_LPARAM(l)) == pressed) Press(K[pressed].id);   // only a click if you let go on the same button
            pressed = -1;
            InvalidateRect(h, NULL, FALSE);
        }
        return 0;

        //case WM_CHAR:
        //    i = w == '\r' ? '=' : w == '\b' ? 'B' : w == 27 ? 'C' : (int)w;   // enter, backspace, escape
        //    if (i && wcschr(L"0123456789.+*-/=BC", i)) Press(i), InvalidateRect(h, NULL, FALSE);
        //    return 0;
        
        // a little sweet treat to those who actually READ the code, congratulations, uncomment it and you have keyboard support

    case WM_COMMAND:
        if (LOWORD(w) == IDM_ABOUT) MessageBoxW(h, L"horrid calculator\n", L"BadCalc by @codenametunes", MB_OK | MB_ICONINFORMATION);
        return 0;

    case WM_DESTROY:
        DeleteObject(fD); DeleteObject(fS); DeleteObject(fE); DeleteObject(fB);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

// negative height = pixel height. positive = something else entirely. thanks gdi
static HFONT Font(int px, DWORD pitch, const wchar_t* face)
{
    return CreateFontW(-px, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, pitch, face);
}

// the entire entry point. it is no longer very small, but it is still proud
int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show)
{
    WNDCLASSW wc;
    MSG msg;
    HWND hw;
    RECT rc = { 0, 0, 300, 450 };   // client size, the window will be bigger. borders and a menu exist
    DWORD st = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;   // fixed size. calculators know their place
    HMENU bar = CreateMenu();
    (void)prev; (void)cmd;

    srand(GetTickCount());   // chaos should at least be different every time
    fD = Font(36, FIXED_PITCH, L"Consolas");
    fS = Font(20, FIXED_PITCH, L"Consolas");
    fE = Font(14, FIXED_PITCH, L"Consolas");
    fB = Font(22, DEFAULT_PITCH, L"Segoe UI");
    AppendMenuW(bar, MF_STRING, IDM_ABOUT, L"About");   // the top bar has one button. we are so back

    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.lpszClassName = L"BadCalcWnd";
    if (!RegisterClassW(&wc)) return 1;

    AdjustWindowRect(&rc, st, TRUE);   // TRUE = a menu bar is eating some height
    hw = CreateWindowExW(0, wc.lpszClassName, L"BadCalc", st, CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top, NULL, bar, inst, NULL);
    if (!hw) return 1;
    ShowWindow(hw, show);

    // pump messages until the window dies. forever is a strong word
    while (GetMessageW(&msg, NULL, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessageW(&msg); }
    return (int)msg.wParam;
}