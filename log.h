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
  void contactServer();
  void hereFromServer();
  void respondClient();
  void closeTunnel();

  Log(int id);
  ~Log();
};
#endif
