#include "log.h"
#include "server.h"
#include <iostream>
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
      // handleHTTPRequest();
      close(newfd);
      _exit(EXIT_SUCCESS);
    }
    close(newfd);
  }
}
