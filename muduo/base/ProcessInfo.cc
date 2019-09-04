// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/ProcessInfo.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/base/FileUtil.h"

#include <algorithm>

#include <assert.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h> // snprintf
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>

namespace muduo
{
namespace detail
{
__thread int t_numOpenedFiles = 0;                                                  // 线程特定数据，记录打开文件的数目
int fdDirFilter(const struct dirent* d)                                             // 传入scandir()函数文件过滤函数，struct dirent用于存放文件信息，包括：文件名等
{
  if (::isdigit(d->d_name[0]))                                                      // isdigit(char ch)判断字符ch是不是十进制字符，也就是'0'-'9'，这里判断文件名的首字符是不是十进制的数字字符
  {
    ++t_numOpenedFiles;
  }
  return 0;
}

__thread std::vector<pid_t>* t_pids = NULL;
int taskDirFilter(const struct dirent* d)
{
  if (::isdigit(d->d_name[0]))
  {
    t_pids->push_back(atoi(d->d_name));
  }
  return 0;
}

int scanDir(const char *dirpath, int (*filter)(const struct dirent *))
{
  struct dirent** namelist = NULL;
  int result = ::scandir(dirpath, &namelist, filter, alphasort);                    // scandir扫描指定目录及其子目录下符合filter过滤条件的文件（包括文件夹），返回值表示满足条件的个数，详细参考：https://www.cnblogs.com/zendu/p/4988001.html
  assert(namelist == NULL);
  return result;
}

Timestamp g_startTime = Timestamp::now();                                           // 进程启动时会执行Timestamp::now()，因为这里是全局变量
// assume those won't change during the life time of a process.                     -- 假设这些在进程的声明周期内不会改变
int g_clockTicks = static_cast<int>(::sysconf(_SC_CLK_TCK));                        // sysconf用来获取系统执行时的配置信息，_SC_CLK_TCK时钟滴答频率
int g_pageSize = static_cast<int>(::sysconf(_SC_PAGE_SIZE));                        // _SC_PAGE_SIZE ??
}  // namespace detail
}  // namespace muduo

using namespace muduo;
using namespace muduo::detail;

pid_t ProcessInfo::pid()
{
  return ::getpid();                                                                // 使用getpid()获取进程ID
}

string ProcessInfo::pidString()
{
  char buf[32];
  snprintf(buf, sizeof buf, "%d", pid());
  return buf;
}

uid_t ProcessInfo::uid()
{
  return ::getuid();                                                                // 使用getuid()获取用户ID
}

string ProcessInfo::username()
{
  struct passwd pwd;                                                                // 用于存放user相关信息包括：用户名，uid，gid等信息
  struct passwd* result = NULL;
  char buf[8192];
  const char* name = "unknownuser";

  getpwuid_r(uid(), &pwd, buf, sizeof buf, &result);                                // 使用getpwuid_r()获取user相关信息
  if (result)
  {
    name = pwd.pw_name;
  }
  return name;
}

uid_t ProcessInfo::euid()
{
  return ::geteuid();                                                               // 使用geteuid()获取用户有效ID
}

Timestamp ProcessInfo::startTime()
{
  return g_startTime;
}

int ProcessInfo::clockTicksPerSecond()
{
  return g_clockTicks;
}

int ProcessInfo::pageSize()
{
  return g_pageSize;
}

bool ProcessInfo::isDebugBuild()
{
#ifdef NDEBUG
  return false;
#else
  return true;
#endif
}

string ProcessInfo::hostname()
{
  // HOST_NAME_MAX 64
  // _POSIX_HOST_NAME_MAX 255
  char buf[256];
  if (::gethostname(buf, sizeof buf) == 0)                                              // 使用gethostname()获取主机名
  {
    buf[sizeof(buf)-1] = '\0';
    return buf;
  }
  else
  {
    return "unknownhost";
  }
}

string ProcessInfo::procname()                                                          // 调用StringPiece ProcessInfo::procname(const string& stat)获取进程名
{
  return procname(procStat()).as_string();
}

StringPiece ProcessInfo::procname(const string& stat)                                   // 读取到的/proc/self/stat内容是：52 (cat) R 4 52 3 1025 0 0 0 0 0 0 0 0 0 0 20 0 1 0 361642 1054825869312 231 18446744073709551615 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
{                                                                                       // ()中是进程名
  StringPiece name;
  size_t lp = stat.find('(');
  size_t rp = stat.rfind(')');
  if (lp != string::npos && rp != string::npos && lp < rp)
  {
    name.set(stat.data()+lp+1, static_cast<int>(rp-lp-1));
  }
  return name;
}

string ProcessInfo::procStatus()
{
  string result;
  FileUtil::readFile("/proc/self/status", 65536, &result);
  return result;
}

string ProcessInfo::procStat()
{
  string result;
  FileUtil::readFile("/proc/self/stat", 65536, &result);
  return result;
}

string ProcessInfo::threadStat()
{
  char buf[64];
  snprintf(buf, sizeof buf, "/proc/self/task/%d/stat", CurrentThread::tid());
  string result;
  FileUtil::readFile(buf, 65536, &result);
  return result;
}

string ProcessInfo::exePath()
{
  string result;
  char buf[1024];
  ssize_t n = ::readlink("/proc/self/exe", buf, sizeof buf);
  if (n > 0)
  {
    result.assign(buf, n);                                                              // 赋值，会发生拷贝
  }
  return result;
}

int ProcessInfo::openedFiles()
{
  t_numOpenedFiles = 0;
  scanDir("/proc/self/fd", fdDirFilter);                                                // /proc/self/fd是一个文件夹，进程没打开一个文件（没有一个文件描述符）就会在该路径下多一项，ls /proc/self/fd看到的内容是：0 1 2 3，数字对应文件描述符
  return t_numOpenedFiles;
}

int ProcessInfo::maxOpenFiles()
{
  struct rlimit rl;
  if (::getrlimit(RLIMIT_NOFILE, &rl))
  {
    return openedFiles();
  }
  else
  {
    return static_cast<int>(rl.rlim_cur);
  }
}

ProcessInfo::CpuTime ProcessInfo::cpuTime()
{
  ProcessInfo::CpuTime t;
  struct tms tms;
  if (::times(&tms) >= 0)
  {
    const double hz = static_cast<double>(clockTicksPerSecond());
    t.userSeconds = static_cast<double>(tms.tms_utime) / hz;
    t.systemSeconds = static_cast<double>(tms.tms_stime) / hz;
  }
  return t;
}

int ProcessInfo::numThreads()
{
  int result = 0;
  string status = procStatus();
  size_t pos = status.find("Threads:");
  if (pos != string::npos)
  {
    result = ::atoi(status.c_str() + pos + 8);
  }
  return result;
}

std::vector<pid_t> ProcessInfo::threads()
{
  std::vector<pid_t> result;
  t_pids = &result;
  scanDir("/proc/self/task", taskDirFilter);
  t_pids = NULL;
  std::sort(result.begin(), result.end());
  return result;
}

