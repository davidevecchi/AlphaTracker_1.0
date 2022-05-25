#include "functions.h"
#include "common.h"

Segment::Segment(const vector<cv::Point> &contour) : contour(contour) {
    minRect = minAreaRect(cv::Mat(this->contour));
    extremes(minRect, A, B);
    M = (A + B) / 2;
    len = norm(B - A);
    dir = calc_dir(A, B);  // NOLINT(cppcoreguidelines-prefer-member-initializer)
}

bool Segment::compare(const Segment &a, const Segment &b) {
    return a.len > b.len;
}