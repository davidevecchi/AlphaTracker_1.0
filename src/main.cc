#include "gui.h"
#include "stack.h"
#include "console.h"
#include "settings.h"
#include "functions.h"

#define INIT_MAT Mat(RES[OUT].width, RES[OUT].height, CV_8U, Scalar(0))

#define GUI true
//#define DEBUG false

using namespace std;
using namespace cv;


// FIXME camera source
// TODO hints


int main(int argc, char **argv) {


/**** [ Initialize ] ****/
    
    if (argc != 2) {
        cerr << "Usage: ./cmake-build-debug/main \"$PWD/\"\n";
        exit(0);
    }
    
    // Common
    char chr;
    char bfr[256];
    
    // Console
    streambuf *cerrSbuf = cerr.rdbuf();
    ofstream ferr("/dev/null");
    cerr.rdbuf(ferr.rdbuf());
    nocursor();
    noecho();



/**** [ Settings ] ****/
    
    // Select source or edit settings
    Settings st(argv[1]);
    window("KBevent", Size(0, 0), Point(4000, 4000), WINDOW_GUI_NORMAL);
    const int SOURCE = st.selectSource();  // ask user to select source or edit settings
    destroyAllWindows();
    
    // FIXME Resolutions
    Size RES[6];  // I/O resolutions
    RES[SCREEN] = st.resolution("Schermo");
    RES[INPUT] = st.resolution(SOURCES[SOURCE]);
    RES[OUT] = st.resolution("Output");
    RES[SRC] = {RES[SCREEN].width / 2, (RES[SCREEN].width / 2) * RES[INPUT].height / RES[INPUT].width};
    RES[CROP] = {RES[INPUT].height / 4, RES[INPUT].height / 4};
    RES[OUTPUT] = {RES[SRC].width, RES[SRC].width};
    const int MIN_LEN = RES[OUT].width / 30;



/**** [ Video source ] ****/
    
    // Get capture
    int totFrame, totTime;
    VideoCapture cap = getCapture(st, totFrame, totTime, SOURCE, RES);
    
    // Perspective
    window("Prospettiva", RES[SRC], Point(RES[SCREEN].width / 2, RES[SCREEN].height - RES[SRC].height), WINDOW_GUI_NORMAL);
    // Gui::instructions();  // TODO
    Mat transmtx = findPerspective(st, cap, SOURCE, RES);



/**** [ Init stuff ] ****/
    
    // Matrixes
    int out = BKG;  // output matrix index [0...N_MAT_OUT)
    Mat kernel1 = getStructuringElement(MORPH_RECT, Size(1, 1));
    Mat kernel9 = getStructuringElement(MORPH_ELLIPSE, Size(9, 9));
    Mat mtx[N_MAT_OUT + N_MAT_WORK + N_MAT_WORK2];
    for (auto &m : mtx) m = INIT_MAT;
    
    // Parameters
    int prmSize = st.iniList["PARAMETRI"].size();
    int prmExtraSize = st.iniList["PARAMETRI_EXTRA"].size();
    Parameter prm[prmSize + prmExtraSize];
    prm[MSK] = {0, N_MAT_OUT - 1, true, {"originale", "sfocata  ", "smussata ", "vuota    "}};
    prm[OPC] = {0, 10, false};
    prm[RAY] = {0, 1, true, {OFF, ON}};
    prm[NUM] = {0, 1, true, {OFF, ON}};
    prm[ELP] = {0, 1, false};
    prm[SEG] = {0, 1, false};
    prm[CON] = {0, 1, false};
    prm[S_F] = {0, 1, false};
    for (int i = 0; i < prmSize; i++) prm[i].value = st.iniList["PARAMETRI"][i].i;
    for (int i = 0; i < prmExtraSize; i++) prm[i + prmSize].value = st.iniList["PARAMETRI_EXTRA"][i].i;
    
    // Variables
    int var[st.iniList["VARIABILI"].size()];
    for (int i = 0; i < st.iniList["VARIABILI"].size(); i++) {
        var[i] = st.iniList["VARIABILI"][i].i;
    }
    
    // Tracks TODO docs
    Ptr<BackgroundSubtractor> bgSub = createBackgroundSubtractorMOG2(80, 24, false);
    int voidFrames = 0;
    int segmentsSize = 0;
    int **joint;
    //int *srt;  // FIXME?
//    int id = 0;
    int counter = 0;
    vector<int> alive;
    vector<int> aliveNew;
    Ray ray;
    stack s;
    init(s);
    
    // Video file output
    window("Tracking", RES[OUTPUT], Point(RES[SCREEN].width / 2, 36), WINDOW_GUI_NORMAL);
    string videoSourceName = "crop_" + currentDateTime() + ".mp4";
    string videoCropName = "source_" + currentDateTime() + ".mp4";
    string videoSourcePath = st.ini["SALVATAGGIO.Percorso"].s + videoSourceName;
    string videoCropPath = st.ini["SALVATAGGIO.Percorso"].s + videoCropName;
    VideoWriter videoSource(videoSourcePath, VideoWriter::fourcc('a', 'v', 'c', '1'), var[FPS_OUT], RES[OUT]);
    VideoWriter videoCrop(videoCropPath, VideoWriter::fourcc('a', 'v', 'c', '1'), var[FPS_OUT], RES[INPUT]);
    
    // Interface and time
    Gui::guiStable(st.iniList);
    int select = 0;             // selected element
    int key = -1;               // last key pressed
    bool frameByFrame = false;  // play frame-by-frame
    bool go = true;             // loop guard
    double time;                // time elapsed
    int fps;                    // processed frames per second
    TickMeter tm;               // timer
    tm.start();



/**** [ Video loop ] ****/
    
    for (int f = 1; go; f++) {
        
        cap.read(mtx[frame]);
        //--> mtx[frame]
        
        // ERROR TODO
        if (mtx[frame].empty()) {
            destroyAllWindows();
/*
            gotoxy(1, 22);
            cerr.rdbuf(cerrSbuf);
            cerr << f_red << endl;
            switch (SOURCE) {
                case CAM:
                    cerr << " Impossibile comunicare con la fotocamera " << st.ini["GENERALE.Camera"].s << endl
                         << f_ylw << "\n Verificare che:\n"
                         << "  - il dispositivo sia funzionante\n"
                         << "  - il dispositivo sia collegato correttamente";
                    break;
                case VID:
                    if (f - 1 == totFrame) {
                        cerr << f_ylw << " Video finito al frame " << f - 1;
                    } else {
                        cerr << " Impossibile riprodurre il video, file danneggiato al frame " << f;
                    }
                    break;
                case IPC:
                    cerr << " Impossibile comunicare con IP camera\n"
                         << f_ylw << "\n Verificare che:\n"
                         << "  - il dispositivo sia attivo\n"
                         << "  - il dispositivo sia connesso alla stessa rete del Raspberry Pi";
                    break;
            }
            cerr << c_normal;
*/
            go = false;
        }
        
        // Quit keyboard
        const int KEYS[4] = {KEY_ESC, KEY_SPACE, KEY_ENTER, KEY_ENTER_ALT};
        if (go && isin(key, KEYS, 4)) {
            destroyAllWindows();
            gotoxy(1, 22);
            cout << f_ylw << " Fermato al frame " << f - 1 << c_normal;
            go = false;
        }
        
        // Here comes the magic
        if (go) {
            
            handleKbEvents(key, select, st.iniList["PARAMETRI"], prm, frameByFrame, tm, var[DELAY]);
            
            
            /**** [ Process frame ] ****/
            
            // Perspective
            resize(mtx[frame], mtx[frame], RES[INPUT]);
            if (SOURCE == CAM) flip(mtx[frame], mtx[frame], -1);
            warpPerspective(mtx[frame], mtx[crop], transmtx, RES[OUT]);
            //--> mtx[crop]
            
            // Background mask
            cvtColor(mtx[crop], mtx[gray], COLOR_BGR2GRAY);
            bgSub->apply(mtx[gray], mtx[MASK]);
            rectangle(mtx[MASK], Point(0, 0), (Point) RES[OUT], Scalar(0), 2 * MIN_LEN, 8);  // erase borders
            dilate(mtx[MASK], mtx[MASK], kernel1, Point(-1, -1), 4);
            erode(mtx[MASK], mtx[MASK], kernel1, Point(-1, -1), 4);
            //--> mtx[MASK]
            
            
            // Critical change
            if (voidFrames || countNonZero(mtx[MASK]) > (RES[OUT].width * RES[OUT].height) / var[VOID_AREA]) {
                
                /**** [ Pause analysis ] ****/
                
                voidFrames = (voidFrames) ? voidFrames - 1 : var[VOID_FRM];
                if (prm[OPC].value > 0) {
                    cvtColor(mtx[prm[MSK].value], mtx[RGB], COLOR_GRAY2BGR);
                    mtx[BKG] = mtx[crop] * prm[OPC].value / 10 + mtx[RGB] * (10 - prm[OPC].value) / 10;
                    out = BKG;
                } else {
                    out = prm[MSK].value;
                }
                
            } else {
                
                /**** [ 1. - Analize frame ] ****/
                
                // Blur and threshold FIXME
                blur(mtx[MASK], mtx[BLUR], Size(var[BLUR_VL], var[BLUR_VL]));
                threshold(mtx[BLUR], mtx[BLOB], var[THR_VL], 255, THRESH_BINARY);
                //--> mtx[BLUR], mtx[BLOB]
                
                // Matrix with new blobs FIXME
                bitwise_not(mtx[PRV], mtx[PRV]);
                bitwise_and(mtx[PRV], mtx[BLOB], mtx[NEW]);              // find new blobs
                bitwise_or(mtx[NEW], mtx[BLOB], mtx[NOW]);               // store current blobs
                dilate(mtx[BLOB], mtx[PRV], kernel9, Point(-1, -1), 6);  // update PRV with current blobs
                //--> mtx[NEW], mtx[PRV], mtx[NOW]
                
                // Find new segments FIXME
                vector<vector<Point>> contoursFound;
                findContours(mtx[NEW], contoursFound, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                vector<Segment> segments;
                for (auto &c : contoursFound) segments.emplace_back(Segment(c));  // approx segments
                segmentsSize = segments.size();
                //--> segments, segmentsSize
                
                if (!segments.empty()) {
                    
                    /**** [ 1.2. - Edit alive rays ] ****/  // TODO here
                    
                    for (const int &id : alive) {
                        
                        bool editRay = false;  //!< ray must be edited
                        dig_ray(s, id, ray);
                        Mat tmp = INIT_MAT;
                        line(tmp, ray.A, ray.B, Scalar(255), 2);
                        
                        
                        /**** [ 1.2.1. - Connect aligned blobs ] ****/
                        
                        for (const auto &ct : segments) {
                            
                            if (ray.len <= MIN_LEN && ct.len <= MIN_LEN) {
                                
                                if (norm(ray.M - ct.M) < 2 * MIN_LEN) {
                                    line(tmp, ct.A, ct.B, Scalar(255), 2);
                                    line(tmp, ray.M, ct.M, Scalar(255), 2);
                                    drawContours(mtx[NEW], contoursFound, &ct - &segments[0], Scalar(0), FILLED);
                                    editRay = true;
                                }
                                
                                /** Start edit **/
//                                else if (abs(ray.frm - f) <= 5) {
//                                    line(tmp, ct.A, ct.B, Scalar(255), 2);
//                                    line(tmp, rayM, ct.M, Scalar(255), 2);
//                                    drawContours(mtx[NEW], contoursFound, &ct - &segments[0], Scalar(0), FILLED);
//                                    editRay = true;
//                                }
                                /** End edit **/
                                
                            } else {
                                
                                Mat tmpi = INIT_MAT;
                                Mat tmpj = INIT_MAT;
                                
                                ellipse(tmpi, ray.M,
                                        Size(max(var[LEN_MULT] * ray.len, MIN_LEN), MIN_LEN),
                                        ray.dir, 0, 360, Scalar(255), FILLED);
                                ellipse(tmpj, ct.M,
                                        Size(max(var[LEN_MULT] * ct.len, MIN_LEN), MIN_LEN),
                                        ct.dir, 0, 360, Scalar(255), FILLED);
                                
                                bitwise_and(tmpi, tmpj, tmpj);
                                int area_i = countNonZero(tmpi);
                                int area_j = countNonZero(tmpj);
                                int area_and = countNonZero(tmpj);
                                
                                if ((double) area_and / (double) min(area_i, area_j) * 100 > var[AREA_RATIO]) {
                                    
                                    tmpj *= 0;
                                    line(tmpj, ray.A, ray.B, Scalar(255), 2);
                                    line(tmpj, ct.A, ct.B, Scalar(255), 2);
                                    line(tmpj, ray.M, ct.M, Scalar(255), 2);
                                    
                                    vector<vector<Point>> contour(1);
                                    findContours(tmpj, contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                                    RotatedRect approxRect = minAreaRect(Mat(contour[0]));
                                    Point A, B;
                                    extremes(approxRect, A, B);
                                    
                                    int dir_diff = abs(ray.dir - calc_dir(A, B));
                                    int len_diff = abs((ray.len + ct.len) - norm(A - B));
                                    int dist_diff = calc_dist(ray.A, ray.B, (A + B) / 2);
                                    
                                    if (dir_diff < var[DIR_DIFF] && dist_diff < MIN_LEN &&
                                        len_diff < 4 * MIN_LEN) {
                                        line(tmp, ct.A, ct.B, Scalar(255), 2);
                                        line(tmp, ray.M, ct.M, Scalar(255), 2);
                                        drawContours(mtx[NEW], contoursFound, &ct - &segments[0], Scalar(0), FILLED);
                                        editRay = true;
                                    }
                                    
                                }
                                
                            }
                            
                        }
                        
                        
                        /**** [ 1.2.2. - Update rays and segments ] ****/
                        
                        if (editRay) {
                            
                            vector<vector<Point>> contour(1);
                            findContours(tmp, contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                            RotatedRect approxRect = minAreaRect(Mat(contour[0]));
                            Point A, B;
                            extremes(approxRect, A, B);
                            ray = {A, B, (int) norm(B - A), (int) calc_dir(A, B), ray.frm};
                            edit(s, id, ray);
                            
                            // Update segments
                            findContours(mtx[NEW], contoursFound, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	                        segmentsSize = segments.size();
                            for (auto &c : contoursFound) segments.emplace_back(Segment(c));  // approx segments
                            
                        }
                        
                    }
                    
                }
                
                
                /**** [ 2. - Connect aligned rays ] ****/
                
                for (const int &id : alive) {
                    
                    bool edit_ray = false;
                    if (id != 0) {
                        
                        dig_ray(s, id, ray);
                        Mat tmp = INIT_MAT;
                        line(tmp, ray.A, ray.B, Scalar(255), 2);
                        
                        for (int &id0 : alive) {
                            
                            if (id != 0) {
                                
                                int count, count0;
                                Ray ray0;
                                dig_ray(s, id0, ray0);
                                dig_count(s, id, count);
                                dig_count(s, id0, count0);
                                
                                if (id0 != id && count0 == 0 && ray.len >= ray0.len) {
                                    
                                    if (ray.len <= MIN_LEN && ray0.len <= MIN_LEN) {
                                        
                                        if (norm(ray.M - ray0.M) < 2 * MIN_LEN) {
                                            line(tmp, ray0.A, ray0.B, Scalar(255), 2);
                                            line(tmp, ray.M, ray0.M, Scalar(255), 2);
                                            pop(s, id0);
                                            id0 = 0;
                                            edit_ray = true;
                                        }
                                        
                                        /** Start edit **/
//                                    } else if (abs(ray.frm - ray0.frm) <= 5) {
//                                        line(tmp, ray0.A, ray0.B, Scalar(255), 2);
//                                        line(tmp, rayM, ray0M, Scalar(255), 2);
//                                        pop(s, id0);
//                                        alive[al] = 0;
//                                        edit_ray = true;
                                        /** End edit **/
                                        
                                    } else {
                                        
                                        Mat tmpi = INIT_MAT;
                                        Mat tmpj = INIT_MAT;
                                        
                                        ellipse(tmpi, ray.M, Size(max(var[LEN_MULT] * ray.len, MIN_LEN), MIN_LEN),
                                                ray.dir, 0, 360, Scalar(255), FILLED);
                                        ellipse(tmpj, ray0.M, Size(max(var[LEN_MULT] * ray0.len, MIN_LEN), MIN_LEN),
                                                ray0.dir, 0, 360, Scalar(255), FILLED);
                                        
                                        int area_i = countNonZero(tmpi);
                                        int area_j = countNonZero(tmpj);
                                        bitwise_and(tmpi, tmpj, tmpj);
                                        int area_and = countNonZero(tmpj);
                                        
                                        if ((double) area_and / double(min(area_i, area_j)) * 100 > var[AREA_RATIO]) {
                                            
                                            tmpj *= 0;
                                            line(tmpj, ray.A, ray.B, Scalar(255), 2);
                                            line(tmpj, ray0.A, ray0.B, Scalar(255), 2);
                                            line(tmpj, ray.M, ray0.M, Scalar(255), 2);
                                            
                                            vector<vector<Point>> contour(1);
                                            findContours(tmpj, contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                                            RotatedRect approxRect = minAreaRect(Mat(contour[0]));
                                            Point A, B;
                                            extremes(approxRect, A, B);
                                            
                                            int dir_diff = abs(ray.dir - calc_dir(A, B));
                                            int len_diff = abs(norm(A - B) - (ray.len + ray0.len)) * 2;
                                            int dist_diff = calc_dist(ray.A, ray.B, (A + B) / 2);
                                            
                                            if (dir_diff < var[DIR_DIFF] && dist_diff < MIN_LEN &&
                                                len_diff < norm(A - B)) {
                                                line(tmp, ray0.A, ray0.B, Scalar(255), 2);
                                                line(tmp, ray.M, ray0.M, Scalar(255), 2);
                                                pop(s, id0);
                                                id0 = 0;
                                                edit_ray = true;
                                            }
                                            
                                        }
                                        
                                    }
                                    
                                }
                                
                            }
                            
                        }
                        
                        // Update ray and segments
                        if (edit_ray) {
                            vector<vector<Point>> contour(1);
                            findContours(tmp, contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                            RotatedRect approxRect = minAreaRect(Mat(contour[0]));
                            Point A, B;
                            extremes(approxRect, A, B);
                            ray = {A, B, (int) norm(B - A), (int) calc_dir(A, B), ray.frm};
                            edit(s, id, ray);
                        }
                        
                    }
                    
                }
                
                // Remove zeros
                alive.erase(remove(alive.begin(), alive.end(), 0), alive.end());
                
                
                /**** [ 3. - Populate ] ****/
                
                if (segmentsSize > 0) {
                    
                    int id = 0;
                    
                    // Find disjointed tracks
                    if (segmentsSize == 1) {
                        
                        ray = {segments[0].A, segments[0].B, segments[0].len, segments[0].dir, f};
                        push(s, ray);
                        top_id(s, id);
                        aliveNew.clear();
                        aliveNew.push_back(id);
                        
                    } else {
                        
                        // delete joint FIXME
                        joint = new int *[segmentsSize];
                        for (int i = 0; i < segmentsSize; i++) {
                            joint[i] = new int[segmentsSize + 1];
                        }
                        
                        sort(segments.begin(), segments.end(), Segment::compare);  // FIXME?
                        
                        // Join segments (from longest to shortest)
                        for (int i = 0; i < segmentsSize; i++) {
                            
                            int k = 0;
                            Mat tmpi = INIT_MAT;
                            ellipse(tmpi, segments[i].M,
                                    Size(max(var[LEN_MULT] * segments[i].len, MIN_LEN), MIN_LEN),
                                    segments[i].dir, 0, 360, Scalar(255), FILLED);
                            
                            for (int j = i; j < segmentsSize; j++) {
                                
                                if (i != j) {
                                    
                                    if (segments[i].len <= MIN_LEN && segments[j].len <= MIN_LEN) {
                                        
                                        if (norm(segments[i].M - segments[j].M) < 2 * MIN_LEN) {
                                            joint[i][k++] = j;
                                        }
                                        
                                    } else {
                                        
                                        Mat tmpj = INIT_MAT;
                                        ellipse(tmpj, segments[j].M,
                                                Size(max(var[LEN_MULT] * segments[j].len, MIN_LEN), MIN_LEN),
                                                segments[j].dir, 0, 360, Scalar(255), FILLED);
                                        
                                        int area_j = countNonZero(tmpj);
                                        bitwise_and(tmpi, tmpj, tmpj);
                                        int area_and = countNonZero(tmpj);
                                        if ((double) area_and / (double) area_j * 100 > var[AREA_RATIO]) {
                                            joint[i][k++] = j;
                                        }
                                        
                                    }
                                    
                                } else {
                                    
                                    joint[i][k++] = j;
                                    
                                }
                            }
                            
                            joint[i][k] = -1;
                            
                        }

//                        if (DEBUG) {
//                            debug(segmentsSize, joint, srt);
//                        }
                        
                        if (size(joint[0]) == segmentsSize) {
                            for (int i = 1; i < segmentsSize; i++) joint[i][0] = -1;
                        } else if (segmentsSize > 2) {
                            disjoin(joint, segmentsSize);
                        }
                        
                        for (int i = 0; i < segmentsSize; i++) {
                            
                            if (joint[i][0] >= 0) {
                                
                                Mat tmp = INIT_MAT;
                                for (int j = 0; j < size(joint[i]); j++) {
                                    line(tmp, segments[joint[i][j]].A, segments[joint[i][j]].B, Scalar(255), 2);
                                    if (joint[i][j + 1] >= 0) {
                                        line(tmp, segments[joint[i][j]].M, segments[joint[i][j + 1]].M, Scalar(255), 2);
                                    }
                                }
                                
                                vector<vector<Point>> contour(1);
                                findContours(tmp, contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
                                RotatedRect approxRect = minAreaRect(Mat(contour[0]));
                                Point A, B;
                                extremes(approxRect, A, B);
                                
                                ray = {A, B, (int) norm(B - A), (int) calc_dir(A, B), f};
                                push(s, ray);
                                top_id(s, id);
                                aliveNew.push_back(id);
                                
                            }
                            
                        }
                        
                    }
                    
                }
                
                
                /**** [ 4. - Life or death ] ****/
                
                for (int &id : alive) {
                    
                    int life = 1;
                    int count = 0;
                    dig_ray(s, id, ray);
                    
                    Mat tmp = INIT_MAT;
                    line(tmp, ray.A, ray.B, Scalar(255), 2);
                    dilate(tmp, tmp, kernel9, Point(-1, -1), 6);
                    bitwise_and(tmp, mtx[NOW], tmp);
                    
                    if (countNonZero(tmp) > 0) {
                        edit(s, id, true);
                    } else {
                        dig_life(s, id, life);
                        if (life >= var[MIN_LIFE]) {
                            edit(s, id, false);
                        } else {
                            pop(s, id);
                        }
                        id = 0;
                    }
                    
                    dig_life(s, id, life);
                    dig_count(s, id, count);
                    
                    if (life == var[MIN_LIFE] && count == 0) edit(s, id, ++counter);
                    
                }
                
                // Rearrange alive array
                alive.erase(remove(alive.begin(), alive.end(), 0), alive.end());
                
                
                /**** [ 5. - Show the magic ] ****/
                
                // Background transparency
                if (prm[OPC].value > 0) {
                    cvtColor(mtx[prm[MSK].value], mtx[RGB], COLOR_GRAY2BGR);
                    mtx[BKG] = mtx[crop] * prm[OPC].value / 10 + mtx[RGB] * (10 - prm[OPC].value) / 10; // FIXME sigsegv
                    out = BKG;
                } else {
                    out = prm[MSK].value;
                }
                
                // Draw rays
                if (prm[RAY].value || prm[NUM].value) {
                    for (int &id : alive) {
                        int count;
                        dig_count(s, id, count);
                        if (count > 0) {
                            dig_ray(s, id, ray);
                            if (prm[RAY].value) {
                                line(mtx[out], ray.A, ray.B,
                                     Scalar((prm[OPC].value > 0 ? 20 : 255), 120, 240), 1);
                            }
                            if (prm[NUM].value) {
                                sprintf(bfr, "%i", count);
                                putText(mtx[out], bfr, ray.M, FONT_HERSHEY_DUPLEX, 0.8, Scalar(240, 240, 240), 1);
                            }
                        }
                    }
                }
                
                // Draw segments
                if (prm[CON].value) {
                    for (const auto &ct : segments) {
                        drawContours(mtx[out], contoursFound, &ct - &segments[0], Scalar(0, 255, 0));  // FIXME
                    }
                }
                
                // Draw segments
                if (prm[SEG].value) {
                    for (int i = 0; i < segmentsSize; i++) {
                        line(mtx[out], segments[i].A, segments[i].B, Scalar(0, 240, 255), 2);
                        if (segmentsSize > 1) {
                            for (int j = 0; joint[i][j] != -1; j++) {
                                if (joint[i][j + 1] != -1) {
                                    line(mtx[out], segments[joint[i][j]].M, segments[joint[i][j + 1]].M,
                                         Scalar(200, 0, 200), 1);
                                }
                            }
                        }
                    }
                }
                
                // Draw ellipses
                if (prm[ELP].value) {
                    for (const auto &ct : segments) {
                        ellipse(mtx[out], ct.M, Size(max(var[LEN_MULT] * ct.len, MIN_LEN), MIN_LEN),
                                ct.dir, 0, 360, Scalar(255, 10, 10), 1);
                    }
                }
                
                
                /**** [ 6. - Concat aliveNew ] ****/

//                aliveSize = alive.size();  // FIXME?
                alive.insert(alive.end(), aliveNew.begin(), aliveNew.end());
                aliveNew.clear();
                
            }
            
            
            
            /**** [ Output ] ****/
            
            // Fps
            tm.stop();
            double tmp = tm.getTimeSec();
            bool del = (intlen(fps) > intlen(1 / tmp));
            time = cap.get(CAP_PROP_POS_MSEC) / 1000;
            fps = 1 / tmp;
            tm.reset();
            tm.start();
            
            
            // User interface
            if (GUI) {
                for (int i = 0; i < st.iniList["PARAMETRI"].size(); i++) {
                    gotoxy(2, 4 + 2 * i);
                    if (select == i) {
                        cout << ln_s << "╴";
                    } else {
                        cout << ln_v << " ";
                    }
                    if (prm[i].hasTxt) {
                        coutxy(prm[i].str[prm[i].value], 14, 4 + 2 * i);
                    } else {
                        coutbar(prm[i].value, 0, 10, 14, 4 + 2 * i);
                    }
                }
                cout << f_wht;
                coutxy(counter, 22, 15, RIGHT);
                cout << c_normal;
                if (del) coutxy(" ", 21 - intlen(fps), 17);
                coutxy(fps, 22, 17, RIGHT);
                if (prm[S_F].value == 0) {
                    if (SOURCE == VID) {
                        char bfr_[16];
                        sprintf(bfr, "%i", (int) time);
                        strcat(bfr, "/");
                        sprintf(bfr_, "%i", totTime);
                        strcat(bfr, bfr_);
                        coutxy(bfr, 22, 19, RIGHT);
                    } else {
                        coutxy((int) time, 22, 19, RIGHT);
                    }
                } else {
                    coutxy((int) f, 22, 19, RIGHT);
                }
                cout << c_normal;
                print(s, 29, 4, 17);
            }
            
            // Video output
            imshow("Tracking", mtx[out]);
            if (prm[MSK].value == BLNK) mtx[BLNK] *= 0;
            videoSource.write(mtx[frame]);
            videoCrop.write(mtx[out]);
        }
    }




/**** [ Restore ] ****/
    
    // Release output
    destroyAllWindows();
    cap.release();
    videoSource.release();
    videoCrop.release();
    save(st.ini["SALVATAGGIO.Percorso"].s, videoSourceName, videoCropName);
    save(s, st.ini["SALVATAGGIO.Percorso"].s);
    deinit(s);
    delete[] joint;
    
    // Reset console
    cerr.rdbuf(cerrSbuf);
    cerr << c_normal;
    cout << c_normal;
    cursor();
    cooked();
    echo();
    
    return 0;
    
}
