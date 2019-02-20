#include "client.h"

const char *Client::getHost(const char *hostname) {
  struct hostent *he = gethostbyname(hostname);
  struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
  return inet_ntoa(*addr_list[0]);
}
// pass unit test
Client::Client(const char *h, const char *p) : port(p) {
  hostname = getHost(h);

  addrinfo host_info;
  addrinfo *host_info_list;

  timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  try {
    if (getaddrinfo(hostname, port, &host_info, &host_info_list) != 0)
      throw std::string("getaddrinfo");

    sockfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                    host_info_list->ai_protocol);
    if (sockfd == -1)
      throw std::string("socket");

    // set recv timeout
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv,
                   sizeof tv))
      throw std::string("setsockopt");

    if ((connect(sockfd, host_info_list->ai_addr,
                 host_info_list->ai_addrlen)) == -1)
      throw std::string("connect");
  } catch (std::string e) {
    std::cerr << e << " failed\n";
  }
  freeaddrinfo(host_info_list);
}
Client::~Client() { close(sockfd); }

// pass unit test
int Client::sendall(int fd, const char *buf, size_t *len) {
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
// pass unit test
void Client::GET(std::string msg) {
  size_t sent = 0;
  size_t len = msg.length();
  size_t max = msg.length();
  while (sent < len) {
    sent = len - sent;
    len = sent;
    if (sendall(sockfd, msg.substr(max - len).c_str(), &sent) == -1) {
      std::cerr << "send failed\n";
      exit(EXIT_FAILURE);
    }
  }
}

// pass unit test, timeout=1, might be too long
std::vector<char> Client::recvall(int fd) {
  std::vector<char> msg;
  size_t index = 0;
  int nbytes;
  while (1) {
    if (msg.size() < index + MAXDATASIZE)
      msg.resize(index + MAXDATASIZE);
    nbytes = recv(fd, &msg.data()[index], MAXDATASIZE - 1, 0);
    if (nbytes == -1 && msg.empty()) {
      std::cerr << "recv failed\n";
      break;
    } else if (nbytes == -1) {
      break;

    } else if (nbytes == 0)
      break;
    else {
      index += nbytes;
    }
  }
  return msg;
}
std::string Client::recvGETResponse() { return recvall(sockfd).data(); }

void Client::POST(std::string msg) {
  size_t sent = 0;
  size_t len = msg.length();
  size_t max = msg.length();
  while (sent < len) {
    sent = len - sent;
    len = sent;
    if (sendall(sockfd, msg.substr(max - len).c_str(), &sent) == -1) {
      std::cerr << "send failed\n";
      exit(EXIT_FAILURE);
    }
  }
}
// initializer test
/*int main() { Client client("localhost", "8080"); }*/
// sendData test
// valgrind clean
/*int main() {
  Client client("rabihyounes.com", "80");
  client.sendData("GET /awesome.txt HTTP/1.1\r\nHost:rabihyounes.com\r\n\r\n");
  std::cout << client.receiveHTTP();
}
*/
// recv data test
/*
int main() {
  Client client("localhost", "8080");
  std::string test = client.receiveHTTP();
  std::cout << test << std::endl;
}
*/
