#ifndef CONSOLE_H
#define CONSOLE_H

using namespace std;

enum alignment {
    LEFT, CENTER, RIGHT
};

enum consoleStyle {
    NORMAL, BOLD
};


// terminal mode
void nocursor();

void cursor();

void noecho();

void echo();

void raw();

void cooked();


// coordinates
void gotoxy(int x, int y);

void coutxy(const string &str, int x, int y, int align = LEFT);

void coutxy(char c, int x, int y, int align = LEFT);

//void coutxy(const std::string &str, int x, int y);

//void coutxy(char ch, int x, int y);

//void coutxy(string str, int x, int y);

//void coutxy(const std::string &str, int x, int y, int align);

void coutxy(int num, int x, int y);

//void coutxy(string str, int x, int y, int align);

void coutxy(int num, int x, int y, int align);


// drawings
void drawBox(const string &title, int x, int y, int w, int h, int align, const char *color, int style);

void drawBox(int x, int y, int w, int h);

void coutbar(int val, int min, int max, int x, int y);


// return
string tab(int len, int num);

string tab(int len);

string currentDateTime();

char char_case(int n, bool shift);

int intlen(double n);

int getch();


// breaks
void gameOver();

void sleep(int milisec);


// special keys
const int KEY_BACK = 8;
const int KEY_TAB = 9;
const int KEY_ENTER = 13;
const int KEY_ESC = 27;
const int KEY_SPACE = 32;
const int KEY_ENTER_ALT = 141;
const int KEY_SHIFT = 65505;
const int KEY_SHIFT_ALT = 65506;
const int KEY_CAPS = 65509;
const int KEY_LEFT = 65361;
const int KEY_UP = 65362;
const int KEY_RIGHT = 65363;
const int KEY_DOWN = 65364;

// characters
const string bl_1 = "░";
const string bl_2 = "▒";
const string bl_3 = "▓";
const string bl_4 = "█";
const string a_tl = "╭";
const string a_tr = "╮";
const string a_bl = "╰";
const string a_br = "╯";
const string ln_v = "│";
const string ln_h = "─";
const string ln_s = "├";
const string ON = "🮱 ";
const string OFF = "▒🮌";


// appearance
#define c_up      "\x1b[1A"
#define c_normal  "\033[0m"
#define c_bold    "\033[1m"
#define c_faint   "\033[2m"
#define c_italic  "\033[3m"
#define c_uline   "\033[4m"
#define c_blink   "\033[5m"
#define c_reverse "\033[7m"
#define c_dbuline "\033[21m"
#define c_dl_bold "\033[22m"

#define f_blk "\033[30m"
#define f_red "\033[31m"
#define f_grn "\033[32m"
#define f_ylw "\033[33m"
#define f_blu "\033[34m"
#define f_vlt "\033[35m"
#define f_azr "\033[36m"
#define f_wht "\033[37m"

#define b_blk "\033[40m"
#define b_red "\033[41m"
#define b_grn "\033[42m"
#define b_ylw "\033[43m"
#define b_blu "\033[44m"
#define b_vlt "\033[45m"
#define b_azr "\033[46m"
#define b_wht "\033[47m"

#define fa_blk "\033[90m"
#define fa_red "\033[91m"
#define fa_grn "\033[92m"
#define fa_ylw "\033[93m"
#define fa_blu "\033[94m"
#define fa_vlt "\033[95m"
#define fa_azr "\033[96m"
#define fa_wht "\033[97m"

#define ba_blk "\033[100m"
#define ba_red "\033[101m"
#define ba_grn "\033[102m"
#define ba_ylw "\033[103m"
#define ba_blu "\033[104m"
#define ba_vlt "\033[105m"
#define ba_azr "\033[106m"
#define ba_wht "\033[107m"


#endif // CONSOLE_H
