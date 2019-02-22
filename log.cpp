#include "log.h"

void Log::save(std::string msg) {
  std::ofstream outfile;
  outfile.open("/var/log/erss/proxy.log", std::ios::out | std::ios::app);
  outfile << msg << std::endl;
  outfile.close();
}

void Log::newRequest(std::string statusLine, std::string clientip) {
  auto now = std::chrono::system_clock::now();
  time_t t = std::chrono::system_clock::to_time_t(now);
  char *time = ctime(&t);
  std::string msg = std::to_string(requestid) + ": " + statusLine + " from " +
                    clientip + std::string(time);
  save(msg);
}

void Log::checkCache() {}

void Log::reqFromServer(std::string statusLine, std::string serverName) {
  std::string msg = std::to_string(requestid) + ": Requesting " + statusLine +
                    " from " + serverName;
  save(msg);
}

void Log::recvFromServer(std::string statusText, std::string serverName) {
  std::string msg = std::to_string(requestid) + ": Received " + statusText +
                    " from " + serverName;
  save(msg);
}

void Log::respondClient(std::vector<char> statusText) {
  std::string msg = std::to_string(requestid) + ": Responding " +
                    std::string(statusText.begin(), statusText.end());
  save(msg);
}

Log::Log() : requestid(-1) {}
Log::Log(int id) : requestid(id) {}
Log::~Log() {}
