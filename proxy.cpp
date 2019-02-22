#include "proxy.h"

std::vector<char> getRevalidRequest(HTTPRSPNSParser &httprspnsparer,
                                    HTTParser &httparser) {

  std::string msg = httparser.getStatusLine() + "\r\n";

  msg = msg + "If-Modified-Since: " + httprspnsparer.getLastModified() + "\r\n";
  msg = msg + "If-None-Match: " + httprspnsparer.getETag() + "\r\n\r\n";
  std::vector<char> request(msg.begin(), msg.end());
  return request;
}
bool revalidationSuccess(HTTPRSPNSParser &httprspnsparser,
                         HTTParser &httparser) {
  std::string hostname = httparser.getHostName();
  std::string port = httparser.getHostPort();
  Client client(hostname.c_str(),
                port.c_str()); // have to check success or not, if failed,
                               // return 503,important
  std::vector<char> validRequest =
      getRevalidRequest(httprspnsparser, httparser);
  client.Send(validRequest);
  std::vector<char> HTTPResponse = client.recvServeResponse();
  HTTPRSPNSParser validate(HTTPResponse);
  if (validate.getStatusCode() == 304)
    return true;
  return false;
}

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
    if (httprspnsparser.stillfresh()) {
      return httprspnsparser.getResponse();
    } else {
      if (revalidationSuccess(httprspnsparser, httparser))
        return HTTPResponse;
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
    return HTTP503();
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
  std::vector<char> HTTPResponse;
  try {
    if (httparser.good4Cache()) {

      HTTPResponse = handlebyCache(cache, httparser);
    }
    if (HTTPResponse.empty())
      HTTPResponse = fetchNewResponse(cache, httparser);
    std::vector<char> pattern{'\r', '\n', '\r', '\n'};
    if (std::search(HTTPResponse.begin(), HTTPResponse.end(), pattern.begin(),
                    pattern.end()) == HTTPResponse.end()) {
      server.sendData(newfd, HTTP502());
      return;
    }
  } catch (std::string e) {
    HTTPResponse = HTTP502();
  }
  server.sendData(newfd, HTTPResponse);
}

/*
 * status:complete
 * handle POST request
 */
void Proxy::POST_handler(HTTParser &httparser, int newfd) {
  std::string hostname = httparser.getHostName();
  std::string port = httparser.getHostPort();
  Client client(hostname.c_str(),
                port.c_str()); // have to check success or not, if failed,
                               // return 503,important
  if (client.getError() == 1) {
    server.sendData(newfd, HTTP503());
    return;
  }
  client.Send(httparser.getRequest());
  std::vector<char> HTTPResponse = client.recvServeResponse();
  server.sendData(newfd, HTTPResponse);
}

/*
 * preparetunnel
 * This function do preparation for select()
 *
 *
 */
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
void tunnelMode(const int &clientfd, Server &server, Client &client) {
  // Build connection
  fd_set master, read_fds;
  int fdmax, serverfd = client.getFD();
  preparetunnel(&master, fdmax, clientfd, serverfd);
  FD_ZERO(&read_fds);
  while (1) {
    read_fds = master;
    if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
      break;
    }
    if (FD_ISSET(serverfd, &read_fds)) {
      std::vector<char> msg = client.recvServeResponse();
      if (msg.empty())
        return;
      server.sendData(clientfd, msg);
    } else if (FD_ISSET(clientfd, &read_fds)) {
      std::vector<char> msg = server.receiveData(clientfd);
      if (msg.empty())
        return;
      client.Send(msg);
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
    server.sendData(newfd, HTTP503());
    return;
  }
  // success

  server.sendData(newfd, HTTP200());
  // transition message
  tunnelMode(newfd, server, client);
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
  try {
    std::vector<char> HTTPRequest = server.receiveData(newfd);
    HTTParser httparser(HTTPRequest);
    std::vector<char> pattern{'\r', '\n', '\r', '\n'};
    if (httparser.getMethod() != "POST" &&
        std::search(HTTPRequest.begin(), HTTPRequest.end(), pattern.begin(),
                    pattern.end()) == HTTPRequest.end()) {
      server.sendData(newfd, HTTP400());
      return;
    }
    if (httparser.getMethod() == "GET")
      GET_handler(httparser, newfd);

    else if (httparser.getMethod() == "CONNECT")
      CONNECT_handler(httparser, newfd);

    else if (httparser.getMethod() == "POST")
      POST_handler(httparser, newfd);
  } catch (std::string e) {
    std::cerr << e << std::endl;
  }
}
Proxy::Proxy() : server() {}
Proxy::Proxy(const char *port) : server(port) {}
Proxy::~Proxy() {}
