#include "proxy.h"
#include <iostream>
// to debug multi-thread
// use"set follow-fork-mode-child"+main breakpoint
int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: HTTPCachingProxy <port>\n";
    exit(EXIT_FAILURE);
  }
  /* daemon(0, 0);
  umask(0);
  pid_t pid = fork();
  if (pid < 0) {
    return EXIT_FAILURE;
  }
  if (pid > 0) {
    return EXIT_SUCCESS;
    }*/
  Proxy proxy(argv[1]);
  while (1) {
    int newfd = proxy.accNewRequest();
    // int pid;
    // if ((pid = fork() == 0)) {
    proxy.handler(newfd);
    // close(newfd);
    // return EXIT_SUCCESS; // we could not use exit here, because resources
    // cannot be released gracefully.
    //}
    close(newfd);
  }
  return EXIT_SUCCESS;
}
