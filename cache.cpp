#include "cache.h"
/*
 * warning: this function will motify original string
 * pass unit test
 */
std::string trimLeadingSpace(std::string &msg) {
  size_t target = msg.find_first_not_of(' ');
  return msg.substr(target);
}
/*
 * warning: this function will motify original string
 * pass unit test
 */
std::string fetchNextSeg(std::string &msg, char c = ' ', size_t substrlen = 1) {
  msg = trimLeadingSpace(msg);
  size_t target = msg.find(c);
  std::string res = msg.substr(0, target);
  if (target != std::string::npos)
    msg = msg.substr(target + substrlen);
  else
    msg = "";

  return res;
}
// pass unit test
void Cache::createIndex(std::string path) {
  std::string dir;
  while (!path.empty()) {
    dir += fetchNextSeg(path, '/');
    if (!path.empty()) {
      mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
      dir += "/";
    }
  }
}
/*
 * status:untested
 * suggestion: avoid invalid url(fatal)
 *
 */
std::string Cache::parseURL(std::string url) {
  size_t target;
  std::string path = cachename;
  if ((target = url.find("://")) != std::string::npos) {
    url = url.substr(target + 3);
  }
  while (!url.empty()) {
    std::string dir = fetchNextSeg(url, '/');
    path += "/" + dir;
  }
  return path;
}
// pass unit test
Cache::Cache() {
  cachename = "proxycache";
  int status;
  status = mkdir(cachename.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}
// pass unit test
void Cache::store(const std::string &url, const std::string &msg) {
  std::ofstream ofs;
  std::string path = parseURL(url);
  createIndex(path);
  ofs.open(path, std::ofstream::out | std::ofstream::trunc);
  ofs << msg;
  ofs.close();
}

/* pass unit test
 * suggestion: use read() after check()
 */
std::string Cache::read(const std::string &url) {
  std::string msg;
  std::ifstream ifs;
  std::string path = parseURL(url);
  ifs.open(path, std::ifstream::in);
  ifs >> msg;
  ifs.close();
  return msg;
}

// pass unit test
// robust code, no way to fail or return wrong result
inline bool Cache::check(const std::string &url) {
  std::string path = parseURL(url);
  struct stat buf;
  return (stat(path.c_str(), &buf) == 0);
}
/*
int main() {
  Cache cache;
  cache.store("https://www.rabihyounes.com/awesome.txt", "another hello");
  if (cache.check("https://www.rabihyounes.com/awesome.txt"))
    std::cout << cache.read("https://www.rabihyounes.com/awesome.txt");
}
*/
