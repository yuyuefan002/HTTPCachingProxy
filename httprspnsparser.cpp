#include "httprspnsparser.h"
/*
 * warning: this function will motify original string
 * pass unit test
 */
std::string trimLeadingSpace(std::string &msg) {
  size_t target = msg.find_first_not_of(' ');
  return msg.substr(target);
}
// pass unit test
std::string fetchNextSeg(std::string &msg, char c = ' ', size_t substrlen = 1) {
  msg = trimLeadingSpace(msg);
  size_t target = msg.find(c);
  std::string res = msg.substr(0, target);
  if (target != std::string::npos)
    msg = msg.substr(target + substrlen);
  else
    msg = "";

  return res;
}
std::string tolower(const std::string &msg) {
  std::string res;
  for (auto c : msg) {
    res += std::tolower(c);
  }
  return res;
}
void HTTPRSPNSParser::parseStatus(std::string statusline) {
  protocol = fetchNextSeg(statusline, '/');
  version_major = stoi(fetchNextSeg(statusline, '.'));
  version_minor = stoi(fetchNextSeg(statusline));
  status_code = stoi(fetchNextSeg(statusline));
  status_text = statusline;
}
void HTTPRSPNSParser::parseHeader(std::string head) {
  while (!head.empty()) {
    std::string key = tolower(fetchNextSeg(head, ':'));
    std::string value = fetchNextSeg(head, '\r', 2);
    headers[key] = value;
  }
}
size_t HTTPTimeRange2Num(std::string end, std::string start) {
  size_t age;
  return age;
}
/*
 * Description: Calcuelate the lifttime of a response
 *              first check "Cache-Control:max-age=N",
 *              if None, second check "Expires",
 *              if None, third check "Last-Modified".
 */
size_t HTTPRSPNSParser::calculateAge() {
  size_t age = 0;
  if (headers.find("cache-control") != headers.end()) {
    size_t target;
    if ((target = headers["cache-control"].find("max-age")) !=
        std::string::npos) {
      int i;
      for (i = target; headers["cache-control"][i] != ' '; i++) {
      }
      age = stoi(headers["cache-control"].substr(target + 9, i));
    }
  } else if (headers.find("expires") != headers.end() &&
             headers.find("date") != headers.end()) {
    age = HTTPTimeRange2Num(headers["expires"], headers["date"]);
  } else if (headers.find("last-modified") != headers.end() &&
             headers.find("date") != headers.end()) {
    size_t age = HTTPTimeRange2Num(headers["date"], headers["last-modified"]);
    age /= 10;
  }
  return age;
}
HTTPRSPNSParser::HTTPRSPNSParser(std::string response) {
  int target = response.find("\r\n");
  std::string statusline = response.substr(0, target);
  parseStatus(statusline);
  int head_end = response.find("\r\n\r\n");
  std::string head = response.substr(target + 2, head_end - target - 2);
  parseHeader(head);
}

size_t HTTPRSPNSParser::getStatusCode() { return status_code; }
bool HTTPRSPNSParser::good4Cache() {
  if (headers.find("cache-control") == headers.end()) {
    return true;
  }
  std::string ctlPolicy = headers["cache-control"];
  if (ctlPolicy.find("private") || ctlPolicy.find("no-cache") ||
      ctlPolicy.find("no-store")) {
    return false;
  }
  return true;
}
