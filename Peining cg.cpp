#include <opencv2/opencv.hpp> // OpenCV library for computer vision processing
#include <opencv2/face.hpp> // OpenCV face recognition library
#include <wiringPi.h> // For controlling Raspberry Pi GPIO
#include <iostream> // Standard input/output
#include <vector> // Vector container
#include <filesystem> // Handling filesystem operations
#include <fstream> // Reading and writing files
#include <ctime> // Time handling
#include <sstream> // Handling string streams
#include <thread> // Multithreading support
#include <mosquitto.h> // MQTT library for remote communication
#include <curl/curl.h> // For HTTP requests and notifications
#include <sqlite3.h> // SQLite database support
#include <httplib.h> // Web server support

#define LOCK_PIN 7 // GPIO pin for controlling the door lock
#define LOG_FILE "access_log.txt" // Access log file
#define IMAGE_LOG_PATH "access_images/" // Directory for storing access images
#define USER_IMAGE_PATH "user_images/" // Directory for storing user photos
#define CONFIDENCE_THRESHOLD 35.0 // Confidence threshold
#define UNLOCK_DURATION 2000 // Door unlock duration (milliseconds)
#define FRAME_SKIP 5 // Perform face detection every N frames for efficiency
#define MQTT_BROKER "mqtt://broker.hivemq.com" // MQTT server address
#define MQTT_TOPIC "smartlock/control" // MQTT topic
#define ALERT_EMAIL "your_email@example.com" // Alert email address
#define DB_FILE "smartlock.db" // Database file

using namespace cv;
using namespace cv::face;
using namespace std;
using namespace httplib;

Ptr<FaceRecognizer> model; // Face recognition model
CascadeClassifier face_cascade; // Face detection classifier
mosquitto *mosq; // MQTT client
sqlite3 *db; // SQLite database handle

// Initialize the database and create necessary tables
void initializeDatabase() {
    if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
        cerr << "Failed to open database" << endl;
        exit(EXIT_FAILURE);
    }
    string sql = "CREATE TABLE IF NOT EXISTS access_log (id INTEGER PRIMARY KEY, time TEXT, label INTEGER, confidence REAL, image_path TEXT);";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
    sql = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT, label INTEGER UNIQUE, image_path TEXT);";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
}

// Handle user photo upload and automatically link to the user record
void uploadUserImage(const Request& req, Response& res) {
    if (req.has_file("image")) {
        auto file = req.get_file_value("image");
        string filename = USER_IMAGE_PATH + file.filename;
        ofstream ofs(filename, ios::binary);
        ofs.write(file.content.data(), file.content.size());
        ofs.close();
        
        size_t underscore = file.filename.find("_");
        if (underscore != string::npos) {
            int label = stoi(file.filename.substr(0, underscore));
            string sql = "UPDATE users SET image_path = '" + filename + "' WHERE label = " + to_string(label) + ";";
            sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
            res.set_content("Image uploaded and linked successfully: " + filename, "text/plain");
        } else {
            res.set_content("Invalid filename format. Use label_name.jpg", "text/plain");
        }
    } else {
        res.set_content("No image uploaded.", "text/plain");
    }
}

// Start the web server to provide log viewing, user management, and remote unlocking
void startWebServer() {
    Server svr;
    
    svr.Get("/manage_users", [](const Request &, Response &res) {
        string page = "<html><body>"
                      "<h1>Manage Users</h1>"
                      "<form action='/upload_image' method='post' enctype='multipart/form-data'>"
                      "Upload Image (Format: label_name.jpg): <input type='file' name='image'><br>"
                      "<input type='submit' value='Upload Image'>"
                      "</form>"
                      "<h2>Registered Users</h2><ul>";
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "SELECT name, label, image_path FROM users;", -1, &stmt, NULL);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            page += "<li>" + string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))) +
                    " (Label: " + to_string(sqlite3_column_int(stmt, 1)) + ") - " +
                    "<img src='" + string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))) + "' width='100'></li>";
        }
        sqlite3_finalize(stmt);
        page += "</ul></body></html>";
        res.set_content(page, "text/html");
    });
    
    svr.Post("/upload_image", uploadUserImage);
    svr.listen("0.0.0.0", 8080);
}

// Main program entry point
int main(int argc, char** argv) {
    wiringPiSetup();
    pinMode(LOCK_PIN, OUTPUT);
    digitalWrite(LOCK_PIN, LOW);
    if (!face_cascade.load("haarcascade_frontalface_default.xml")) {
        cerr << "Error loading face cascade." << endl;
        return EXIT_FAILURE;
    }
    filesystem::create_directory(IMAGE_LOG_PATH);
    filesystem::create_directory(USER_IMAGE_PATH);
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