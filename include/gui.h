#ifndef GUI_H
#define GUI_H

#include "common.h"

using namespace std;

class Gui {

public:
  
    static void homeLoading();

    static void homeUpdate(const bool isSourceOpen[], int idx, const string& val);

    static void homeSelect(const bool isSourceOpen[], int select);

    static void settingsStable(map<string, vector<IniParam>> &iniList);

    static void settingsSelect(map<string, vector<IniParam>> &iniList, int slc);

    static void guiStable(map<string, vector<IniParam>> &iniList);

    static void instructions();

};


#endif // GUI_H