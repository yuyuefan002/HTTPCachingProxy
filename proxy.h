#ifndef __PROXY_H__
#define __PROXY_H__
#include "cache.h"
#include "client.h"
#include "helper.h"
#include "http.h"
#include "httparser.h"
#include "httprspnsparser.h"
#include "log.h"
#include "server.h"
#define NOTINCACHE 1
#define EXPIRED 2
#define NEEDVALIDATE 3
#define VALID 4
typedef struct sockaddr sockaddr;
class Proxy {
private:
  Server server;
  Helper helper;
  Log log;
  std::vector<char> handlebyCache(Cache &cache, HTTParser &httparser);
  std::vector<char> fetchNewResponse(Cache &cache, HTTParser &httparser,
                                     const std::vector<char> &request);
  std::vector<char> revalidation(HTTPRSPNSParser &httprspnsparser,
                                 HTTParser &httparser, Cache &cache);
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
