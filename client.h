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
#define MAXDATASIZE 100
typedef struct addrinfo addrinfo;
typedef struct timeval timeval;
class Client {
private:
  const char *hostname;
  const char *port;
  int sockfd;
  const char *getHost(const char *hostname);
  std::string recvall(int fd);
  int sendall(int fd, const char *buf, size_t *len);

public:
  Client(const char *h, const char *p);
  ~Client();
  std::string receiveHTTP();
  void sendData(std::string msg);
};
#endif