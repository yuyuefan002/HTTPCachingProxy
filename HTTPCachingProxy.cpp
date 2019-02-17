#include "client.h"
#include "httparser.h"
#include "log.h"
#include "server.h"
#include <iostream>
// to debug multi-thread
// use"set follow-fork-mode-child"+main breakpoint
int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: HTTPCachingProxy <port>\n";
    exit(EXIT_FAILURE);
  }
  Server server(argv[1]);
  while (1) {
    int newfd = server.acceptNewConn();
    if (newfd < 0)
      std::cerr << "Fail to accept a new request\n";
    int pid;
    if ((pid = fork() == 0)) {
      std::cout << "new request" << std::endl;
      std::string HTTPRequest = server.receiveHTTPRequest(newfd);
      HTTParser httparser(HTTPRequest);
      std::string hostname = httparser.getHostName();
      std::string port = httparser.getHostPort();
      Client client(
          hostname.c_str(),
          port.c_str()); // have to check success or not, if failed, return 503
      client.sendData(httparser.getRequest());
      std::string HTTPResponse = client.receiveHTTP();
      server.sendData(newfd, HTTPResponse);
      close(newfd);
      return EXIT_SUCCESS; // we could not use exit here, because resources
      // cannot be released gracefully.
    }
    close(newfd);
  }
  return EXIT_SUCCESS;
}
