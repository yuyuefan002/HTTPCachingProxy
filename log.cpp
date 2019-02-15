#include "log.h"

void Log::save(std::string msg) {
  std::ofstream outfile;
  outfile.open("/var/log/erss/proxy.log", std::ios::out | std::ios::app);
  outfile << msg << std::endl;
  outfile.close();
}
