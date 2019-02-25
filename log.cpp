#include "log.h"

void Log::save(std::string msg) {
  std::ofstream outfile;
  outfile.open("/var/log/erss/proxy.log", std::ios::out | std::ios::app);
  outfile << msg;
  outfile.close();
}

void Log::newRequest(std::string statusLine, std::string clientip) {
  auto now = std::chrono::system_clock::now();
  time_t t = std::chrono::system_clock::to_time_t(now);
  char *time = ctime(&t);
  std::string msg = std::to_string(requestid) + ": " + statusLine + " from " +
                    clientip + " @ " + std::string(time);
  save(msg);
}

void Log::checkCache(int status, std::string expiredTime) {
  std::string msg = std::to_string(requestid) + ": ";
  if (status == NOTINCACHE) {
    msg += "not in cache";
  } else if (status == EXPIRED) {
    msg += "in cache, but expired at " + expiredTime;
  } else if (status == NEEDVALIDATE) {
    msg += "in cache, requires validation";
  } else if (status == VALID) {
    msg += "in cache, valid";
  } else {
    msg += "fail to check cache status";
  }
  save(msg);
}

void Log::reqFromServer(std::string statusLine, std::string serverName) {
  std::string msg = std::to_string(requestid) + ": Requesting " + statusLine +
                    " from " + serverName;
  save(msg);
}

void Log::recvFromServer(std::vector<char> statusText, std::string serverName) {
  std::string msg = std::to_string(requestid) + ": Received " +
                    std::string(statusText.begin(), statusText.end()) +
                    " from " + serverName;
  save(msg);
}

void Log::respondClient(std::vector<char> statusText) {
  std::string msg = std::to_string(requestid) + ": Responding " +
                    std::string(statusText.begin(), statusText.end());
  save(msg);
}

void Log::notCacheable(std::string reason) {
  std::string msg =
      std::to_string(requestid) + ": not cacheable because " + reason;
  save(msg);
}
void Log::cached(std::string expireDate) {
  std::string msg =
      std::to_string(requestid) + ": cached, expires at " + expireDate;
  save(msg);
}
void Log::cachedNeedRevalid() {
  std::string msg =
      std::to_string(requestid) + ": cached, but requires revalidation\n";
  save(msg);
}

void Log::closeTunnel() {
  std::string msg = std::to_string(requestid) + ": Tunnel closed\n";
  save(msg);
}

Log::Log() : requestid(-1) {}
Log::Log(int id) : requestid(id) {}
Log::~Log() {}
