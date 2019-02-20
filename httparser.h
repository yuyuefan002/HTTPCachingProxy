#ifndef __HTTPARSER_H__
#define __HTTPARSER_H__
#include "helper.h"
#include <iostream>
#include <map>
#include <string>
class HTTParser {
private:
  Helper helper;
  int errnum;
  std::string HTTPRequest;
  std::string method;
  std::string protocol;
  std::string path;
  unsigned short version_major;
  unsigned short version_minor;
  std::string host;
  std::string port;
  std::map<std::string, std::string> headers;
  void parseRequest(std::string request);
  void parseHeader(std::string head);
  int verifyHeader();
  std::string updateHTTPath(std::string &path);
  std::string updateHTTPRequest(std::string request);

public:
  HTTParser(std::string r);
  ~HTTParser();
  int errorDetection();
  std::string getHostName();
  std::string getHostPort();
  std::string getRequest();
  std::string getMethod();
  std::string getURL();
  bool good4Cache();
};
#endif
