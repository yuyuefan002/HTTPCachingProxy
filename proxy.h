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
  int newfd;
  Server server;
  void GET_handler(HTTParser &httparser);
  void CONNECT_handler(HTTParser &httparser);

public:
  Proxy(const char *port);
  ~Proxy();
  void accNewRequest();
  void handler();
};
#endif
