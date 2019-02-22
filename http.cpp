#include "http.h"
std::vector<char> HTTP200() {
  std::string r = "HTTP/1.1 200 OK\r\n\r\n";
  std::vector<char> HTTP200(r.begin(), r.end());
  return HTTP200;
}
std::vector<char> HTTP400() {
  std::string r = "HTTP/1.1 400 Bad Request\r\n\r\n";
  std::vector<char> HTTP400(r.begin(), r.end());
  return HTTP400;
}
std::vector<char> HTTP502() {
  std::string r = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
  std::vector<char> HTTP502(r.begin(), r.end());
  return HTTP502;
}

std::vector<char> HTTP503() {
  std::string r = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
  std::vector<char> HTTP503(r.begin(), r.end());
  return HTTP503;
}
