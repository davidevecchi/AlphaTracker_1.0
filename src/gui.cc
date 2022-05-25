#include "gui.h"
#include "console.h"

#include <iostream>

using namespace std;

void Gui::homeLoading() {
    int y = 3;
    system("clear");
    drawBox(15, y++, 28, 3);
    cout << c_bold << f_azr;
    coutxy("AlphaTracker", 29, y++, CENTER);
    cout << c_normal;
    drawBox(15, ++y, 28, 15);
    cout << c_faint;
    y += 2;
    for (int i = 0; i < 3; i++) {
        coutxy(SOURCES[i], 29, y + 3 * i, CENTER);
        coutxy("...", 29, y + 1 + 3 * i, CENTER);
    }
    y += 10;
    cout << c_normal << f_ylw;
    coutxy("Impostazioni", 29, y, CENTER);
    cout << c_normal;
    coutxy("Premere [H] per mostrare i suggerimenti", 29, 23, CENTER);
    cout << c_normal << c_bold << f_azr;
    coutxy("[H]", 17, 23);
    cout << c_normal;
    gotoxy(1, 1);
}


void Gui::homeUpdate(const bool isSourceOpen[], int idx, const string &val) {
    int y = 8;
    if (idx == 2 && isSourceOpen[idx])
        coutxy(tab(100), 1, 1);
    cout << c_normal << (isSourceOpen[idx] ? f_grn : c_faint);
    coutxy(SOURCES[idx], 29, y + 3 * idx, CENTER);
    cout << (isSourceOpen[idx] ? c_normal : c_faint);
    coutxy(val, 29 + (idx == 1) * (val.length() % 2), y + 1 + 3 * idx, CENTER);
    if (idx == 1)
        gotoxy(1, 1);
    cout << c_normal;
}


void Gui::homeSelect(const bool isSourceOpen[], int select) {
    int y = 8;
    for (int i = 0; i < N_SOURCES; i++) {
        cout << c_normal << (isSourceOpen[i] ? f_grn : c_faint);
        if (select == i) cout << c_reverse;
        coutxy(SOURCES[i], 29, y + 3 * i, CENTER);
    }
    y += 10;
    cout << c_normal << f_ylw;
    if (select == N_SOURCES) cout << c_reverse;
    coutxy("Impostazioni", 29, y, CENTER);
    cout << c_normal;
}


void Gui::settingsStable(map<string, vector<IniParam>> &iniList) {
    int y = 2, src = 0;
    system("clear");
    cout << c_bold << f_azr << c_uline;
    coutxy("IMPOSTAZIONI", 29, y++, CENTER);
    for (int i = 0; i < iniList["GENERALE"].size(); i++) {
        if (i == 0 || i == 1 || i == 3) {
            cout << c_normal << f_grn;
            coutxy(SOURCES[src++], 29, ++y, CENTER);
            cout << c_normal;
        }
        if (i != 4 && i != 6) y++;
        drawBox(iniList["GENERALE"][i].param,
                ((i != 4 && i != 6) ? 5 : 29), y,
                (i < 3 ? 47 : 23), 3, LEFT, f_ylw, NORMAL);
        if (i == 0 || i == 2) y += 3;
        if (i == 1 || i == 4) y += 2;
    }
    gotoxy(1, 1);
}


void Gui::settingsSelect(map<string, vector<IniParam>> &iniList, int slc) {  // FIXME segfault
    
    int y = 4;
    
    cout << iniList["GENERALE"][0].s;

    for (int i = 0; i < iniList["GENERALE"].size(); i++) {

        string key = iniList["GENERALE"][i].param;
        string val = iniList["GENERALE"][i].s;
        int len = val.length();

        cout << f_ylw;
        if (slc == i) cout << c_reverse;
        if (key != "Porta" && key != "Password") y++;
        else y--;

        gotoxy(key != "Porta" && key != "Password" ? 7 : 31, y);
        cout << ' ' << key << ' ' << c_normal; // name

        int dim = (i < 3) ? 41 : 17;
        if (len > dim)
            val = val.substr(len - dim, len);

        coutxy(len > dim ? "-" : " ",
               key != "Porta" && key != "Password" ? 7 : 31,
               ++y);
        cout << val;

        if (key != "Camera")
            cout << c_blink << (slc == i ? bl_4 : " ") << ' ' << c_normal;
        if (key == "Camera" || key == "File")
            y += 3;
        if (key == "Percorso" || key == "Porta")
            y += 1;

    }
    
    gotoxy(1, 1);
    
}


void Gui::guiStable(map<string, vector<IniParam>> &iniList) {
    system("clear");
    drawBox("Vista", 2, 2, 24, 11, CENTER, f_grn, BOLD);
    cout << f_ylw;
    int i = 0;
    for (const auto &p : iniList["PARAMETRI"]) {
        coutxy(p.param, 4, 4 + 2 * i++);
    }
    cout << c_normal;
    drawBox("Stato", 2, 13, 24, 9, CENTER, f_grn, BOLD);
    cout << f_ylw;
    coutxy("Conteggio", 5, 15);
    coutxy("FPS", 5, 17);
    coutxy("Tempo", 5, 19);
    cout << c_normal;
    drawBox("Eventi", 27, 2, 28, 20, CENTER, f_grn, BOLD);
    cout << f_azr;
    coutxy("ID  frame  len  dir life", 29, 3);
    cout << c_normal;
    coutxy("Premere [H] per mostrare i suggerimenti", 29, 23, CENTER);
    cout << c_normal << c_bold << f_azr;
    coutxy("[H]", 17, 23);
    cout << c_normal;
}


void Gui::instructions() {
    gotoxy(1, 1);
    cout << "\n " << c_bold << f_azr << c_uline
         << "ISTRUZIONI\n" << c_normal
         << "\n"
         << " L'interazione con il programma avviene unicamente da tastiera e solo quando\n"
         << " il focus e' sulla finestra con il video (quando la barra del titolo e' grigia);\n"
         << " se non si clicca all'esterno della finestra il programma procede in automatico\n"
         << "\n"
         << "\n" << c_bold << f_azr
         << " Comandi\n" << c_normal
         << "\n" << c_bold
         << "   Correzione della prospettiva\n" << c_normal
         << "     - [ESC]    termina l'esecuzione del programma\n"
         << "     - [ENTER]  seleziona i vertici attualmente visualizzati\n"
         << "\n" << c_bold
         << "   Tracciamento delle particelle\n" << c_normal
         << "     - [ENTER][ESC][SPACE]  terminano l'esecuzione del programma\n"
         << "     - [8][9]  scorrimento orizzontale fra i parametri\n"
         << "     - [5][6]  modificano il parametro selezionato\n"
         << "     - [4][7]  scorrimento verticale fra gli oggetti da mostrare\n"
         << "     - [1]     attiva/disattiva l'oggetto selezionato\n"
         << "     - [0]     ripristina i valori predefiniti\n"
         << "     - [+]     pausa/play\n\n";
}
