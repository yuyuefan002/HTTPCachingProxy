#include "httparser.h"
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
  method = fetchNextSeg(request);
  path = fetchNextSeg(request);
  // port = smartPort(path);
  protocol = fetchNextSeg(request, '/');
  version_major = stoi(fetchNextSeg(request, '.'));
  version_minor = stoi(fetchNextSeg(request));
}
// pass unit test
void HTTParser::parseHeader(std::string head) {
  while (!head.empty()) {
    std::string key = fetchNextSeg(head, ':');
    std::string value = fetchNextSeg(head, '\r', 2);
    headers[key] = value;
  }
}
/*
 * status: incomplete
 * Errnum Mapping Table
 * errnum     meaning
 *      1     missing Host(fatal)
 */
int HTTParser::verifyHeader() {
  try {
    if (headers.find("Host") == headers.end())
      throw std::string("Host");
  } catch (std::string e) {
    std::cerr << "missing " << e << std::endl;
    errnum = 1;
    return -1;
  }
  return 0;
}
std ::string HTTParser::updateHTTPRequest(std::string request) {
  size_t target;
  if ((target = request.find("http://")) != std::string::npos) {
    request.replace(target, 7, "");
  }
  if ((target = request.find(host)) != std::string::npos) {
    request.replace(target, host.size(), "");
  }
  if ((target = request.find(":" + port)) != std::string::npos)
    request.replace(target, 1 + port.size(), "");
  return request;
}
HTTParser::HTTParser(std::string r) : HTTPRequest(r) {
  errnum = 0;
  int target = HTTPRequest.find("\r\n");
  std::string request = HTTPRequest.substr(0, target);
  parseRequest(request);
  int head_end = HTTPRequest.find("\r\n\r\n");
  std::string head = HTTPRequest.substr(target + 2, head_end - target - 2);
  parseHeader(head);
  if (verifyHeader() != -1) {
    host = headers["Host"];
    port = smartPort(host);
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
std::string HTTParser::getRequest() { return HTTPRequest; }
// valgrind clean
/*
int main() {
  std::string HTTPRequest = "GET /awesome.txt HTTP/1.1\r\nHost: "
                            "rabihyounes.com\r\nUser-Agent:Feedburner\r\n\r\n";
  HTTParser httparser(HTTPRequest);
}
*/
