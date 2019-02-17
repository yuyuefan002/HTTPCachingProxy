#ifndef __CACHE_H__
#define __CACHE_H__
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
class Cache {
private:
  std::string cachename;
  std::string parseURL(std::string url);
  void createIndex(std::string path);

public:
  Cache();
  void store(const std::string &path, const std::string &msg);
  std::string read(const std::string &url);
  inline bool check(const std::string &url);
};

#endif
