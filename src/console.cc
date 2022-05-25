
#include <iostream>
#include <cmath>
#include <ctime>
#include "console.h"

using namespace std;

char char_case(int n, bool shift) {
    char c = n;
    if (c >= 'a' && c <= 'z' && shift)
        c -= 'a' - 'A';
    return c;
}


int getch() {
    raw();
    int ch = cin.get();
    cooked();
    return ch;
}


void nocursor() {
    printf("\e[?25l");
}

void cursor() {
    system("tput reset");
    printf("\e[?25h");
}


void raw() {
    system("/bin/stty raw");
}

void cooked() {
    system("/bin/stty cooked");
}


void noecho() {
    system("/usr/bin/stty -echo");
}

void echo() {
    system("/usr/bin/stty echo");
}


void gotoxy(int x, int y) {
    printf("\033[%d;%df", y, x);
    fflush(stdout);
}


void coutxy(const string& str, int x, int y, int align) {
    gotoxy(x - (double) align / 2.0 * str.length(), y);
    cout << str;
    fflush(stdout);
}

void coutxy(char c, int x, int y, int align) {
    gotoxy(x - (double) align / 2.0, y);
    cout << c;
    fflush(stdout);
}
//
//void coutxy(char ch, int x, int y) {
//    gotoxy(x, y);
//    cout << ch;
//    fflush(stdout);
//}
//
//void coutxy(const char *str, int x, int y) {
//    gotoxy(x, y);
//    cout << str;
//    fflush(stdout);
//}
//
//void coutxy(const std::string& str, int x, int y) {
//    gotoxy(x, y);
//    cout << str;
//    fflush(stdout);
//}
//
void coutxy(int num, int x, int y) {
    gotoxy(x, y);
    cout << num;
    fflush(stdout);
}
//
//void coutxy(const char *str, int x, int y, int align) {
//    coutxy(str, x - (double) align / 2.0 * strlen(str), y);
//}
//
//void coutxy(const std::string& str, int x, int y, int align) {
//    coutxy(str, x - (double) align / 2.0 * str.length(), y);
//}
//
void coutxy(int num, int x, int y, int align) {
    coutxy(num, x - (double) align / 2.0 * intlen(num), y);
}


void gameOver() {
    gotoxy(2, 25);
    cout << c_blink << "Premere un tasto per uscire " << c_normal;
    getch();
//    coutxy(tab(64), 2, 25);
    gotoxy(1, 25);
}


void coutbar(int val, int min, int max, int x, int y) {
    gotoxy(x, y);
    for (int i = min; i < val; i++)
        cout << bl_4;
    for (int i = val; i < max; i++)
        cout << bl_1;
}


void drawBox(const string& title, int x, int y, int w, int h, int align, const char *color, int style) {
    int len = w - title.length() - 4;
    int left, right;

    switch (align) {
        case LEFT:
            left = 1;
            break;
        case CENTER:
            left = len / 2;
            break;
        case RIGHT:
            left = len - 1;
            break;
    }
    right = len - left;

    coutxy(a_tl, x, y);
    for (int i = 0; i < left; i++)
        cout << ln_h;
    if (style == BOLD) cout << c_bold;
    cout << ' ' << color << title << c_normal << ' ';
    for (int i = 0; i < right; i++)
        cout << ln_h;
    cout << a_tr;

    for (int i = 1; i < h - 1; i++) {
        coutxy(ln_v, x, y + i);
        coutxy(ln_v, x + w - 1, y + i);
    }

    coutxy(a_bl, x, y + h - 1);
    for (int i = 0; i < w - 2; i++)
        cout << ln_h;
    cout << a_br;
}


void drawBox(int x, int y, int w, int h) {
    coutxy(a_tl, x, y);
    for (int i = 0; i < w - 2; i++)
        cout << ln_h;
    cout << a_tr;

    for (int i = 1; i < h - 1; i++) {
        coutxy(ln_v, x, y + i);
        coutxy(ln_v, x + w - 1, y + i);
    }

    coutxy(a_bl, x, y + h - 1);
    for (int i = 0; i < w - 2; i++)
        cout << ln_h;
    cout << a_br;
}


string tab(int len, int num) {
    string tab;
    for (int i = 0; i < len - num; i++)
        tab += ' ';
    return tab;
}


string tab(int len) {
    string tab;
    for (int i = 0; i < len; i++)
        tab += ' ';
    return tab;
}


string currentDateTime() {
    time_t now = time(nullptr);
    struct tm tstruct;
    char *buf = new char[4 + 2 + 2 + 1 + 6 + 1];
    tstruct = *localtime(&now);
    strftime(buf, (4 + 2 + 2 + 1 + 6 + 1), "%Y%m%d_%H%M%S", &tstruct);
    return buf;
}


int intlen(double n) {
    return (n != 0) ? (log10(abs(n)) + (n < 0) + 1) : 1;
}


void sleep(int milisec) {
    struct timespec req = {0};
    time_t sec = (int) (milisec / 1000);
    milisec -= (sec * 1000);
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    while (nanosleep(&req, &req) == -1)
        continue;
}   

 
