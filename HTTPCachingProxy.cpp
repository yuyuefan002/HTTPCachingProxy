#include "proxy.h"
#include <iostream>
#include <thread>
// to debug multi-thread
// use"set follow-fork-mode-child"+main breakpoint
void createIndex(std::string path) {
  std::string dir;
  Helper helper;
  while (!path.empty()) {
    dir += helper.fetchNextSeg(path, '/');
    if (!path.empty()) {
      mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      dir += "/";
    }
  }
}

void proxy_func(std::pair<int, int> *args) {
  int newfd = args->first;
  int requestid = args->second;
  Proxy proxy(requestid);
  proxy.handler(newfd);
  close(newfd);
}
int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: HTTPCachingProxy <port>\n";
    exit(EXIT_FAILURE);
  }
  int requestid = 0;
  Proxy proxy(argv[1]);
  createIndex(PATH);
  // become a daemon
  daemon(0, 0);
  umask(0);
  pid_t pid = fork();
  if (pid < 0)
    std::cerr << "fail to fork" << std::endl;
  while (1) {
    int newfd = proxy.accNewRequest();
    std::pair<int, int> *args = new std::pair<int, int>(newfd, requestid++);
    // std::thread t = std::thread(proxy_func, args);
    // t.detach();
    proxy_func(args);
  }
  return EXIT_SUCCESS;
}
