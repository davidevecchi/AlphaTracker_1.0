#ifndef SETTINGS_H
#define SETTINGS_H

#include "gui.h"

using namespace std;

class Settings {
    
    string path;
    
    static bool isOnline(const string &address, int port);

public:
    
    map<string, vector<IniParam>> iniList;
    map<string, IniParam> ini;
    
    // Sources
    bool isSourceOpen[N_SOURCES + 1];  //!< open Sources (and settings)
    string ipcLink;                    //!<
    
    explicit Settings(const string &pwd);
    
    void parse();
    
    void save();
    
    void checkSoures();
    
    bool connectIPC();
    
    int selectSource();
    
    void editSettings();
    
    cv::Size resolution(const string &param);
};



#endif // SETTINGS_H