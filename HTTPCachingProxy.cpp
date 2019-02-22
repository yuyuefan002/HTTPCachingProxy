#include "proxy.h"
#include <iostream>
#include <thread>
#define THREADNUM 100
// to debug multi-thread
// use"set follow-fork-mode-child"+main breakpoint
void proxy_func(int newfd, int requestid) {
  std::cout << "new thread\n";
  Proxy proxy(requestid);
  proxy.handler(newfd);
  close(newfd);
}
int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: HTTPCachingProxy <port>\n";
    exit(EXIT_FAILURE);
  }
<<<<<<< HEAD
  // std::thread threads[THREADNUM];
  daemon(0, 0);
=======

  /* daemon(0, 0);
>>>>>>> c8e852f60a364c1dcd9d2725bc0220e4193df367
  umask(0);
  pid_t pid = fork();
  if (pid < 0) {
    return EXIT_FAILURE;
  }
  if (pid > 0) {
    return EXIT_SUCCESS;
<<<<<<< HEAD
  }
  /*  int i = 0;
=======
    }*/
  std::thread threads[THREADNUM];
  int i = 0;
  int requestid = 0;
  Proxy proxy(argv[1]);
  while (1) {
    int newfd = proxy.accNewRequest();
    threads[i++] = std::thread(proxy_func, newfd, requestid++);
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
      proxy.handler(newfd); //, requestid);
      close(newfd);
      return EXIT_SUCCESS; // we could not use exit here, because resources
      // cannot be released gracefully.
    }
    close(newfd);
  }
  */
  return EXIT_SUCCESS;
}
