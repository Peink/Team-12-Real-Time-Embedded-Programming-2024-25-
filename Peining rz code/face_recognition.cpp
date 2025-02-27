#include "face_recognition.h"
#include <iostream>

CascadeClassifier face_cascade;

bool loadFaceCascade(const std::string &cascadePath) {
    return face_cascade.load(cascadePath);
}

void recognizeFace(VideoCapture &cap) {
    Mat frame, gray;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;
        cvtColor(frame, gray, COLOR_BGR2GRAY);
        std::vector<Rect> faces;
        face_cascade.detectMultiScale(gray, faces);
        for (auto &face : faces) {
            rectangle(frame, face, Scalar(255, 0, 0), 2);
        }
        imshow("Face Recognition", frame);
        if (waitKey(10) == 27) break;
    }
}
