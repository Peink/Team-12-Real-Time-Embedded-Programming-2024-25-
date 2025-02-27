#ifndef FACE_RECOGNITION_H
#define FACE_RECOGNITION_H

#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
using namespace cv;
using namespace cv::face;

void recognizeFace(VideoCapture &cap);
bool loadFaceCascade(const std::string &cascadePath);

#endif // FACE_RECOGNITION_H