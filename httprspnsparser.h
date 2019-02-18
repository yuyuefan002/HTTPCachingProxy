#ifndef __HTTPRSPNSPARSER_H__
#define __HTTPRSPNSPARSER_H__
#include "helper.h"
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <time.h>
class HTTPRSPNSParser {
private:
  Helper helper;
  std::string HTTPResponse;
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
  HTTPRSPNSParser(std::string response);
  size_t getStatusCode();
  bool good4Cache();
  bool stillfresh();
  std::string getResponse();
};
#endif
