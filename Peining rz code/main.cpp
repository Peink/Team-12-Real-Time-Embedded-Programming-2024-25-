#include "face_recognition.h"
#include "database.h"
#include "web_server.h"
#include "gpio_control.h"
#include <opencv2/opencv.hpp>
#include <wiringPi.h>
#include <thread>
#include <iostream>

using namespace std;

int main(int argc, char** argv) {
    wiringPiSetup();
    pinMode(LOCK_PIN, OUTPUT);
    digitalWrite(LOCK_PIN, LOW);
    
    if (!loadFaceCascade("assets/haarcascade_frontalface_default.xml")) {
        cerr << "Error loading face cascade." << endl;
        return EXIT_FAILURE;
    }
    
    createDirectories();
    initializeDatabase();
    thread webThread(startWebServer);
    webThread.detach();
    
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Failed to open camera!" << endl;
        return EXIT_FAILURE;
    }
    
    recognizeFace(cap);
    
    sqlite3_close(db);
    return EXIT_SUCCESS;
}
