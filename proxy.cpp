#include "proxy.h"

void Proxy::GET_handler(HTTParser &httparser, int newfd) {
  Cache cache;
  std::string url = httparser.getURL();
  if (cache.check(url)) {
    std::vector<char> HTTPResponse = cache.read(url);
    std::cout << "in cache\n";
    HTTPRSPNSParser httprspnsparser(HTTPResponse);
    if (httparser.good4Cache() && httprspnsparser.stillfresh()) {
      server.sendData(newfd, httprspnsparser.getResponse());
      close(newfd);
      return;
    }
  }
  std::string hostname = httparser.getHostName();
  std::string port = httparser.getHostPort();
  Client client(hostname.c_str(),
                port.c_str()); // have to check success or not, if failed,
                               // return 503,important
  client.GET(httparser.getRequest());
  std::vector<char> HTTPResponse = client.recvServeResponse();

  HTTPRSPNSParser httprspnsparser(HTTPResponse);
  if (httprspnsparser.getStatusCode() == 200 && httprspnsparser.good4Cache() &&
      httparser.good4Cache())
    cache.store(url, HTTPResponse);
  server.sendData(newfd, HTTPResponse);
  std::cout << "new response send\n";
}
void Proxy::CONNECT_handler(HTTParser &httparser) { httparser.getHostName(); }
int Proxy::accNewRequest() {
  int newfd = server.acceptNewConn();
  if (newfd < 0)
    std::cerr << "Fail to accept a new request\n";
  return newfd;
}
void Proxy::handler(int newfd) {
  std::vector<char> HTTPRequest = server.receiveHTTPRequest(newfd);
  std::cout << HTTPRequest.data();
  HTTParser httparser(HTTPRequest);
  if (httparser.getMethod() == "GET")
    GET_handler(httparser, newfd);
  else {
    //    CONNECT_handler(httparser);
  }
}
Proxy::Proxy(const char *port) : server(port) {}
Proxy::~Proxy() {}
