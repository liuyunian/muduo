// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_BASE_PROCESSINFO_H
#define MUDUO_BASE_PROCESSINFO_H

#include "muduo/base/StringPiece.h"
#include "muduo/base/Types.h"
#include "muduo/base/Timestamp.h"
#include <vector>
#include <sys/types.h>

namespace muduo
{

namespace ProcessInfo                                               // 进程相关信息
{
  pid_t pid();                                                      // 获取进程id
  string pidString();                                               // 进程id字符串形式
  uid_t uid();                                                      // 获取用户id
  string username();                                                // 获取用户名
  uid_t euid();                                                     // 有效的用户id，用来确定进程对某些资源和文件的访问权限
  Timestamp startTime();                                            // 进程的启动时间
  int clockTicksPerSecond();                                        // 获取时钟滴答的频率 -- 1秒中产生时钟滴答的次数
  int pageSize();                                                   // ??
  bool isDebugBuild();  // constexpr                                // 判断是不是为了debug而创建的进程

  string hostname();                                                // 获取主机名
  string procname();                                                // 获取进程名，返回值为std::string
  StringPiece procname(const string& stat);                         // 获取进程名，返回值为StringPiece

  /// read /proc/self/status                                        -- 读/proc/self/status文件
  string procStatus();                                              // 获取进程状态

  /// read /proc/self/stat                                          -- 读/proc/self/stat文件
  string procStat();                                                // 获取进程状态

  /// read /proc/self/task/tid/stat                                 // 读/proc/self/task/tid/stat
  string threadStat();                                              // 获取线程状态

  /// readlink /proc/self/exe                                       -- readlink用来找出符号链接所指向的位置，这里readlink /proc/self/exe是找到该进程可执行文件的位置
  string exePath();

  int openedFiles();                                                // 获取进程当前打开的文件个数
  int maxOpenFiles();                                               // 获取进程打开文件的最大个数

  struct CpuTime
  {
    double userSeconds;
    double systemSeconds;

    CpuTime() : userSeconds(0.0), systemSeconds(0.0) { }

    double total() const { return userSeconds + systemSeconds; }
  };
  CpuTime cpuTime();

  int numThreads();
  std::vector<pid_t> threads();
}  // namespace ProcessInfo

}  // namespace muduo

#endif  // MUDUO_BASE_PROCESSINFO_H
