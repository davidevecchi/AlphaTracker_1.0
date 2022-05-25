#ifndef COMMON_H
#define COMMON_H

#include <opencv2/opencv.hpp>
#include <opencv2/bgsegm.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <map>

using namespace std;

const int N_SOURCES = 3;

const string SOURCES[N_SOURCES] = {
        "FOTOCAMERA",
        "FILE VIDEO",
        "IP CAMERA"
};

enum Sources {
    CAM,  //!< camera
    VID,  //!< video file
    IPC,  //!< ip camera
};

enum Parameters {
    MSK,
    OPC,
    RAY,
    NUM,
    ELP,
    SEG,
    CON,
    S_F
};

enum Variables {
    FPS_OUT,
    DELAY,
    AREA_RATIO,
    DIR_DIFF,
    MIN_LIFE,
    LEN_MULT,
    BLUR_VL,
    THR_VL,
    VOID_FRM,
    VOID_AREA
};

enum Resolutions {
    SCREEN,
    INPUT,
    OUT,
    SRC,
    CROP,
    OUTPUT
};

const int N_MAT_OUT = 4;
const int N_MAT_WORK = 5;
const int N_MAT_WORK2 = 3;

enum outputMat {
    MASK,   //!< bgsub output
    BLUR,   //!< blurred MASK
    BLOB,   //!< thresholded BLUR
    BLNK,   //!< blank
    
    BKG,    //!< original video
    PRV,    //!< previous blobs dilated
    NOW,    //!< current blobs
    NEW,    //!< new blobs from PRV
    RGB,    //!<
    
    frame,  //!< original frame
    crop,   //!< cropped perspecrive
    gray    //!< grayscale crop
};

struct IniParam {
    string param;  //!< parameter name
    string s;      //!< string value
    int i;         //!< int value
} __attribute__((aligned(128)));

struct Parameter {
    int min, max;           //!< value range
    bool hasTxt;            //!< has string values
    string str[N_MAT_OUT];  //!< string values
    int value;              //!< value
} __attribute__((aligned(128)));

class Segment {
public:
    vector<cv::Point> contour;
    cv::RotatedRect minRect;
    cv::Point A;
    cv::Point B;
    cv::Point M;
    int len;
    int dir;
    
    explicit Segment(const vector<cv::Point> &contour);
    
    static bool compare(const Segment &a, const Segment &b);
};

#endif //COMMON_H
