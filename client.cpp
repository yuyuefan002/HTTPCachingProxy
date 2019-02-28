#include "client.h"

/*
 * getHost
 * This function can get IP from hostname
 *
 */
const char *Client::getHost(const char *hostname) {
  struct hostent *he = gethostbyname(hostname);
  if (he == nullptr)
    throw std::string("no host");
  struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
  return inet_ntoa(*addr_list[0]);
}

/*
 * Sendall
 * This function try to send all data to the socket
 *
 * Test Status: pass unit test
 */
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

/*
 * Send
 * This function make sure send all data to the socket
 *
 * Test Status: pass unit test
 */
void Client::Send(const std::vector<char> &msg) {
  size_t sent = 0;
  size_t len = msg.size();
  size_t max = msg.size();
  while (sent < len) {
    sent = len - sent;
    len = sent;
    if (sendall(sockfd, &msg.data()[max - len], &sent) == -1) {
      throw std::string("send failed");
    }
  }
}

/*
 * recvall
 * This function will receive all data from one socket
 *
 * Test Status: pass unit test, timeout=1, might be too long
 */
std::vector<char> Client::recvall(int fd) {
  // set recv timeout
  timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv))
    throw std::string("setsockopt");

  std::vector<char> msg;
  size_t index = 0;
  int nbytes;
  while (1) {
    if (msg.size() < index + MAXDATASIZE)
      msg.resize(index + MAXDATASIZE);
    nbytes = recv(fd, &msg.data()[index], MAXDATASIZE - 1, 0);
    if (nbytes == -1 && msg.empty()) {
      break;
    } else if (nbytes <= 0) {
      break;

    } else {
      index += nbytes;
    }
  }
  msg.resize(index);
  return msg;
}
std::vector<char> Client::basicRecv() {
  int index = 0;
  std::vector<char> msg;
  msg.resize(MAXDATASIZE);
  index = recv(sockfd, &msg.data()[index], MAXDATASIZE - 1, 0);
  if (index == -1)
    throw std::string("recv failed");
  msg.resize(index);
  return msg;
}
std::vector<char> Client::recvServeResponse() { return recvall(sockfd); }

int Client::getError() { return error; }

int Client::getFD() { return sockfd; }

/*
 * initializer
 * This function initialize the socket and connect to server
 * status: uncomplete, exception
 *
 */
Client::Client(const char *h, const char *p) : port(p) {
  hostname = getHost(h);
  error = 0;
  addrinfo host_info;
  addrinfo *host_info_list;

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

    if ((connect(sockfd, host_info_list->ai_addr,
                 host_info_list->ai_addrlen)) == -1)
      throw std::string("connect");
  } catch (std::string e) {
    error = 1;
  }
  freeaddrinfo(host_info_list);
}
Client::~Client() { close(sockfd); }
// initializer test
/*int main() { Client client("localhost", "8080"); }*/
// sendData test
// valgrind clean
/*int main() {
  Client client("rabihyounes.com", "80");
  client.sendData("GET /awesome.txt
HTTP/1.1\r\nHost:rabihyounes.com\r\n\r\n"); std::cout << client.receiveHTTP();
}
*/
// recv data test
/*
int main() {
  Client client("www.google.com", "443");
  std::vector<char> msg = {'t', 'e', 's', 't'};
  client.Send(msg);
  std::vector<char> test = client.recvServeResponse();
  std::cout << test.data() << std::endl;
}
*/
