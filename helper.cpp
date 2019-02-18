#include "helper.h"
/*
 * warning: this function will motify original string
 * pass unit test
 */
std::string Helper::trimLeadingSpace(std::string &msg) {
  size_t target = msg.find_first_not_of(' ');
  return msg.substr(target);
}
// pass unit test
std::string Helper::fetchNextSeg(std::string &msg, char c, size_t substrlen) {
  msg = trimLeadingSpace(msg);
  size_t target = msg.find(c);
  std::string res = msg.substr(0, target);
  if (target != std::string::npos)
    msg = msg.substr(target + substrlen);
  else
    msg = "";

  return res;
}
std::string Helper::tolower(const std::string &msg) {
  std::string res;
  for (auto c : msg) {
    res += std::tolower(c);
  }
  return res;
}
int Helper::wdayTable(std::string wday) {
  std::map<std::string, int> w = {{"Sun", 0}, {"Mon", 1}, {"Tue", 2},
                                  {"Wed", 3}, {"Thu", 4}, {"Fri", 5},
                                  {"Sat", 6}};
  return w[wday];
}
int Helper::monTable(std::string mon) {
  std::map<std::string, int> m = {
      {"Jan", 0}, {"Feb", 1}, {"Mar", 2}, {"Apr", 3}, {"May", 4},  {"Jun", 5},
      {"Jul", 6}, {"Aug", 7}, {"Sep", 8}, {"Oct", 9}, {"Nov", 10}, {"Dec", 11}};
  return m[mon];
}
/*
 * status: complete
 * problem: after difftime, end_tm.tm_hour will increase 1, cannot resolve
 * solution: set tm_isdst
 * pass unit test
 */
double Helper::HTTPTimeRange2Num(std::string end, std::string start) {
  double seconds;

  struct tm end_tm, start_tm;
  end_tm.tm_wday = wdayTable(fetchNextSeg(end, ','));
  end_tm.tm_mday = stoi(fetchNextSeg(end));
  end_tm.tm_mon = monTable(fetchNextSeg(end));
  end_tm.tm_year = stoi(fetchNextSeg(end)) - 1900;
  end_tm.tm_hour = stoi(fetchNextSeg(end, ':'));
  end_tm.tm_min = stoi(fetchNextSeg(end, ':'));
  end_tm.tm_sec = stoi(fetchNextSeg(end));
  end_tm.tm_isdst =
      0; // this is important, missing this will cause ambigous time
  std::string zone = fetchNextSeg(end);
  end_tm.tm_zone = zone.c_str();

  start_tm.tm_wday = wdayTable(fetchNextSeg(start, ','));
  start_tm.tm_mday = stoi(fetchNextSeg(start));
  start_tm.tm_mon = monTable(fetchNextSeg(start));
  start_tm.tm_year = stoi(fetchNextSeg(start)) - 1900;
  start_tm.tm_hour = stoi(fetchNextSeg(start, ':'));
  start_tm.tm_min = stoi(fetchNextSeg(start, ':'));
  start_tm.tm_sec = stoi(fetchNextSeg(start));
  start_tm.tm_isdst = 0;
  std::string zone2 = fetchNextSeg(start);
  start_tm.tm_zone = zone2.c_str();

  seconds = difftime(mktime(&end_tm), mktime(&start_tm));
  return seconds;
}
/* pass unit test
 * warning: something unsure may happen, currently usring gmt time,
 *          i don't know how difftime works, will it change
 *          timezone automatically?
 *
 *
 */
size_t Helper::HTTPAge(std::string date) {
  size_t seconds;
  struct tm date_tm;
  date_tm.tm_wday = wdayTable(fetchNextSeg(date, ','));
  date_tm.tm_mday = stoi(fetchNextSeg(date));
  date_tm.tm_mon = monTable(fetchNextSeg(date));
  date_tm.tm_year = stoi(fetchNextSeg(date)) - 1900;
  date_tm.tm_hour = stoi(fetchNextSeg(date, ':'));
  date_tm.tm_min = stoi(fetchNextSeg(date, ':'));
  date_tm.tm_sec = stoi(fetchNextSeg(date));
  date_tm.tm_isdst = 0;
  std::string zone2 = fetchNextSeg(date);
  date_tm.tm_zone = zone2.c_str();
  time_t now;
  time(&now);
  struct tm *ptm = gmtime(&now);
  seconds = difftime(mktime(ptm), mktime(&date_tm));
  return seconds;
}
std::string Helper::deleteALine(std::string msg, size_t date) {
  size_t end = date;
  while (msg[end] != '\n')
    end++;
  msg.erase(date, end + 1 - date);
  return msg;
}
/*
int main() {
  Helper helper;
  //  std::cout << helper.HTTPTimeRange2Num("Fri, 08 Feb 2019 18:44:41 GMT",
  //                                     "Fri, 08 Feb 2019 18:43:41 GMT")
  //<< std::endl;
  std::cout << helper.HTTPAge("Date: Mon, 18 Feb 2019 20:55:11 GMT");
  // std::cout <<
  // helper.deleteALine("asd\r\ndelete\r\nA
  // new line\r\n", 5);
}
*/
