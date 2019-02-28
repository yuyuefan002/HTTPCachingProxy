#include "proxy.h"
std::vector<char> getRevalidRequest(HTTPRSPNSParser &httprspnsparer,
                                    HTTParser &httparser) {

  std::string msg = httparser.getStatusLine() + "\r\n";
  msg = msg + "Host: " + httparser.getHostName() + "\r\n";
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
std::vector<char> Proxy::revalidation(HTTPRSPNSParser &httprspnsparser,
                                      HTTParser &httparser, Cache &cache) {
  std::vector<char> request = getRevalidRequest(httprspnsparser, httparser);
  return fetchNewResponse(cache, httparser, request);
}

/*
 * handlebyCache
 * use response in cache
 *
 */

std::vector<char> Proxy::handlebyCache(Cache &cache, HTTParser &httparser) {
  std::string url = httparser.getURL();
  if (!cache.check(url)) {
    log.checkCache(NOTINCACHE, "");
    return std::vector<char>();
  }
  std::vector<char> HTTPResponse = cache.read(url);
  HTTPRSPNSParser httprspnsparser(HTTPResponse);
  bool needRevalid = false;
  // need revalid
  if (httparser.mustRevalidate() || !httparser.good4Cache() ||
      httprspnsparser.mustRevalidate()) {
    log.checkCache(NEEDVALIDATE, "");
    needRevalid = true;
  }
  // expire
  else if (!httprspnsparser.not_expire()) {
    log.checkCache(EXPIRED, httprspnsparser.expiresAt());
    needRevalid = true;
  }
  if (needRevalid)
    return revalidation(httprspnsparser, httparser, cache);
  // valid
  log.checkCache(VALID, "");
  return httprspnsparser.getResponse();
}
/*
 * fetchNewresponse
 * ask original server for new response
 */
std::vector<char> Proxy::fetchNewResponse(Cache &cache, HTTParser &httparser,
                                          const std::vector<char> &request) {
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
  client.Send(request);
  log.reqFromServer(statusLine, hostname);
  std::vector<char> HTTPResponse = client.recvServeResponse();
  HTTPRSPNSParser httprspnsparser(HTTPResponse);
  log.recvFromServer(httprspnsparser.getStatusText(), hostname);
  if (httprspnsparser.getStatusCode() == 200 && httprspnsparser.good4Cache() &&
      httparser.good4Cache()) {
    cache.store(url, HTTPResponse);
    if (httprspnsparser.mustRevalidate())
      log.cachedNeedRevalid();
    else
      log.cached(httprspnsparser.expiresAt());
  } else {
    if (!httprspnsparser.good4Cache())
      log.notCacheable(httprspnsparser.whyBad4Cache());
    else if (!httparser.good4Cache())
      log.notCacheable(httparser.whyBad4Cache());
  }
  if (httprspnsparser.getStatusCode() == 304) {
    std::vector<char> HTTPResponse = cache.read(url);
    HTTPRSPNSParser respinCache(HTTPResponse);
    return respinCache.getResponse();
  }
  return HTTPResponse;
}

/*
 * status: complete
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
    HTTPResponse = handlebyCache(cache, httparser);
    if (HTTPResponse.empty()) {
      HTTPResponse = fetchNewResponse(cache, httparser, httparser.getRequest());
    }
    if (!helper.containNewLine(HTTPResponse)) {
      server.sendData(newfd, HTTP502());
      log.respondClient(HTTP502());
      return;
    }
  } catch (std::string e) {
    HTTPResponse = HTTP502();
  }
  if (HTTPResponse.size() != 0) {
    HTTPRSPNSParser httprspnsparser(HTTPResponse);
    log.respondClient(httprspnsparser.getStatusText());
  } else
    HTTPResponse = HTTP502();
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
  HTTPRSPNSParser httprspnsparser(HTTPResponse);

  log.recvFromServer(httprspnsparser.getStatusText(), hostname);
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
void tunnelMode(const int &clientfd, Log &log, Server &server, Client &client) {
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
      if (msg.empty()) {
        log.closeTunnel();
        return;
      }
      server.sendData(clientfd, msg);
    } else if (FD_ISSET(clientfd, &read_fds)) {
      std::vector<char> msg = server.basicRecv(clientfd);
      if (msg.empty()) {
        log.closeTunnel();
        return;
      }
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
    log.respondClient(HTTP503());
    server.sendData(newfd, HTTP503());
    return;
  }
  // success
  log.respondClient(HTTP200());
  server.sendData(newfd, HTTP200());
  // transition message
  tunnelMode(newfd, log, server, client);
}

/*
 *accept new request coming from browser
 */
int Proxy::accNewRequest() {
  int newfd = server.acceptNewConn();
  // if (newfd < 0)
  return newfd;
}

bool request_is_imcomplete(HTTParser &httparser,
                           std::vector<char> &HTTPRequest) {
  Helper helper;
  if (httparser.getMethod() != "POST" &&
      (!helper.containNewLine(HTTPRequest) || httparser.errorDetection() == 1))
    return true;
  return false;
}
void Proxy::handler(int newfd) {
  try {
    std::vector<char> HTTPRequest = server.receiveHTTPRequest(newfd);

    // std::vector<char> HTTPRequest = server.receiveData(newfd);
    HTTParser httparser(HTTPRequest);
    if (request_is_imcomplete(httparser, HTTPRequest)) {
      server.sendData(newfd, HTTP400());
      log.respondClient(HTTP400());
      return;
    }
    log.newRequest(httparser.getStatusLine(), getclientip(newfd));
    if (httparser.getMethod() == "GET")
      GET_handler(httparser, newfd);
    else if (httparser.getMethod() == "CONNECT")
      CONNECT_handler(httparser, newfd);
    else if (httparser.getMethod() == "POST")
      POST_handler(httparser, newfd);

  } catch (std::string e) {
  }
}
Proxy::Proxy(int requestid) : server(), log(requestid) {}
Proxy::Proxy(const char *port) : server(port), log() {}
Proxy::~Proxy() {}
