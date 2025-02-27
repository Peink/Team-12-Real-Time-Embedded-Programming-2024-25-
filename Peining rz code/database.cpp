#include "database.h"
#include <iostream>
#include <filesystem>

sqlite3 *db;

void initializeDatabase() {
    if (sqlite3_open("smartlock.db", &db) != SQLITE_OK) {
        std::cerr << "Failed to open database" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::string sql = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT, label INTEGER UNIQUE, image_path TEXT);";
    sqlite3_exec(db, sql.c_str(), NULL, NULL, NULL);
}

void createDirectories() {
    std::filesystem::create_directory("user_images");
    std::filesystem::create_directory("access_images");
}
