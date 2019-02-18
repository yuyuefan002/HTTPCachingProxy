#ifndef __HTTPRSPNSPARSER_H__
#define __HTTPRSPNSPARSER_H__
#include <map>
#include <string>
class HTTPRSPNSParser {
private:
  std::string HTTPResponse;
  std::string protocol;
  unsigned short version_major;
  unsigned short version_minor;
  size_t status_code;
  std::string status_text;
  std::map<std::string, std::string> headers;
  void parseStatus(std::string statusline);
  void parseHeader(std::string head);
  size_t calculateAge();

public:
  HTTPRSPNSParser(std::string response);
  ~HTTPRSPNSParser();
  size_t getStatusCode();
  bool good4Cache();
};
#endif
