#pragma once
#include <opencv2/core.hpp>
void corr_init(cv::Mat &etal_frame);
//void corr_init(int event, int p1x, int p1y, int flags, void* userdata);
cv::Point corr_next(cv::Mat& main_frame);

