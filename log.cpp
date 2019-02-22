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

Log::Log(int id) : requestid(id) {}
Log::~Log() {}
