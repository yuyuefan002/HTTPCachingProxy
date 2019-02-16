#include "server.h"
using namespace std;
Server::Server(const char *p) : port(p) {
  const char *hostname = NULL;
  addrinfo *host_info_list;
  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;
  try {
    if (getaddrinfo(hostname, port, &host_info, &host_info_list) != 0)
      throw std::string("getaddrinfo");
    listener = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                      host_info_list->ai_protocol);
    if (listener == -1)
      throw std::string("socket");

    int yes = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listener, host_info_list->ai_addr, host_info_list->ai_addrlen) ==
        -1)
      throw std::string("bind");
    if (listen(listener, 100) == -1)
      throw std::string("listen");
  } catch (std::string e) {
    std::cerr << "Error: " << e << " failed" << std::endl;
    freeaddrinfo(host_info_list);
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(host_info_list);
}

int Server::acceptNewConn() {
  sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof socket_addr;
  int newfd = accept(listener, (sockaddr *)&socket_addr, &socket_addr_len);
  if (newfd == -1) {
    std::cerr << "accept failed\n";
    return -1;
  }
  return newfd;
}
std::string Server::recvall(int fd) {
  std::string msg;
  while (msg.find("\r\n\r\n") == std::string::npos) {
    char buf[MAXDATASIZE] = "";
    int nbytes;
    if ((nbytes = recv(fd, buf, MAXDATASIZE - 1, 0)) == -1) {
      std::cerr << "recv failed\n";
      exit(EXIT_FAILURE);
    } else {
      buf[nbytes] = '\0';
      msg += buf;
    }
  }
  return msg;
}

int Server::sendall(int fd, const char *buf, size_t *len) {
  size_t total = 0;     // how many bytes we've sent
  int bytesleft = *len; // how many we have left to send
  int n;

  while (total < *len) {
    if ((n = send(fd, buf + total, bytesleft, 0)) == -1) {
      break;
    }
    total += n;
    bytesleft -= n;
  }

  *len = total; // return number actually sent here

  return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}
std::string Server::receiveHTTPRequest(int fd) { return recvall(fd); }
void Server::sendData(int fd, std::string msg) {
  size_t sent = 0;
  size_t len = msg.length();
  size_t max = msg.length();
  while (sent < len) {
    sent = len - sent;
    len = sent;
    if (sendall(fd, msg.substr(max - len).c_str(), &sent) == -1) {
      std::cerr << "send failed\n";
    }
  }
}
Server::~Server() { close(listener); }
