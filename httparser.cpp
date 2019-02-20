#include "httparser.h"
// pass unit test
std::string smartPort(std::string &msg) {
  size_t target;
  std::string port;
  if ((target = msg.find(':')) != std::string::npos) {
    port = msg.substr(target + 1);
    msg = msg.substr(0, target);
  } else
    port = "80";
  return port;
}
// pass unit test
void HTTParser::parseRequest(std::string request) {
  method = helper.fetchNextSeg(request);
  path = helper.fetchNextSeg(request);
  protocol = helper.fetchNextSeg(request, '/');
  version_major = stoi(helper.fetchNextSeg(request, '.'));
  version_minor = stoi(helper.fetchNextSeg(request));
}
// pass unit test
void HTTParser::parseHeader(std::string head) {
  while (!head.empty()) {
    std::string key = helper.tolower(helper.fetchNextSeg(head, ':'));
    std::string value = helper.fetchNextSeg(head, '\r', 2);
    headers[key] = value;
  }
}

bool HTTParser::good4Cache() {
  if (headers.find("cache-control") == headers.end()) {
    return true;
  }
  std::string ctlPolicy = headers["cache-control"];
  if (ctlPolicy.find("private") != std::string::npos ||
      ctlPolicy.find("no-cache") != std::string::npos ||
      ctlPolicy.find("no-store") != std::string::npos) {
    return false;
  }
  size_t target;
  if ((target = headers["cache-control"].find("max-age")) !=
      std::string::npos) {
    size_t i;
    std::string age_str = headers["cache-control"].substr(target);
    for (i = 0; age_str[i] != ' ' && i < age_str.length(); i++) {
    }
    int age = stoi(age_str.substr(8, i));
    if (age == 0)
      return false;
  }
  return true;
}
/*
 * status: incomplete
 * Errnum Mapping Table
 * errnum     meaning
 *      1     missing Host(fatal)
 */
int HTTParser::verifyHeader() {
  try {
    if (headers.find("host") == headers.end())
      throw std::string("host");
  } catch (std::string e) {
    std::cerr << "missing " << e << std::endl;
    errnum = 1;
    return -1;
  }
  return 0;
}
std ::string HTTParser::updateHTTPRequest(std::string request) {
  size_t target;
  size_t end = request.find("\r\n");
  if ((target = request.find("http://")) != std::string::npos) {
    request.replace(target, 7, "");
  }
  if ((target = request.find(host)) != std::string::npos && target < end) {
    request.replace(target, host.size(), "");
  }
  if ((target = request.find(":" + port)) != std::string::npos)
    request.replace(target, 1 + port.size(), "");
  end = request.find("\r\n");
  path = request.substr(0, end);
  path = updateHTTPath(path);
  return request;
}
/*
 * status: need more test
 * currently pass unit test
 */
std::string HTTParser::updateHTTPath(std::string &path) {
  helper.fetchNextSeg(path);
  path = helper.fetchNextSeg(path);
  if (path.find(host) == std::string::npos)
    path.insert(0, host);
  return path;
}
HTTParser::HTTParser(const std::vector<char> &r) : HTTPRequest(r.data()) {
  errnum = 0;
  int target = HTTPRequest.find("\r\n");
  std::string request = HTTPRequest.substr(0, target);
  parseRequest(request);
  int head_end = HTTPRequest.find("\r\n\r\n");
  std::string head = HTTPRequest.substr(target + 2, head_end - target - 2);
  parseHeader(head);
  if (verifyHeader() != -1) {

    port = smartPort(headers["host"]);
    host = headers["host"];
  }
  HTTPRequest = updateHTTPRequest(HTTPRequest);
}

HTTParser::~HTTParser() {}
/*
 * advice: run this function to detect error in HTTP request
 * reason: this class cannot make every request  valid,
 *         but I can detect them and supply solution
 *
 */
int HTTParser::errorDetection() { return errnum; }
std::string HTTParser::getHostName() { return host; }
std::string HTTParser::getHostPort() { return port; }
std::vector<char> HTTParser::getRequest() {
  std::vector<char> r(HTTPRequest.begin(), HTTPRequest.end());
  return r;
}
std::string HTTParser::getMethod() { return method; }
std::string HTTParser::getURL() { return path; } // valgrind clean
/*
int main() {
  std::string HTTPRequest =
      "GET http://rabihyounes.com/awesome.txt HTTP/1.1\r\nHost: "
      "rabihyounes.com\r\nUser-Agent:Feedburner\r\n\r\n";
  HTTParser httparser(HTTPRequest);
  std::cout << httparser.getRequest();
}
*/
