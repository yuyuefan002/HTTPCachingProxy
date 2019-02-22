#ifndef __LOG_H__
#define __LOG_H__
#include <arpa/inet.h>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
class Log {
private:
  int requestid;
  std::string getclientip(int newfd);
  void save(std::string msg);

public:
  void newRequest(std::string statusLine, std::string clientip);
  void checkCache();
  void reqFromServer(std::string statusLine, std::string serverName);
  void recvFromServer(std::string statusText, std::string serverName);
  void respondClient(std::vector<char> statusText);
  void closeTunnel();

  Log();
  Log(int id);
  ~Log();
};
#endif
