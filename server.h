#ifndef __SERVER_H__
#define __SERVER_H__
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define MAXDATASIZE 100
typedef struct addrinfo addrinfo;
typedef struct sockaddr_storage sockaddr_storage;
typedef struct sockaddr sockaddr;
class Server {
private:
  const char *port;
  addrinfo host_info;
  int listener;
  std::string recvall(int fd);
  int sendall(int fd, const char *buf, size_t *len);

public:
  Server(const char *p);
  ~Server();
  int acceptNewConn();
  std::string receiveHTTPRequest(int fd);
  void sendData(int fd, std::string msg);
};
#endif
