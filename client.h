#ifndef __CLIENT_H__
#define __CLIENT_H__
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#define MAXDATASIZE 65536
typedef struct addrinfo addrinfo;
typedef struct timeval timeval;
class Client {
private:
  const char *hostname;
  const char *port;
  int error;
  int sockfd;
  const char *getHost(const char *hostname);

  int sendall(int fd, const char *buf, size_t *len);

public:
  Client(const char *h, const char *p);
  ~Client();
  std::vector<char> recvServeResponse();
  std::vector<char> recvall(int fd);
  void Send(const std::vector<char> &msg);
  int getError();
  int getFD();
};
#endif
