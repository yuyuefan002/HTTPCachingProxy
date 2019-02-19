#include "httprspnsparser.h"
// pass unit test
void HTTPRSPNSParser::parseStatus(std::string statusline) {
  protocol = helper.fetchNextSeg(statusline, '/');
  version_major = stoi(helper.fetchNextSeg(statusline, '.'));
  version_minor = stoi(helper.fetchNextSeg(statusline));
  status_code = stoi(helper.fetchNextSeg(statusline));
  status_text = statusline;
}
// pass unit test
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
 * status: no problem until now
 */
size_t HTTPRSPNSParser::getMaxAge() {
  size_t age = 0;
  if (headers.find("cache-control") != headers.end()) {
    size_t target;
    if ((target = headers["cache-control"].find("s-maxage")) !=
        std::string::npos) {
      size_t i;
      std::string age_str = headers["cache-control"].substr(target);
      for (i = 0; age_str[i] != ' ' && i < age_str.length(); i++) {
      }
      age = stoi(age_str.substr(9, i));
      return age;
    } else if ((target = headers["cache-control"].find("max-age")) !=
               std::string::npos) {
      size_t i;
      std::string age_str = headers["cache-control"].substr(target);
      for (i = 0; age_str[i] != ' ' && i < age_str.length(); i++) {
      }
      age = stoi(age_str.substr(8, i));
      return age;
    }
  }
  if (headers.find("expires") != headers.end() &&
      headers.find("date") != headers.end()) {
    age = helper.HTTPTimeRange2Num(headers["expires"], headers["date"]);
  } else if (headers.find("last-modified") != headers.end() &&
             headers.find("date") != headers.end()) {
    age = helper.HTTPTimeRange2Num(headers["date"], headers["last-modified"]);
    age /= 10;
  }
  return age;
}

// pass unit test
// if this go wrong, check helper.HTTPAge
size_t HTTPRSPNSParser::getAge() {
  size_t age = 0;
  age = helper.HTTPAge(headers["date"]);
  return age;
}
// pass unit test
void HTTPRSPNSParser::updateAgeField() {
  size_t target;
  if ((target = HTTPResponse.find("Age:")) != std::string::npos) {
    helper.deleteALine(HTTPResponse, target);
  }

  std::stringstream ss;
  ss << "Age: " << getAge() << "\r\n";
  std::string Age = ss.str();
  target = HTTPResponse.find("\r\n\r\n");
  HTTPResponse.insert(target + 2, Age);
}
// pass unit test
HTTPRSPNSParser::HTTPRSPNSParser(std::string response) {
  if (response.empty())
    throw std::string("root dir");
  HTTPResponse = response;
  int target = HTTPResponse.find("\r\n");
  std::string statusline = HTTPResponse.substr(0, target);
  parseStatus(statusline);
  int head_end = HTTPResponse.find("\r\n\r\n");
  std::string head = HTTPResponse.substr(target + 2, head_end - target - 2);
  parseHeader(head);
}

size_t HTTPRSPNSParser::getStatusCode() { return status_code; }
// pass unit test
bool HTTPRSPNSParser::good4Cache() {
  if (headers.find("cache-control") == headers.end()) {
    return true;
  }
  std::string ctlPolicy = headers["cache-control"];
  if (ctlPolicy.find("private") != std::string::npos ||
      ctlPolicy.find("no-cache") != std::string::npos ||
      ctlPolicy.find("no-store") != std::string::npos) {
    return false;
  }
  return true;
}
// pass unit test
bool HTTPRSPNSParser::stillfresh() {
  size_t maxage = getMaxAge();
  size_t age = getAge();
  return maxage >= age;
}
std::string HTTPRSPNSParser::getResponse() {
  updateAgeField();
  return HTTPResponse;
}
/*
int main() {
  HTTPRSPNSParser httprspnsparser(
      "HTTP/1.1 200 OK\r\nServer: nginx/1.14.1\r\nDate: Mon, 18 Feb 2019 "
      "20:55:11 GMT\r\nContent-Type: text/plain\r\nContent-Length: "
      "2360\r\nConnection: keep-alive\r\nLast-Modified: Fri, 08 Feb 2019 "
      "18:43:41 GMT\r\nAccept-Ranges: bytes\r\nCache-Control: "
      "public public\r\nExpires: Mon, 25 Feb 2019 20:55:11 "
      "GMT\r\nVary: "
      "Accept-Encoding\r\nX-Endurance-Cache-Level: 4\r\n\r\nText");
  // std::cout << httprspnsparser.good4Cache();
  std::cout << httprspnsparser.getResponse();
  std::cout << httprspnsparser.getStatusCode();
}
*/
