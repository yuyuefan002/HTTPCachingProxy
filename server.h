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
typedef struct addrinfo addrinfo;
typedef struct sockaddr_storage sockaddr_storage;
typedef struct sockaddr sockaddr;
class Server {
private:
  const char *port;
  addrinfo host_info;
  int listener;

public:
  Server(const char *p);
  ~Server();
  int acceptNewConn();
};
#endif
