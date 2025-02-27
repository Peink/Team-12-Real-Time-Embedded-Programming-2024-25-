#include "web_server.h"
#include "database.h"
#include <httplib.h>
#include <sqlite3.h>
#include <iostream>

using namespace httplib;

void startWebServer() {
    Server svr;
    svr.Get("/manage_users", [](const Request &, Response &res) {
        res.set_content("<html><body><h1>User Management</h1></body></html>", "text/html");
    });
    svr.listen("0.0.0.0", 8080);
}
