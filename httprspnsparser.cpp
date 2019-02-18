#include "httprspnsparser.h"

void HTTPRSPNSParser::parseStatus(std::string statusline) {
  protocol = helper.fetchNextSeg(statusline, '/');
  version_major = stoi(helper.fetchNextSeg(statusline, '.'));
  version_minor = stoi(helper.fetchNextSeg(statusline));
  status_code = stoi(helper.fetchNextSeg(statusline));
  status_text = statusline;
}
void HTTPRSPNSParser::parseHeader(std::string head) {
  while (!head.empty()) {
    std::string key = helper.tolower(helper.fetchNextSeg(head, ':'));
    std::string value = helper.fetchNextSeg(head, '\r', 2);
    headers[key] = value;
  }
}

/*
 * Description: Calcuelate the lifttime of a response
 *              first check "Cache-Control:max-age=N",
 *              if None, second check "Expires",
 *              if None, third check "Last-Modified".
 */
size_t HTTPRSPNSParser::getMaxAge() {
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
    age = helper.HTTPTimeRange2Num(headers["expires"], headers["date"]);
  } else if (headers.find("last-modified") != headers.end() &&
             headers.find("date") != headers.end()) {
    age = helper.HTTPTimeRange2Num(headers["date"], headers["last-modified"]);
    age /= 10;
  }
  return age;
}
size_t HTTPRSPNSParser::getAge() {
  size_t age = 0;
  age = helper.HTTPAge(headers["date"]);
  return age;
}
void HTTPRSPNSParser::updateAgeField() {
  size_t target;
  if ((target = HTTPResponse.find("Age:")) != std::string::npos) {
    helper.deleteALine(HTTPResponse, target);
  }

  std::stringstream ss;
  ss << "Age: " << getAge() << "\r\n";
  std::string Age = ss.str();
  target = HTTPResponse.find("\r\n\r\n");
  HTTPResponse.insert(target, Age);
}
HTTPRSPNSParser::HTTPRSPNSParser(std::string response) {
  HTTPResponse = response;
  int target = HTTPResponse.find("\r\n");
  std::string statusline = HTTPResponse.substr(0, target);
  parseStatus(statusline);
  int head_end = HTTPResponse.find("\r\n\r\n");
  std::string head = HTTPResponse.substr(target + 2, head_end - target - 2);
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
