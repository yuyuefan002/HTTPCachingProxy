#include "cache.h"
// pass unit test
void Cache::createIndex(std::string path) {
  std::string dir;
  while (!path.empty()) {
    dir += helper.fetchNextSeg(path, '/');
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
  int root = 1;
  while (!url.empty()) {
    std::string dir = helper.fetchNextSeg(url, '/');
    path += "/" + dir;
    // untested code
    if (url.empty() && dir.find('.') == std::string::npos)
      path += "/index.html";
    if (root && url.empty())
      path += "/index.html";
    root = 0;
  }
  return path;
}
// pass unit test
Cache::Cache() {
  cachename = "proxycache";
  mkdir(cachename.c_str(),
        S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // ignore fail here
}
// original problem: cannot open a file has the samename as a path
// pass unit test
void Cache::store(const std::string &url, const std::vector<char> &msg) {
  std::ofstream ofs;
  std::string path = parseURL(url);
  createIndex(path);
  ofs.open(path, std::ofstream::out | std::ofstream::trunc);
  ofs << msg.data();
  ofs.close();
}

/* pass unit test
 * suggestion: use read() after check()
 * problem here, ifstream only input oneline here, use loop to input all
 */
std::vector<char> Cache::read(const std::string &url) {

  std::string path = parseURL(url);
  struct stat s;
  if (stat(path.c_str(), &s) == 0) {
    if (s.st_mode & S_IFDIR) {
      path += "/index.html";
    }
  }
  std::ifstream ifs(path, std::ios::binary);
  std::vector<char> msg(std::istreambuf_iterator<char>{ifs}, {});

  /*  std::stringstream buf;
  buf << t.rdbuf();
  msg = buf.str();*/
  return msg;
}

// pass unit test
// robust code, no way to fail or return wrong result
bool Cache::check(const std::string &url) {
  std::string path = parseURL(url);
  struct stat buf;
  return (stat(path.c_str(), &buf) == 0);
}
/*
int main() {
  Cache cache;
  cache.store(
      "HTTP/1.1 200 OK\r\nDate: Tue, 19 Feb 2019 00:45:50 GMT\r\nServer: "
      "Apache\r\nLast-Modified: Thu, 22 Apr 2004 15:52:25 "
      "GMT\r\nAccept-Ranges: bytes\r\nContent-Length: 24557\r\nVary: "
      "Accept-Encoding,User-Agent\r\nContent-Type: text/html\r\nSet-Cookie: "
      "BIGipServer~CUIT~www.columbia.edu-80-pool=1781021568.20480.0000; "
      "expires=Tue, 19-Feb-2019 06:45:50 GMT; path=/; "
      "Httponly\r\n\r\n<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 "
      "Transitional//EN\">\r\n<html>\r\n<head>\r\n<!-- THIS IS A "make
  if (cache.check("https://rabihyounes.com/awesome.txt"))
    std::cout << cache.read("https://rabihyounes.com/awesome.txt");
  std::cout << "finish\n";
}
    */
// check freshness
// check expires field at first, check max-age directive at second(more
// important, if exist,ignore expire)
// check age first, if none, check expire,if none,check last-modefied,10%
// header may has a field cache-control:(no-cache)
// URI containing '?' is not cacheable
