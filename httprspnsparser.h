#ifndef __HTTPRSPNSPARSER_H__
#define __HTTPRSPNSPARSER_H__
#include "helper.h"
#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
class HTTPRSPNSParser {
private:
  Helper helper;
  std::string HTTPResponse;
  std::vector<char> HTTPResponse_char;
  std::string protocol;
  unsigned short version_major;
  unsigned short version_minor;
  size_t status_code;
  std::string status_text;
  std::map<std::string, std::string> headers;
  void parseStatus(std::string statusline);
  void parseHeader(std::string head);
  size_t getMaxAge();
  void updateAgeField();
  size_t getAge();

public:
  HTTPRSPNSParser(std::vector<char> response);
  size_t getStatusCode();
  bool mustRevalidate();
  bool good4Cache();
  bool stillfresh();
  bool not_expire();
  std::vector<char> getResponse();
  std::string getLastModified();
  std::string getETag();
  std::vector<char> getStatusText();
  std::string expiresAt();
  std::string whyBad4Cache();
};
#endif
