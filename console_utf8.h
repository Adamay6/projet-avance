#ifndef CONSOLE_UTF8_H
#define CONSOLE_UTF8_H

#ifdef _WIN32
#include <windows.h>
#endif

static inline void initialiser_console_utf8(void)
{
#ifdef _WIN32
    /* Les messages sont en UTF-8, y compris avec un lancement direct du .exe. */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

#endif
