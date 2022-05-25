#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <cmath>
#include "functions.h"
#include "console.h"

using namespace std;

cv::VideoCapture getCapture(Settings &st, int &totFrame, int &totTime, const int &source, Size RES[]) {
    VideoCapture cap;
    switch (source) {
        case CAM:
            cap = VideoCapture(0);  // FIXME?
            break;
        case VID:
            cap = VideoCapture(st.ini["GENERALE.Percorso"].s + st.ini["GENERALE.File"].s);
            totFrame = cap.get(CAP_PROP_FRAME_COUNT);
            totTime = totFrame / cap.get(CAP_PROP_FPS);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT, RES[INPUT].height);
            cap.set(cv::CAP_PROP_FRAME_WIDTH, RES[INPUT].width);
            break;
        case IPC:
            cap = VideoCapture(st.ipcLink);
            break;
    }
    
    // ERROR TODO
    if (!cap.isOpened()) {
        cap.release();
        streambuf *cerrSbuf = cerr.rdbuf();
        ofstream ferr("/dev/null");
        cerr.rdbuf(ferr.rdbuf());
        cerr.rdbuf(cerrSbuf);
        cerr << f_red;
        switch (source) {
            case CAM:
                cerr << "\n Impossibile aprire la fotocamera " << st.ini["GENERALE.Camera"].s << endl;
                cap = VideoCapture(0);
                if (cap.isOpened()) {
                    cerr << f_grn << "\n Fotocamera 0 aperta correttamente\n\n";
                } else {
                    cerr << f_red << "\n Impossibile aprire la fotocamera 0\n" << f_ylw
                         << "\n Verificare che:\n"
                         << "  - il dispositivo sia funzionante\n"
                         << "  - il dispositivo sia collegato correttamente";
                }
                break;
            case VID:
                cerr << " Impossibile aprire il file video\n" << f_ylw
                     << "\n Verificare che:\n"
                     << "  - il percorso \"" << st.ini["GENERALE.Percorso"].s << st.ini["GENERALE.File"].s
                     << "\" sia corretto\n"
                     << "  - il file sia leggibile";
                break;
            case IPC:
                cerr << " Impossibile raggiungere IP camera\n"
                     << f_ylw << "\n Verificare che:\n"
                     << "  - il dispositivo sia attivo\n"
                     << "  - il dispositivo sia connesso alla stessa rete del Raspberry Pi\n"
                     << "  - l'indirizzo \"" << st.ipcLink << "\" sia corretto";
                break;
        }
        
        cerr << c_normal;
        if (!cap.isOpened()) {
            gameOver();
            quit(source);
        }
    }
    return cap;
}

void window(const String &name, const Size &size, const Point &pos, int flags) {
    namedWindow(name, flags);
    resizeWindow(name, size.width, size.height);
    moveWindow(name, pos.x, pos.y);
}


cv::Mat findPerspective(Settings &st, cv::VideoCapture cap, int SOURCE, Size RES[]) {
    // Perspective loop
    Mat source, cropped, dst, transmtx;
    bool found = false;
    int key;
    for (int f = 1; ((key = waitKey(1 + (SOURCE == VID) * st.ini["VARIABILI.DELAY"].i)) != KEY_ENTER
                     && key != KEY_ENTER_ALT) || !found; f++) {
        
        if (key == KEY_ESC) {
            cap.release();
            quit(3);
        }
        
        cap.read(source);

//        // ERROR TODO
//        if (source.empty()) {
//            destroyAllWindows();
//            cap.release();
//            system("clear");
//            cerr.rdbuf(cerrSbuf);
//            cerr << f_red << endl;
//            switch (SOURCE) {
//                case CAM:
//                    cerr << " Impossibile comunicare con la fotocamera " << st.ini["GENERALE.Camera"].s << endl
//                         << f_ylw << "\n Verificare che:\n"
//                         << "  - il dispositivo sia funzionante\n"
//                         << "  - il dispositivo sia collegato correttamente";
//                    break;
//                case VID:
//                    if (f - 1 == totFrame) {
//                        cerr << f_ylw << " Video finito al frame " << f - 1;
//                    } else {
//                        cerr << " Impossibile riprodurre il video, file danneggiato al frame " << f;
//                    }
//                    break;
//                case IPC:
//                    cerr << " Impossibile comunicare con IP camera\n"
//                         << f_ylw << "\n Verificare che:\n"
//                         << "  - il dispositivo sia attivo\n"
//                         << "  - il dispositivo sia connesso alla stessa rete del Raspberry Pi";
//                    break;
//            }
//            gameOver();
//            quit(4 + SOURCE);
//        }
        
        resize(source, source, RES[INPUT], 0, 0);
        if (SOURCE == CAM) {
            flip(source, source, -1);
        }
        
        // Find quad
        if (!found && findQuad(source, transmtx, RES[OUT].width)) {
            found = true;
        }
        if (found) {
            findQuad(source, transmtx, RES[OUT].width);
            warpPerspective(source, cropped, transmtx, RES[OUT]);
            resize(cropped, dst, RES[CROP]);
            dst.copyTo(source(Rect(0, 0, RES[CROP].width, RES[CROP].height)));
            rectangle(source, Point(0, 0), Point(RES[CROP].width, RES[CROP].height), Scalar(20, 255, 0), 1, 8);
        }
        imshow("Prospettiva", source);
    }
    destroyAllWindows();
    system("clear");
    return transmtx;
}

bool isin(int n, const int list[], int len) {
    for (int i = 0; i < len; i++) {
        if (n == list[i]) return true;
    }
    return false;
}

bool findQuad(Mat &src, Mat &transmtx, int OUT_RES) {
    bool ret = false;
    
    // colors
    const Scalar RED = {0, 0, 240};
    const Scalar BLUE = {255, 80, 50};
    const Scalar YELLOW = {0, 220, 255};
    const Scalar GREEN = {20, 255, 0};
    const Scalar CYAN = {255, 240, 0};
    
    // find black and orange
    Mat bin;
//    cvtColor(src, bin, COLOR_BGR2Lab);
    cvtColor(src, bin, COLOR_BGR2GRAY);
    bin -= mean(bin)[0];
    bin *= 256;
    blur(bin, bin, Size(8, 8));
    threshold(bin, bin, 100, 255, THRESH_BINARY);
    bitwise_not(bin, bin);
//    imshow("BW", bin);
    
    // find contours
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(bin, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
    
    if (!contours.empty()) {
        // init
        int dist_center = src.cols;
        int inCenter = 0;
        Point src_center = {src.cols / 2, src.rows / 2};
        line(src, src_center - Point(4, 0), src_center + Point(4, 0), YELLOW, 1, 8);
        line(src, src_center - Point(0, 4), src_center + Point(0, 4), YELLOW, 1, 8);
        
        // find the nearest contour to center
        Moments mu;
        Point mc;
        for (int i = 0; i < contours.size(); i++) {
            if (contourArea(contours[i]) > src.cols * src.rows / 8) {
                mu = moments(contours[i], false);
                mc = Point(mu.m10 / mu.m00, mu.m01 / mu.m00);
                int a = (int) norm(mc - src_center);
                if (a < dist_center) {
                    dist_center = a;
                    inCenter = i;
                }
            }
            // drawContours(src, contours, i, CYAN, 1, 8);
        }
        // drawContours(src, contours, inCenter, YELLOW, 2, 8);
        
        // calc poly of nearest contour
        vector<Point> poly;
        double epsilon = 0.018 * arcLength(contours[inCenter], true);
        approxPolyDP(Mat(contours[inCenter]), poly, epsilon, true);
        Point2f Q[4];
        
        // calc quad
        if (poly.size() == 4) {
            // find top-left vertex
            int dist_vert = (int) norm(Point(0, 0) - poly[0]);
            int vertex = 0;
            for (int i = 1; i < 4; i++) {
                int a = (int) norm(Point(0, 0) - poly[i]);
                if (a < dist_vert) {
                    dist_vert = a;
                    vertex = i;
                }
            }
            
            // calc vertexes
            for (int i = 0; i < 4; i++) {
                Q[i] = poly[(vertex + i) % 4];
            }
            for (int i = 0; i < 4; i++) {
                line(src, Q[i], Q[(i + 1) % 4], GREEN, 2, LINE_AA);
            }
            circle(src, Q[0], 10, YELLOW, FILLED, LINE_AA);
            for (auto &q : Q) {
                circle(src, q, 6, RED, FILLED, LINE_AA);
            }
            
            // correct perspective
            Point2f P[4];
            P[0] = Point2f(0, 0);
            P[1] = Point2f(0, OUT_RES);
            P[2] = Point2f(OUT_RES, OUT_RES);
            P[3] = Point2f(OUT_RES, 0);
            vector<Point2f> quad_pts;
            vector<Point2f> squre_pts;
            for (int i = 0; i < 4; i++) {
                quad_pts.push_back(Q[i]);
                squre_pts.push_back(P[i]);
            }
            transmtx = getPerspectiveTransform(quad_pts, squre_pts);
            ret = true;
        }
    }
    return ret;
}


double calc_m(const Point &A, const Point &B) {
    return (B.x - A.x != 0) ?
           double(B.y - A.y) / double(B.x - A.x) : INFINITY;
}


int calc_dir(const Point &A, const Point &B) {
    return atan(calc_m(A, B)) * 180 / CV_PI;
}


int calc_dist(const Point &A, const Point &B, const Point &M) {
    double m = calc_m(A, B);
    double q = A.y - m * A.x;
    return abs(M.y - m * M.x - q);
}


int size(const int *joint) {
    int ret;
    for (ret = 0; joint[ret] >= 0; ret++);
    return ret;
}


void sort(int *srt, int contourSize, vector<int> len) {
    int *len_sort = new int[contourSize];
    
    for (int i = 0; i < contourSize; i++) {
        i = i;
        len_sort[i] = len[i];
    }
    
    int max, tmp;
    
    for (int i = 0; i < contourSize; i++) {
        max = len_sort[i];
        for (int j = i; j < contourSize; j++) {
            if (len_sort[j] > max) {
                max = len_sort[j];
                
                tmp = len_sort[i];
                len_sort[i] = len_sort[j];
                len_sort[j] = tmp;
                
                tmp = i;
                i = j;
                j = tmp;
            }
        }
    }
}


//void connect (int** joint, int* srt, int contourSize, vector <int> len, vector <int> dir, vector <Point> pointM, int MIN_LEN, int OUT_RES)
//{
//    int k;
//    
//    for (int i = 0; i < contourSize; i++)
//    {
//        k = 0;
//        
//        Mat tmp_i =  Mat (OUT_RES, OUT_RES, CV_8U, Scalar(0));
//        ellipse (tmp_i, Point(pointM[i]), size(len[i], MIN_LEN), dir[i], 0, 360, Scalar(255), FILLED);
//        
//        for (int j = i; j < contourSize; j++)
//        {
//            if (i != j)
//            {
//                if (len[i] <= MIN_LEN && len[j] <= MIN_LEN)
//                {
//                    if (norm(pointM[i]-pointM[j]) < 2 * MIN_LEN)
//                        joint[i][k++] = j;
//                }
//                else
//                {
//                    Mat tmp_j =  Mat (OUT_RES, OUT_RES, CV_8U, Scalar(0));
//                    ellipse (tmp_j, Point(pointM[j]), size(len[j], MIN_LEN), dir[j], 0, 360, Scalar(255), FILLED);
//                    int area_j = countNonZero (tmp_j);
//                    bitwise_and (tmp_i, tmp_j, tmp_j);
//                    int area_and = countNonZero (tmp_j);
//                    if ((double)area_and / (double)area_j * 100 > AREA_RATIO)
//                        joint[i][k++] = j;
//                }
//            }
//            else
//                joint[i][k++] = j;
//        }
//        joint[i][k] = -1;
//    }
//}


void disjoin(int **joint, int contourSize) {
    for (int i = 0; i < contourSize; i++) {
        int last = size(joint[i]);
        if (joint[i][0] >= 0) {
            for (int j = 1; j < last; j++) {
                int idx = joint[i][j];
                if (size(joint[idx]) > 1) {
                    for (int k = 1; k < size(joint[idx]); k++) {
                        bool found_new = true;
                        for (int h = j + 1; h < last; h++) {
                            if (joint[i][h] == joint[idx][k]) {
                                found_new = false;
                            }
                        }
                        if (found_new) {
                            joint[i][last] = joint[idx][k];
                            joint[i][++last] = -1;
                        }
                    }
                }
                joint[idx][0] = -1;
            }
        }
    }
}


void debug(int contourSize, int **joint, const int *srt) {
    if (contourSize > 1) {
        for (int i = 0; i < contourSize; i++) {
            for (int j = 0; joint[i][j] >= 0; j++) {
                cout << joint[i][j] << ' ';
            }
            cout << endl;
        }
        cout << "----" << endl;
        for (int i = 0; i < contourSize; i++) {
            for (int j = 0; joint[i][j] >= 0; j++) {
                cout << joint[i][j] << ' ';
            }
            cout << endl;
        }
        cout << "________" << endl << endl;
    }
}


void extremes(const RotatedRect &minRect, Point &A, Point &B) {
    Point2f rect_points[4], meds[4];
    minRect.points(rect_points);
    for (int j = 0; j < 4; j++) {
        meds[j] = (rect_points[j] + rect_points[(j + 1) % 4]) / 2;
    }
    int min = norm(meds[0] - meds[2]) < norm(meds[1] - meds[3]);
    A = meds[0 + min];
    B = meds[2 + min];
}


Point med(const Point& A, const Point& B) {
    return {(A.x + B.x) / 2, (A.y + B.y) / 2};
}



//void save (char path[])
//{
//    gotoxy (1, 23);
//    cout << c_blink << " Premere " << c_bold << f_azr << "[S]" << c_normal << c_blink << " per salvare la registrazione";
//    gotoxy (1, 24);
//    cout << " o un altro tasto per uscire" << c_normal;
//    char ch = getch ();
//    coutxy (tab(54), 1, 23);
//    coutxy (tab(54), 1, 24);
//    if (ch == 's' || ch == 'S')
//    {
//        gotoxy (1, 22);
//        cout << " Video " << c_bold << path << c_normal << " salvato";
//    }
//    else
//    {
//        gotoxy (1, 23);
//        char cmd[256] = "rm ";
//        strcat (cmd, path);
//        system (cmd);
//        cout << " Video eliminato";
//    }
//    gotoxy (1, 24);
//}


void save(const string &path, const string &file1, const string &file2) {
    coutxy(tab(54), 1, 23);
    gotoxy(1, 23);
    cout << c_blink << " Premere " << c_bold << f_azr << "[S]" << c_normal << c_blink
         << " per salvare la registrazione";
    gotoxy(1, 24);
    cout << " o un altro tasto per uscire" << c_normal;
    char ch = getch();
    coutxy(tab(54), 1, 23);
    coutxy(tab(54), 1, 24);
    gotoxy(1, 23);
    if (ch == 's' || ch == 'S') {
        cout << " Video salvati in " << c_bold << path << c_normal;
    } else {
        system(("rm " + path + file1).c_str());
        system(("rm " + path + file2).c_str());
        cout << " Video eliminati";
    }
    gotoxy(1, 24);
}


void quit(int n) {
    destroyAllWindows();
    system("clear");
    cerr << c_normal;
    cout << c_normal;
    cursor();
    cooked();
    echo();
    exit(n);
}


void handleKbEvents(int &key, int &select, vector<IniParam> iniPrm, Parameter prm[], bool &frameByFrame, TickMeter tm, int delay) {
    if (key != -1) {
        switch (key) {
            // Restore
            case '0':
            case 176 + 0:
            case 'r':
            case 'R':
                for (int i = 0; i < iniPrm.size(); i++) {
                    prm[i].value = iniPrm[i].i;
                }
                break;
                
                // change prm_name
            case '5':
            case 176 + 5:
            case KEY_LEFT:
                if (prm[select].hasTxt) {
                    prm[select].value = (prm[select].value > prm[select].min)
                                        ? prm[select].value - 1 : prm[select].max;
                } else {
                    prm[select].value -= (prm[select].value > prm[select].min);
                }
                break;
            case '6':
            case 176 + 6:
            case KEY_RIGHT:
                if (prm[select].hasTxt) {
                    prm[select].value = (prm[select].value + 1) % (prm[select].max + 1);
                } else {
                    prm[select].value += (prm[select].value < prm[select].max);
                }
                break;
                
                // vertcal selector
            case '7':
            case 176 + 7:
            case KEY_UP:
                select--;
                if (select < 0) select = iniPrm.size() - 1;
                break;
            case '4':
            case 176 + 4:
            case KEY_DOWN:
                select = (select + 1) % iniPrm.size();
                break;
                
                // gameOver
            case '+':
            case 171:
//                if (GUI) {
                gotoxy(21, 9);
                cout << f_vlt << ln_v << ln_v << c_normal;
//                }
                frameByFrame = true;
                tm.stop();
                break;
        }
    }
    
    // WaitKey
    if (frameByFrame) {
//        if (GUI) {
            gotoxy(21, 9);
            cout << f_vlt << ln_v << ln_v << c_normal;
//        }
        key = waitKeyEx();
        if (key == '+' || key == 171) {
            frameByFrame = false;
//            if (GUI)
            coutxy("  ", 21, 9);
        }
    }
    tm.start();
    key = waitKeyEx(delay);
}