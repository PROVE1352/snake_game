// curses_compat.h
// OS에 따라 curses 헤더와 sleep 함수를 알맞게 골라주는 호환 레이어.
//   - Windows(MinGW/MSYS2) : PDCurses + Windows Sleep
//   - macOS / Linux        : ncurses + POSIX usleep

#ifndef CURSES_COMPAT_H
#define CURSES_COMPAT_H

#ifdef _WIN32               //전처리기. Windows 환경에서 진행.
    #include <pdcurses.h>   // curses.h -> pdcurses.h로 변경(윈도우 환경 실행오류 해결)
    #include <windows.h>
    // PDCurses 환경에서는 usleep이 없으므로 Sleep(밀리초)으로 매핑
    inline void sleep_usec(const int usec) {
        Sleep(usec / 1000);
    }
#else
    #include <ncurses.h>    // macOS / Linux
    #include <unistd.h>
    inline void sleep_usec(const int usec) {
        usleep(usec);
    }
#endif

#endif
