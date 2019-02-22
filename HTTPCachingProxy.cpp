#include "proxy.h"
#include <iostream>
#include <thread>
#define THREADNUM 100
// to debug multi-thread
// use"set follow-fork-mode-child"+main breakpoint
void proxy_func(int newfd) {
  std::cout << "new thread\n";
  Proxy proxy;
  proxy.handler(newfd);
  close(newfd);
}
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
  std::thread threads[THREADNUM];
  int i = 0;
  Proxy proxy(argv[1]);
  while (1) {
    int newfd = proxy.accNewRequest();
    threads[i++] = std::thread(proxy_func, newfd);
    if (i >= THREADNUM) {
      for (i = 0; i < THREADNUM; i++) {
        threads[i].join();
      }
      i = 0;
    }
  }
  /*
  Proxy proxy(argv[1]);
  while (1) {
    int newfd = proxy.accNewRequest();
    int pid;
    if ((pid = fork() == 0)) {
      proxy.handler(newfd);
      close(newfd);
      return EXIT_SUCCESS; // we could not use exit here, because resources
      // cannot be released gracefully.
    }
    close(newfd);
  }*/
  return EXIT_SUCCESS;
}
