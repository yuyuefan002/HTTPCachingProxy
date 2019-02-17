#ifndef __HTTPARSER_H__
#define __HTTPARSER_H__
#include <iostream>
#include <map>
#include <string>
class HTTParser {
private:
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

public:
  HTTParser(std::string r);
  ~HTTParser();
  int errorDetection();
  std::string getHostName();
  std::string getHostPort();
};
#endif
