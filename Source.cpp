#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include "Header.h"
#include <filesystem>
#include <fstream>
#include <random>
#include <cmath>
//cv::Mat etalImage;
cv::Point pt1, pt2;
bool drawing;
cv::Mat now_etal_im;
const int cap_len(320), cap_height(240);
namespace fs = std::filesystem;



//входные параметры p1x и p1y обновляются каждый раз при ведении мыши с зажатой ЛКМ
void mouse_clbk(int event, int p1x, int p1y, int flags, void* userdata) {
	// Явное приведение указателя userdata к указателю на Mat
	cv::Mat* framePtr = reinterpret_cast<cv::Mat*>(userdata);
	if (!framePtr || framePtr->empty()) {
		std::cerr << "Error: empty matrix!" << std::endl;
		return;
	}
	if (event == cv::EVENT_LBUTTONDOWN) {
		drawing = true;
		pt1 = cv::Point(p1x, p1y);  // Устанавливаем точку начала выделения
		pt2 = pt1;
		// drawing = true;
	}
	else if (event == cv::EVENT_MOUSEMOVE && drawing) {
		int x = std::clamp(p1x, 0, framePtr->cols - 1);//Ограничиваем p1x в пределах от 0 до frame.cols-1
		int y = std::clamp(p1y, 0, framePtr->rows - 1);
		pt2 = cv::Point(x, y);  // Обновляем точку конца выделения
		drawing = true;
	}
	else if (event == cv::EVENT_LBUTTONUP) {
		drawing = false;        // Завершаем рисование
		cv::Rect rect(
			std::min(pt1.x, pt2.x),  // левый верхний x
			std::min(pt1.y, pt2.y),  // левый верхний y
			std::abs(pt2.x - pt1.x), // ширина
			std::abs(pt2.y - pt1.y)  // высота
		);
		std::cout << pt1;
		std::cout << pt2;
		//pt2 = cv::Point(p1x, p1y);
		//if ((rect.x < 0) || (rect.width + rect.x > cap_len) || (rect.y < 0) || (rect.height + rect.y > cap_height)) {
		//  std::cerr << "RECT OUT OF RANGE" << std::endl;
		//return;
		//}
		if (rect.x >= 0 && rect.y >= 0) //&&
			//rect.x + rect.width <= framePtr->cols &&
			//rect.y + rect.height <= framePtr->rows)
		{
			//Взятие подматрицы из исходного изображения
			cv::Mat cropped = (*framePtr)(rect);
			//cv::Mat gray16bit;
			if (!cropped.empty()) {
				//cv::cvtColor(cropped, grayCropped, cv::COLOR_BGR2GRAY);
			//grayCropped.convertTo(gray16bit, CV_16U, 256.0);
				cv::imwrite("rect_image.png", cropped);
				corr_init(cropped);
				now_etal_im = cropped;
			}
			else {
				std::cerr << "EMPTY CROPPED IMAGE!" << std::endl;
			}
		}
	}
}


int main() {
	std::ofstream MyFile("filename11.txt");
	// Открываем камеру по умолчанию (обычно ID 0)
	//cv::VideoCapture cap("rtsp://root:nilaius@192.168.49.165:554/mpeg4/media.amp?resolution=640x480&fps=25");
	cv::VideoCapture cap(0);



	std::cout << "Current auto exposure mode: " << cap.get(cv::CAP_PROP_AUTO_EXPOSURE) << std::endl;
	//const int cap_len(320), cap_height(240);
	//std::cout << cap.get(cv::CAP_PROP_FRAME_WIDTH);
// Проверяем, удалось ли открыть
	if (!cap.isOpened()) {
		std::cerr << "Failed to open camera!" << std::endl;
		return -1;
	}
	std::cout << "Camera launched. Press ESC to exit." << std::endl;
	//std::cout << cap.get(cv::CAP_PROP_FRAME_WIDTH);
	cv::Mat frame;//(cap_len, cap_height, CV_8UC1);
	cv::Mat gray;//(cap_len, cap_height, CV_8UC1);//(cap_len, cap_height, CV_16UC1, cv::Scalar(0));
	//cv::Mat gray16bit(320, 240, CV_16UC1, cv::Scalar(0));

	if (fs::exists("rect_image.png")) {
		now_etal_im = cv::imread("rect_image.png", cv::IMREAD_UNCHANGED);
		corr_init(now_etal_im);
	}
	//corr_init(now_etal_im);
	cv::namedWindow("camera");
	cv::Mat frameOriginal; // Сохраняем оригинал для вырезки
	cv::setMouseCallback("camera", mouse_clbk, &frameOriginal);
	//cv::Mat now_etal_im = cv::imread("rect_image.png", cv::IMREAD_UNCHANGED);
	//corr_init(now_etal_im);
	while (true) {
		//cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
		//cv::namedWindow("камера");
	   // cv::resizeWindow("камера", cap_len, cap_height);
		cap >> frame; // Захват кадра
		if (frame.empty()) {
			std::cerr << "Empty frame!" << std::endl;
			break;
		}
		//frame = cv::imread("image.png");
		frame = frame(cv::Rect(0, 0, cap_len, cap_height));
		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);



		frameOriginal = gray.clone(); // Сохраняем оригинал для вырезки
		if (drawing) {
			// Рисуем прямоугольник при движении мыши
			cv::rectangle(gray, pt1, pt2, cv::Scalar(0, 255, 0), 0);
		}
		//Разностный алгоритм:
		//cv::Mat now_etal_im = cv::imread("rect_image.png", cv::IMREAD_UNCHANGED);
		if (!now_etal_im.empty()) {
			cv::Point point_true = corr_next(gray);
			//std::cout << point_true << std::endl;
			MyFile << point_true << std::endl;
			cv::rectangle(
				gray,
				point_true,
				cv::Point(point_true.x + now_etal_im.cols, point_true.y + now_etal_im.rows),
				cv::Scalar(255, 0, 0),
				2
			);
		}
		// Показываем кадр
		cv::imshow("camera", gray);
		if (cv::waitKey(10) == 27) break;
	}
	MyFile.close();
	cap.release();
	cv::destroyAllWindows();
	return 0;
}