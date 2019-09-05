// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_LOGFILE_H
#define MUDUO_BASE_LOGFILE_H

#include "muduo/base/Mutex.h"
#include "muduo/base/Types.h"

#include <memory>

namespace muduo
{

namespace FileUtil
{
class AppendFile;
}

class LogFile : noncopyable                                                 // 该类用于实现日志滚动
{
 public:
  LogFile(const string& basename,
          off_t rollSize,
          bool threadSafe = true,                                           // 是否需要线程安全，如果是多线程环境下，需要线程安全，如果是单线程使用，则不需要保证线程安全，默认情况是采用线程安全
          int flushInterval = 3,                                            // 指定多少秒将日志写入文件中，默认是3秒
          int checkEveryN = 1024);                                          // 指定计数器count_比较标准
  ~LogFile();

  void append(const char* logline, int len);                                // 加锁方式向日志文件追加内容
  void flush();                                                             // 刷新缓冲区
  bool rollFile();                                                          // 切换日志文件

 private:
  void append_unlocked(const char* logline, int len);                       // 无锁方式向日志文件追加内容

  static string getLogFileName(const string& basename, time_t* now);        // 获取日志文件的名称

  const string basename_;                                                   // /home/lyn/123.log -- basename是指123.log
  const off_t rollSize_;                                                    // 用来指示日志文件多大时切换日志文件 -- 比如1G，off_t类型是
  const int flushInterval_;                                                 // 写到日志文件的时间间隔，并不是每次打印日志都会直接输出到硬盘上，这样效率低
  const int checkEveryN_;                                                   // 计数器比较的标准

  int count_;                                                               // 计数器，和checkEveryN_比较，如果相等就判断是否到了该将日志写到硬盘文件上？是不是文件的大小达到了rollSize_?

  std::unique_ptr<MutexLock> mutex_;
  time_t startOfPeriod_;                                                    // 开始记录日志的时间
  time_t lastRoll_;                                                         // 上一次日志滚动的时间
  time_t lastFlush_;                                                        // 上一次日志写入文件的时间
  std::unique_ptr<FileUtil::AppendFile> file_;                              // AppendFile类智能指针

  const static int kRollPerSeconds_ = 60*60*24;                             // 表示24小时
};

}  // namespace muduo
#endif  // MUDUO_BASE_LOGFILE_H
