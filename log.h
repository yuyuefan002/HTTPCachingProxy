#ifndef __LOG_H__
#define __LOG_H__
#include <fstream>
#include <string>
#include <unistd.h>
class Log {
private:
  void save(std::string msg);

public:
};
#endif
