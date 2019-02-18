#ifndef __HELPER_H__
#define __HELPER_H__
#include <ctime>
#include <iostream>
#include <map>
#include <string>
class Helper {
public:
  std::string trimLeadingSpace(std::string &msg);
  std::string fetchNextSeg(std::string &msg, char c = ' ',
                           size_t substrlen = 1);
  std::string tolower(const std::string &msg);
  int wdayTable(std::string wday);
  int monTable(std::string mon);
  double HTTPTimeRange2Num(std::string end, std::string start);
  std::string deleteALine(std::string msg, size_t start);
  size_t HTTPAge(std::string date);
};
#endif
