#ifndef __PROXY_H__
#define __PROXY_H__
#include "cache.h"
#include "client.h"
#include "http.h"
#include "httparser.h"
#include "httprspnsparser.h"
#include "log.h"
#include "server.h"
#include <iostream>
#define NOTINCACHE 1
#define EXPIRED 2
#define NEEDVALIDATE 3
#define VALID 4
typedef struct sockaddr sockaddr;
class Proxy {
private:
  Server server;
  Log log;
  std::vector<char> handlebyCache(Cache &cache, HTTParser &httparser);
  std::vector<char> fetchNewResponse(Cache &cache, HTTParser &httparser);
  void GET_handler(HTTParser &httparser, int newfd);
  void POST_handler(HTTParser &httparser, int newfd);
  void CONNECT_handler(HTTParser &httparser, int newfd);

public:
  Proxy(int requestid);
  Proxy(const char *port);
  ~Proxy();
  int accNewRequest();
  void handler(int newfd); //, int requestid);
};
#endif
