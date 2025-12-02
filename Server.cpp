#include <cstdio>
#include "httplib.h"
#include <iostream>

using namespace httplib;
using namespace std;

string dump_headers(const Headers &headers) {
  string s;
  char buf[BUFSIZ];

  for (const auto &x : headers) {
    snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
    s += buf;
  }

  return s;
}

string dump_multipart_formdata(const MultipartFormData &form) {
  string s;
  char buf[BUFSIZ];

  s += "--------------------------------\n";

  for (const auto &x : form.fields) {
    const auto &name = x.first;
    const auto &field = x.second;

    snprintf(buf, sizeof(buf), "name: %s\n", name.c_str());
    s += buf;

    snprintf(buf, sizeof(buf), "text length: %zu\n", field.content.size());
    s += buf;

    s += "----------------\n";
  }

  for (const auto &x : form.files) {
    const auto &name = x.first;
    const auto &file = x.second;

    snprintf(buf, sizeof(buf), "name: %s\n", name.c_str());
    s += buf;

    snprintf(buf, sizeof(buf), "filename: %s\n", file.filename.c_str());
    s += buf;

    snprintf(buf, sizeof(buf), "content type: %s\n", file.content_type.c_str());
    s += buf;

    snprintf(buf, sizeof(buf), "text length: %zu\n", file.content.size());
    s += buf;

    s += "----------------\n";
  }

  return s;
}

string log(const Request &req, const Response &res) {
  string s;
  char buf[BUFSIZ];

  s += "================================\n";

  snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
           req.version.c_str(), req.path.c_str());
  s += buf;

  string query;
  for (auto it = req.params.begin(); it != req.params.end(); ++it) {
    const auto &x = *it;
    snprintf(buf, sizeof(buf), "%c%s=%s",
             (it == req.params.begin()) ? '?' : '&', x.first.c_str(),
             x.second.c_str());
    query += buf;
  }
  snprintf(buf, sizeof(buf), "%s\n", query.c_str());
  s += buf;

  s += dump_headers(req.headers);
  s += dump_multipart_formdata(req.form);

  s += "--------------------------------\n";

  snprintf(buf, sizeof(buf), "%d\n", res.status);
  s += buf;
  s += dump_headers(res.headers);

  return s;
}

int main(int argc, const char **argv) {
  if (argc > 1 && string("--help") == argv[1]) {
    cout << "usage: Server [PORT] [DIR]" << endl;
    return 1;
  }

  Server server;

  server.Post("/multipart", [](const Request &req, Response &res) {
    auto body = dump_headers(req.headers) + dump_multipart_formdata(req.form);
    res.set_content(body, "text/plain");
  });

  server.set_error_handler([](const Request & /*req*/, Response &res) {
    const char *fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
    char buf[BUFSIZ];
    snprintf(buf, sizeof(buf), fmt, res.status);
    res.set_content(buf, "text/html");
  });

  server.set_logger([](const Request &req, const Response &res) { cout << log(req, res); });

  auto port = 8080;
  if (argc > 1) { port = atoi(argv[1]); }

  auto base_dir = "./";
  if (argc > 2) { base_dir = argv[2]; }

  if (!server.set_mount_point("/", base_dir)) {
    cout << "The specified base directory doesn't exist...";
    return 1;
  }

  cout << "The server started at port " << port << "..." << endl;

  server.listen("localhost", port);

  return 0;
}
