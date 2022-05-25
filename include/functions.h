#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "settings.h"

using namespace std;
using namespace cv;

cv::VideoCapture getCapture(Settings &st, int &totFrame, int &totTime, const int &source, Size RES[]);

void window(const String &name, const Size &size, const Point &pos, int flags = WINDOW_AUTOSIZE);

bool isin(int n, const int list[], int len);

cv::Mat findPerspective(Settings &st, cv::VideoCapture cap, int SOURCE, Size RES[]);

bool findQuad(Mat &src, Mat &transmtx, int OUT_RES);

double calc_m(const Point &A, const Point &B);

int calc_dir(const Point &A, const Point &B);

int calc_dist(const Point &A, const Point &B, const Point &M);

void extremes(const RotatedRect &, Point &, Point &);

Point med(const Point& A, const Point& B);

int size(const int *);

void sort(int *, int, vector<int>);

/*void connect (int**, int*, int, vector <int>, vector <int>, vector <Point>, int MIN_LEN, int OUT_RES);*/
void disjoin(int **, int);

void debug(int contourSize, int **joint, const int *srt);

/*void save (char path[]);*/
void save(const string &path, const string &file1, const string &file2);

void quit(int n);

void handleKbEvents(int &key, int &select, vector<IniParam> iniPrm, Parameter prm[], bool &frameByFrame, TickMeter tm, int delay);

#endif // FUNCTIONS_H
