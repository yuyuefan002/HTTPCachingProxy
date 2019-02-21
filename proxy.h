#ifndef __PROXY_H__
#define __PROXY_H__
#include "cache.h"
#include "client.h"
#include "httparser.h"
#include "httprspnsparser.h"
#include "log.h"
#include "server.h"
#include <iostream>
class Proxy {
private:
  Server server;
  void GET_handler(HTTParser &httparser, int newfd);
  void POST_handler(HTTParser &httparser, int newfd);
  void CONNECT_handler(HTTParser &httparser, int newfd);

public:
  Proxy(const char *port);
  ~Proxy();
  int accNewRequest();
  void handler(int newfd);
};
#endif
