#include "proxy.h"

std::vector<char> getRevalidRequest(HTTPRSPNSParser &httprspnsparer,
                                    HTTParser &httparser) {

  std::string msg = httparser.getStatusLine() + "\r\n";

  msg = msg + "If-Modified-Since: " + httprspnsparer.getLastModified() + "\r\n";
  std::string tmp = httprspnsparer.getETag();
  if (tmp.size() != 0)
    msg = msg + "If-None-Match: " + tmp + "\r\n";
  msg += "\r\n";
  std::vector<char> request(msg.begin(), msg.end());
  return request;
}
/*
 * revalidation
 * status: untested
 */
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
  std::string statusLine = httparser.getStatusLine();
  Client client(hostname.c_str(),
                port.c_str()); // have to check success or not, if failed,
                               // return 503,important
  if (client.getError() == 1) {
    return HTTP503();
  }
  client.Send(httparser.getRequest());
  log.reqFromServer(statusLine, hostname);
  std::vector<char> HTTPResponse = client.recvServeResponse();
  log.recvFromServer(statusLine, hostname);
  HTTPRSPNSParser httprspnsparser(HTTPResponse);
  if (httprspnsparser.getStatusCode() == 200 && httprspnsparser.good4Cache() &&
      httparser.good4Cache())
    cache.store(url, HTTPResponse);
  return HTTPResponse;
}

/*
 * status: not tested
 * get ip of client
 */
std::string getclientip(int newfd) {
  sockaddr_storage addr;
  socklen_t len = sizeof addr;
  char ip[INET_ADDRSTRLEN];
  try {
    if (getpeername(newfd, (struct sockaddr *)&addr, &len) == -1)
      throw std::string("getpeername");
    sockaddr_in *s = (sockaddr_in *)&addr;
    if (inet_ntop(AF_INET, &s->sin_addr, ip, sizeof ip) == NULL)
      throw std::string("inet_ntop");
  } catch (std::string e) {
    std::cerr << "Error: " << e << " failed" << std::endl;
  }
  return std::string(ip);
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
    if (HTTPResponse.empty()) {
      HTTPResponse = fetchNewResponse(cache, httparser);
    }
    std::vector<char> pattern{'\r', '\n', '\r', '\n'};
    if (std::search(HTTPResponse.begin(), HTTPResponse.end(), pattern.begin(),
                    pattern.end()) == HTTPResponse.end()) {
      server.sendData(newfd, HTTP502());
      log.respondClient(std::string(HTTP502().begin(), HTTP502().end()));
      return;
    }
  } catch (std::string e) {
    HTTPResponse = HTTP502();
  }
  HTTPRSPNSParser httprspnsparser(HTTPResponse);
  log.respondClient(httprspnsparser.getStatusText());
  server.sendData(newfd, HTTPResponse);
}

/*
 * status:complete
 * handle POST request
 */
void Proxy::POST_handler(HTTParser &httparser, int newfd) {
  std::string hostname = httparser.getHostName();
  std::string port = httparser.getHostPort();
  std::string statusLine = httparser.getStatusLine();
  Client client(hostname.c_str(),
                port.c_str()); // have to check success or not, if failed,
                               // return 503,important
  if (client.getError() == 1) {
    server.sendData(newfd, HTTP503());
    return;
  }
  client.Send(httparser.getRequest());
  log.reqFromServer(statusLine, hostname);
  std::vector<char> HTTPResponse = client.recvServeResponse();
  log.recvFromServer(statusLine, hostname);
  HTTPRSPNSParser httprspnsparser(HTTPResponse);
  log.respondClient(httprspnsparser.getStatusText());
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
      std::vector<char> msg = client.basicRecv();
      if (msg.empty())
        return;
      server.sendData(clientfd, msg);
    } else if (FD_ISSET(clientfd, &read_fds)) {
      std::vector<char> msg = server.basicRecv(clientfd);
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
    log.respondClient(std::string(HTTP503().begin(), HTTP503().end()));
    server.sendData(newfd, HTTP503());
    return;
  }
  // success
  log.respondClient(std::string(HTTP200().begin(), HTTP200().end()));
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

bool request_is_imcomplete(HTTParser &httparser,
                           std::vector<char> &HTTPRequest) {
  std::vector<char> pattern{'\r', '\n', '\r', '\n'};
  if (httparser.getMethod() != "POST" &&
      std::search(HTTPRequest.begin(), HTTPRequest.end(), pattern.begin(),
                  pattern.end()) == HTTPRequest.end()) {
    return true;
  }
  return false;
}
void Proxy::handler(int newfd) {
  try {
    std::vector<char> HTTPRequest = server.receiveData(newfd);
    HTTParser httparser(HTTPRequest);
    if (request_is_imcomplete(httparser, HTTPRequest)) {
      server.sendData(newfd, HTTP400());
      return;
    }
    // log.newRequest(httparser.getStatusLine(), getclientip(newfd));

    if (httparser.getMethod() == "GET")

      GET_handler(httparser, newfd);

    else if (httparser.getMethod() == "CONNECT") {
      CONNECT_handler(httparser, newfd);
    }

    else if (httparser.getMethod() == "POST") {
      POST_handler(httparser, newfd);
    }
  } catch (std::string e) {
    std::cerr << e << std::endl;
  }
}
Proxy::Proxy(int requestid) : server(), log(requestid) {}
Proxy::Proxy(const char *port) : server(port), log() {}
Proxy::~Proxy() {}
