#include "proxy.h"

void Proxy::GET_handler(HTTParser &httparser) {
  Cache cache;
  std::string url = httparser.getURL();
  if (cache.check(url)) {
    std::string HTTPResponse = cache.read(url);
    std::cout << "in cache\n";
    HTTPRSPNSParser httprspnsparser(HTTPResponse);
    if (httprspnsparser.stillfresh()) {
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
  std::cout << httparser.getRequest();
  client.GET(httparser.getRequest());
  std::string HTTPResponse = client.recvGETResponse();

  HTTPRSPNSParser httprspnsparser(HTTPResponse);
  if (httprspnsparser.getStatusCode() == 200 && httprspnsparser.good4Cache())
    cache.store(url, HTTPResponse);
  server.sendData(newfd, HTTPResponse);
  std::cout << "new response send\n";
}
void Proxy::CONNECT_handler(HTTParser &httparser) { httparser.getHostName(); }
void Proxy::accNewRequest() {
  newfd = server.acceptNewConn();
  if (newfd < 0)
    std::cerr << "Fail to accept a new request\n";
}
void Proxy::handler() {
  std::string HTTPRequest = server.receiveHTTPRequest(newfd);
  std::cout << HTTPRequest;
  HTTParser httparser(HTTPRequest);
  if (httparser.getMethod() == "GET")
    GET_handler(httparser);
  else {
    //    CONNECT_handler(httparser);
  }
}
Proxy::Proxy(const char *port) : server(port) {}
Proxy::~Proxy() { close(newfd); }
