#include "proxy.h"

/*
 * handlebyCache
 * use response in cache
 *
 */
std::vector<char> Proxy::handlebyCache(Cache &cache, HTTParser &httparser) {
  std::string url = httparser.getURL();
  if (cache.check(url)) {
    std::vector<char> HTTPResponse = cache.read(url);
    HTTPRSPNSParser httprspnsparser(HTTPResponse);
    if (httparser.good4Cache() && httprspnsparser.stillfresh()) {
      return httprspnsparser.getResponse();
    }
  }
  return std::vector<char>();
}

/*
 * fetchNewresponse
 * ask original server for new response
 */
std::vector<char> Proxy::fetchNewResponse(Cache &cache, HTTParser &httparser) {
  std::string url = httparser.getURL();
  std::string hostname = httparser.getHostName();
  std::string port = httparser.getHostPort();
  Client client(hostname.c_str(),
                port.c_str()); // have to check success or not, if failed,
                               // return 503,important
  if (client.getError() == 1) {
    std::string r = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
    std::vector<char> HTTP503(r.begin(), r.end());
    return HTTP503;
  }
  client.Send(httparser.getRequest());
  std::vector<char> HTTPResponse = client.recvServeResponse();

  HTTPRSPNSParser httprspnsparser(HTTPResponse);
  if (httprspnsparser.getStatusCode() == 200 && httprspnsparser.good4Cache() &&
      httparser.good4Cache())
    cache.store(url, HTTPResponse);
  return HTTPResponse;
}

/*
 * status:complete
 * handle Method GET
 */
void Proxy::GET_handler(HTTParser &httparser, int newfd) {
  Cache cache;
  std::vector<char> HTTPResponse = handlebyCache(cache, httparser);
  if (HTTPResponse.empty())
    HTTPResponse = fetchNewResponse(cache, httparser);
  server.sendData(newfd, HTTPResponse);
}

/*
 * status:unfinished
 * handle POST request
 */
void Proxy::POST_handler(HTTParser &httparser, int newfd) {
  std::string hostname = httparser.getHostName();
  std::string port = httparser.getHostPort();
  Client client(hostname.c_str(),
                port.c_str()); // have to check success or not, if failed,
                               // return 503,important
  if (client.getError() == 1) {
    std::string r = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
    std::vector<char> HTTP503(r.begin(), r.end());
    server.sendData(newfd, HTTP503);
    return;
  }
  client.Send(httparser.getRequest());
  std::vector<char> HTTPResponse = client.recvServeResponse();
  server.sendData(newfd, HTTPResponse);
}
void preparetunnel(fd_set *master, int &fdmax, const int &clientfd,
                   const int &serverfd) {
  FD_ZERO(master);
  FD_SET(clientfd, master);
  FD_SET(serverfd, master);
  fdmax = std::max(clientfd, serverfd);
}

/*
 * tunnelmode
 * This function just implement a select(), working as a tunnel transfer message
 * between browser and original server
 *
 *
 */
void tunnelMode(fd_set &master, int &fdmax, const int &clientfd,
                const int &serverfd, Server &server, Client &client) {
  fd_set read_fds;
  FD_ZERO(&read_fds);
  while (1) {
    read_fds = master;
    if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
      break;
    }
    for (int i = fdmax; i >= 0; i--) {
      if (FD_ISSET(i, &read_fds)) {
        if (i == serverfd) {
          std::vector<char> msg = client.recvServeResponse();
          if (msg.empty()) {
            return;
          }
          server.sendData(clientfd, msg);
        } else if (i == clientfd) {
          std::vector<char> msg = server.receiveData(clientfd);
          if (msg.empty()) {
            return;
          }
          client.Send(msg);
        }
      }
    }
  }
}
/*
 * status:unfinished
 * handle method CONNECT
 */
void Proxy::CONNECT_handler(HTTParser &httparser, int newfd) {
  std::string hostname = httparser.getHostName();
  std::string port = httparser.getHostPort();
  Client client(hostname.c_str(),
                port.c_str()); // if fail, return 503,important
  if (client.getError() == 1) {
    std::string r = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
    std::vector<char> HTTP503(r.begin(), r.end());
    server.sendData(newfd, HTTP503);
    return;
  }
  // success
  std::string r = "HTTP/1.1 200 OK\r\n\r\n";
  std::vector<char> HTTP200(r.begin(), r.end());
  server.sendData(newfd, HTTP200);
  // Build connection
  fd_set master;
  int fdmax;
  int serverfd = client.getFD();
  preparetunnel(&master, fdmax, newfd, serverfd);
  // transition message
  tunnelMode(master, fdmax, newfd, serverfd, server, client);
}

/*
 *accept new request coming from browser
 */
int Proxy::accNewRequest() {
  int newfd = server.acceptNewConn();
  if (newfd < 0)
    std::cerr << "Fail to accept a new request\n";
  return newfd;
}
void Proxy::handler(int newfd) {
  // std::vector<char> HTTPRequest = server.receiveHTTPRequest(newfd);
  std::vector<char> HTTPRequest = server.receiveData(newfd);
  std::cout << HTTPRequest.data();
  try {
    HTTParser httparser(HTTPRequest);
    if (httparser.getMethod() == "GET")
      GET_handler(httparser, newfd);
    else if (httparser.getMethod() == "CONNECT")
      CONNECT_handler(httparser, newfd);
    else if (httparser.getMethod() == "POST")
      POST_handler(httparser, newfd);
  } catch (std::string e) {
  }
}
Proxy::Proxy(const char *port) : server(port) {}
Proxy::~Proxy() {}
