#include "proxy.h"
#include <iostream>
#include <thread>
#define THREADNUM 100
// to debug multi-thread
// use"set follow-fork-mode-child"+main breakpoint
//<<<<<< HEAD
void proxy_func(std::pair<int, int> *args) {
  std::cout << "new thread\n";
  int newfd = args->first;
  int requestid = args->second;
  Proxy proxy(requestid);
  /*
  =======
void proxy_func(int newfd) {
  Proxy proxy;
>>>>>>> 3637008686cedc19dd3b6c286b162045b4fad4dd

  */
  proxy.handler(newfd);
  close(newfd);
}
int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: HTTPCachingProxy <port>\n";
    exit(EXIT_FAILURE);
  }
  //<<<<<<< HEAD
  // std::thread threads[THREADNUM];
  // daemon(0, 0);

  std::thread threads[THREADNUM];
  int i = 0;
  int requestid = 0;
  Proxy proxy(argv[1]);
  while (1) {
    int newfd = proxy.accNewRequest();
    std::pair<int, int> *args = new std::pair<int, int>(newfd, requestid++);
    threads[i++] = std::thread(proxy_func, args);
    if (i >= THREADNUM) {
      for (i = 0; i < THREADNUM; i++) {
        threads[i].join();
      }
      i = 0;
    }
  }
  /*
  Proxy proxy(argv[1]);
=======
  //  std::thread threads[THREADNUM];
  // int i = 0;
  Proxy proxy(argv[1]);
  while (1) {
    try {
      int newfd = proxy.accNewRequest();
      std::thread t = std::thread(proxy_func, newfd);
      t.detach();
    } catch (std::string e) {
    }
  }
  */
  /* Proxy proxy(argv[1]);
>>>>>>> 3637008686cedc19dd3b6c286b162045b4fad4dd
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
<<<<<<< HEAD
  }
  */
  //=======

  //>>>>>>> 3637008686cedc19dd3b6c286b162045b4fad4dd
  return EXIT_SUCCESS;
}
