#pragma once
#include <windows.h>

// menu command ids. there is exactly one. we are minimalists now
enum {
    IDM_CREDITS = 1001
};

// builds the top bar
HMENU Menu_Create(void);