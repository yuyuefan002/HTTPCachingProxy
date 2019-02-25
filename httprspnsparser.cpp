#include "httprspnsparser.h"

/*
 * parseStatus
 * This function parse the status line
 *
 * Test Status: pass unit test
 */
void HTTPRSPNSParser::parseStatus(std::string statusline) {
  protocol = helper.fetchNextSeg(statusline, '/');
  version_major = stoi(helper.fetchNextSeg(statusline, '.'));
  version_minor = stoi(helper.fetchNextSeg(statusline));
  status_code = stoi(helper.fetchNextSeg(statusline));
  status_text = statusline;
}

/*
 * parseHeader
 * This function parse the header
 *
 * Test Status: pass unit test
 */
void HTTPRSPNSParser::parseHeader(std::string head) {
  while (!head.empty()) {
    std::string key = helper.tolower(helper.fetchNextSeg(head, ':'));

    std::string value = helper.fetchNextSeg(head, '\r', 2);

    headers[key] = value;
  }
}

/*
 * getMaxage
 * This function get the Max lifttime of a cached http response
 * Description: Calcuelate the lifttime of a response
 *              first check "Cache-Control:max-age=N",
 *              if None, second check "Expires",
 *              if None, third check "Last-Modified".
 * Test Status: no problem until now
 */
size_t HTTPRSPNSParser::getMaxAge() {
  size_t age = 0;
  // check "cache-control"
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

  // check "expires"
  if (headers.find("expires") != headers.end() &&
      headers.find("date") != headers.end()) {
    age = helper.HTTPTimeRange2Num(headers["expires"], headers["date"]);
  }
  // infer a lifetime
  else if (headers.find("last-modified") != headers.end() &&
           headers.find("date") != headers.end()) {
    age = helper.HTTPTimeRange2Num(headers["date"], headers["last-modified"]);
    age /= 10;
  }
  return age;
}

/*
 * getAge
 * This function get the age of cached HTTP response
 *
 * Test Status: pass unit test
 * if this go wrong, check helper.HTTPAge
 */
size_t HTTPRSPNSParser::getAge() {
  size_t age = 0;
  age = helper.HTTPAge(headers["date"]);
  return age;
}

/*
 * updateAgeField
 * This function update the Age field in cached HTTP response
 *
 * Test Status: pass unit test
 */
void HTTPRSPNSParser::updateAgeField() {

  // delete old Age string
  std::vector<char> pattern = {'A', 'g', 'e', ':'};
  auto begin = std::search(HTTPResponse_char.begin(), HTTPResponse_char.end(),
                           pattern.begin(), pattern.end());
  if (begin != HTTPResponse_char.end())
    HTTPResponse_char = helper.deleteALine(HTTPResponse_char, begin);

  // generate new Age string
  std::stringstream ss;
  ss << "Age: " << getAge() << "\r\n";
  std::string Age = ss.str();

  // insert new Age string
  pattern = {'\r', '\n', '\r', '\n'};
  auto it = std::search(HTTPResponse_char.begin(), HTTPResponse_char.end(),
                        pattern.begin(), pattern.end());
  it += 2;
  HTTPResponse_char.insert(it, Age.begin(), Age.end());
}

/*
 * initializer
 * This function will parse all data
 *
 * Test Status: pass unit test
 */

HTTPRSPNSParser::HTTPRSPNSParser(std::vector<char> response) {
  if (response.empty())
    throw std::string("no response");
  HTTPResponse = response.data();
  HTTPResponse_char = response;
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

std::string HTTPRSPNSParser::whyBad4Cache() {
  std::string ctlPolicy = headers["cache-control"];
  if (ctlPolicy.find("private") != std::string::npos)
    return "private policy of http response";
  if (ctlPolicy.find("no-cache") != std::string::npos)
    return "no-cache policy of http response";
  if (ctlPolicy.find("no-store") != std::string::npos)
    return "no-store policy of http response";
  return "max cache time is 0";
}

bool HTTPRSPNSParser::not_expire() {
  size_t maxage = getMaxAge();
  size_t age = getAge();
  return maxage > age;
}

bool HTTPRSPNSParser::mustRevalidate() {
  std::string ctlPolicy = headers["cache-control"];
  if (ctlPolicy.find("must-revalidate") != std::string::npos)
    return true;
  return false;
}

// pass unit test
bool HTTPRSPNSParser::stillfresh() { return not_expire() && !mustRevalidate(); }

std::vector<char> HTTPRSPNSParser::getResponse() {
  updateAgeField();
  return HTTPResponse_char;
}

std::string HTTPRSPNSParser::getLastModified() {
  if (headers.find("last-modified") == headers.end())
    return std::string();
  return headers["last-modified"];
}

std::string HTTPRSPNSParser::getETag() {
  if (headers.find("etag") == headers.end())
    return std::string();
  return headers["etag"];
}

std::vector<char> HTTPRSPNSParser::getStatusText() {
  return std::vector<char>(status_text.begin(), status_text.end());
}

std::string HTTPRSPNSParser::expiresAt() {
  struct tm born = helper.strtotm(headers["date"]);
  struct tm expires = born;
  expires.tm_sec += getMaxAge();
  time_t t = mktime(&expires);
  char *time = ctime(&t);
  return std::string(time);
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
