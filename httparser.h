#ifndef __HTTPARSER_H__
#define __HTTPARSER_H__
#include "helper.h"
#include <iostream>
#include <map>
#include <string>
#include <vector>
class HTTParser {
private:
  Helper helper;
  int errnum;
  std::string HTTPRequest;
  std::vector<char> HTTPRequest_char;
  std::string method;
  std::string protocol;
  std::string path;
  std::string statusline;
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
  HTTParser(const std::vector<char> &r);
  ~HTTParser();
  int errorDetection();
  std::string getHostName();
  std::string getHostPort();
  std::vector<char> getRequest();
  std::string getMethod();
  std::string getURL();
  std::string getStatusLine();
  bool good4Cache();
};
#endif
