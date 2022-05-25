#include "settings.h"
#include "console.h"
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <SFML/Network.hpp>

using namespace std;

Settings::Settings(const string &pwd) {
    path = pwd + "/settings.ini";
    parse();
}

void Settings::parse() {
    ifstream in(path);
    string section, param, strVal;
    int intVal;
    char c, bfr[256];
    while (!in.eof()) {
        c = in.peek();
        switch (c) {
            case '[':
                in.get();
                in.getline(bfr, 256, ']');
                section = bfr;
                in.getline(bfr, 256);
                break;
            case ' ':
            case ';':
            case '\n':
                in.getline(bfr, 256);
                break;
            default:
                in.getline(bfr, 256, '=');
                param = bfr;
                while (!param.empty() && param.back() == ' ') {
                    param.pop_back();
                }
                in.getline(bfr, 256);
                strVal = bfr;
                while (!strVal.empty() && strVal.front() == ' ') {
                    strVal.erase(0, 1);
                }
                while (!strVal.empty() && strVal.back() == ' ') {
                    strVal.pop_back();
                }
                try {
                    intVal = stoi(strVal);
                } catch (std::exception const &e) {
                    intVal = -1;
                }
                if (!param.empty() && !strVal.empty()) {
                    ini[(string) section + "." + (string) param] = {param, strVal, intVal};
                    iniList[section].push_back({param, strVal, intVal});
                }
                break;
        }
    }
    ipcLink = "http://";
    if (!ini["GENERALE.Utente"].s.empty()) {
        ipcLink.append(ini["GENERALE.Utente"].s + ":" +
                       ini["GENERALE.Password"].s + "@");
    }
    ipcLink.append(ini["GENERALE.Indirizzo"].s + ":" +
                   ini["GENERALE.Porta"].s + "/video?x.mjpeg");
}

void Settings::save() {
    // TODO
}

bool Settings::connectIPC() {
    mutex m;
    condition_variable cv;
    bool ret;
    
    thread t([&cv, &ret, this](const string &adr, int prt) {
        ret = isOnline(ini["GENERALE.Indirizzo"].s, ini["GENERALE.Porta"].i);
        cv.notify_one();
    }, ini["GENERALE.Indirizzo"].s, ini["GENERALE.Porta"].i);
    
    t.detach();
    
    unique_lock<mutex> l(m);
    if (cv.wait_for(l, chrono::duration<int>(1)) == cv_status::timeout) {
        ret = false;
    }
    
    return ret;
}

bool Settings::isOnline(const string &address, int port) {
    sf::TcpSocket socket;
    bool open = (socket.connect(sf::IpAddress(address), port) == sf::Socket::Done);
    socket.disconnect();
    return open;
}

void Settings::checkSoures() {
    isSourceOpen[CAM] = cv::VideoCapture(0).isOpened();  // FIXME?
    Gui::homeUpdate(isSourceOpen, CAM, ini["GENERALE.Camera"].s);
    isSourceOpen[VID] = cv::VideoCapture(ini["GENERALE.Percorso"].s + ini["GENERALE.File"].s).isOpened();
    Gui::homeUpdate(isSourceOpen, VID, ini["GENERALE.File"].s);
    isSourceOpen[IPC] = connectIPC();
    Gui::homeUpdate(isSourceOpen, IPC, ini["GENERALE.Indirizzo"].s);
    isSourceOpen[N_SOURCES] = true;  // make "Impostazioni" selectable
}

int Settings::selectSource() {
    bool restart = true;
    int select, key;
    while (restart) {
        restart = false;
        Gui::homeLoading();
        checkSoures();
        select = 0;
        key = -1;
        while (key != KEY_ENTER && key != KEY_ENTER_ALT) {
            Gui::homeSelect(isSourceOpen, select);
            key = cv::waitKeyEx(0);
            switch (key) {
                case KEY_DOWN:
                    while (!isSourceOpen[select = (select + 1) % (N_SOURCES + 1)]);
                    break;
                case KEY_UP:
                    while (!isSourceOpen[(select == 0) ? select = N_SOURCES : --select]);
                    break;
                case KEY_ESC:
                    exit(0);
            }
            
        }
        if (select == N_SOURCES) {
            editSettings();
            restart = true;
        }
    }
    system("clear");
    return select;
}

void Settings::editSettings() {
    Gui::settingsStable(iniList);
    int select = 0, key = -1;
    bool saved = false, shift = false, caps = false, edit = true;
    while (key != KEY_ENTER && key != KEY_ENTER_ALT && key != KEY_ESC) {
        Gui::settingsSelect(iniList, select);
        if (!edit) {
            edit = true;
        } else {
            key = cv::waitKeyEx(0);
        }
        switch (key) {
            case KEY_DOWN:
                select = (select + 1 + (select >= 3 && select <= 5)) % iniList["GENERALE"].size();
                break;
            case KEY_UP:
                select = (select == 0) ? 5 : (select - 1 - (select >= 4 && select <= 6));
                break;
            case KEY_RIGHT:
            case KEY_TAB:
                select = (select + 1) % iniList["GENERALE"].size();
                break;
            case KEY_LEFT:
                select = (select == 0) ? (iniList["GENERALE"].size() - 1) : (select - 1);
                break;
            default:
                while (edit && iniList["GENERALE"][select].param != "Camera") {
                    if (key == KEY_ENTER
                        || key == KEY_ESC
                        || key == KEY_DOWN
                        || key == KEY_LEFT
                        || key == KEY_RIGHT
                        || key == KEY_UP) {
                        edit = false;
                        saved = (key == KEY_ENTER);
                    } else {
                        string val = iniList["GENERALE"][select].s;
                        int len = val.length();
                        if (key == KEY_BACK) {
                            iniList["GENERALE"][select].s = val.substr(0, len - 1);
                        } else if (key == KEY_SHIFT || key == KEY_SHIFT_ALT) {
                            shift = !shift;
                        } else if (key == KEY_CAPS) {
                            caps = !caps;
                            shift = caps;
                        } else {
                            iniList["GENERALE"][select].s = val + char_case(key, shift);
                            shift = caps;
                        }
                        Gui::settingsSelect(iniList, select);
                        // coutxy (key, 1, 1);
                        key = cv::waitKeyEx(0);
                        // coutxy (tab(8), 1, 1);
                    }
                }
                break;
        }
    }
    if (saved) {
        save();
    }
    parse();
}

cv::Size Settings::resolution(const string &param) {
    string value = ini["RISOLUZIONI." + param].s;
    int x = value.find('x');
    int w = 853, h = 480;
    try {
        w = stoi(value.substr(0, x));
        h = stoi(value.substr(x + 1, value.length()));
    } catch (std::exception const &e) {}
    
    return {w, h};
}
