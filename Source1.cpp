#include "Header.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <omp.h>

cv::Mat etalImage;
//cv::Point pt1, pt2;
//bool drawing = false;          // рисуем ли
//cv::Mat grayCropped;
//const int cap_len(320), cap_height(240);



void corr_init(cv::Mat& etal_frame)
{
    etalImage = etal_frame;
};





cv::Point corr_next(cv::Mat& main_frame) {
    const int rows_etal = etalImage.rows;
    const int cols_etal = etalImage.cols;

    const int rows_main = main_frame.rows;
    const int cols_main = main_frame.cols;

    int MIN_S = INT_MAX;
    int alpha = 0, beta = 0;

    uchar* main_data = main_frame.data;
    uchar* etal_data = etalImage.data;
    int main_step = static_cast<int>(main_frame.step1());
    int etal_step = static_cast<int>(etalImage.step1());

    // Локальные переменные для минимального значения на каждый поток
#pragma omp parallel
    {
        int local_MIN_S = INT_MAX;
        int local_alpha = 0;
        int local_beta = 0;

#pragma omp for collapse(2) schedule(dynamic)
        for (int k = 0; k <= rows_main - rows_etal; k++) {
            for (int l = 0; l <= cols_main - cols_etal; l++) {
                int S = 0;

                for (int i = 0; i < rows_etal; i++) {
                    if (S > local_MIN_S) break;

                    for (int j = 0; j < cols_etal; j++) {
                        int main_val = main_data[(k + i) * main_step + (l + j)];
                        int etal_val = etal_data[i * etal_step + j];
                        S += std::abs(main_val - etal_val);
                        if (S > local_MIN_S) break;
                    }
                }

                if (S < local_MIN_S) {
                    local_MIN_S = S;
                    local_alpha = k;
                    local_beta = l;
                }
            }
        }

        // Критическая секция для обновления глобального минимума
#pragma omp critical
        {
            if (local_MIN_S < MIN_S) {
                MIN_S = local_MIN_S;
                alpha = local_alpha;
                beta = local_beta;
            }
        }
    }

    return cv::Point(beta, alpha);
}