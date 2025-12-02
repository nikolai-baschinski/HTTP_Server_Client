#include "httplib.h"
#include <iostream>

using namespace std;

int main(void) {

  httplib::Client client("localhost", 8080);

  if (auto res = client.Get("/hi/hi.txt")) {
    cout << res->status << endl;
    cout << res->get_header_value("Content-Type") << endl;
    cout << res->body << endl;
  } else {
    cout << "error code: " << res.error() << std::endl;
  }

  return 0;
}
