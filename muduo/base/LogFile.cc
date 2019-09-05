// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/LogFile.h"

#include "muduo/base/FileUtil.h"
#include "muduo/base/ProcessInfo.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

using namespace muduo;

LogFile::LogFile(const string& basename,
                 off_t rollSize,
                 bool threadSafe,
                 int flushInterval,
                 int checkEveryN)
  : basename_(basename),
    rollSize_(rollSize),
    flushInterval_(flushInterval),
    checkEveryN_(checkEveryN),
    count_(0),
    mutex_(threadSafe ? new MutexLock : NULL),
    startOfPeriod_(0),
    lastRoll_(0),
    lastFlush_(0)
{
  assert(basename.find('/') == string::npos);                                // 断言basename中不包含'/'
  rollFile();
}

LogFile::~LogFile() = default;                                              // 采用编译器提供的析构函数体

void LogFile::append(const char* logline, int len)
{
  if (mutex_)
  {
    MutexLockGuard lock(*mutex_);
    append_unlocked(logline, len);
  }
  else
  {
    append_unlocked(logline, len);
  }
}

void LogFile::flush()
{
  if (mutex_)
  {
    MutexLockGuard lock(*mutex_);
    file_->flush();
  }
  else
  {
    file_->flush();
  }
}

void LogFile::append_unlocked(const char* logline, int len)
{
  file_->append(logline, len);

  if (file_->writtenBytes() > rollSize_)                                    // 如果已向日志文件写入的字节数大于日志滚动门限值，此时应该进行日志滚动
  {
    rollFile();
  }
  else
  {
    ++count_;                                                               // 每调用一次LogFile::append()获取LogFile::append_unlocked()，count_计数器都会加1
    if (count_ >= checkEveryN_)                                             // 如果count_计数器达到了门限值，需要判断是否进行flush(将内核输出缓存区的内容写到硬盘文件上)
    {
      count_ = 0;
      time_t now = ::time(NULL);
      time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (thisPeriod_ != startOfPeriod_)                                    // 表示到了下一天的0点，此时需要进行日志滚动
      {
        rollFile();
      }
      else if (now - lastFlush_ > flushInterval_)                           // 时间间隔大于刷新间隔，此时要进行flush
      {
        lastFlush_ = now;
        file_->flush();
      }
    }
  }
}

bool LogFile::rollFile()
{
  time_t now = 0;
  string filename = getLogFileName(basename_, &now);
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;                     // 记录当天0点对应时间

  if (now > lastRoll_)                                                          // 当前的时间大于上一次日志滚动的时间
  {
    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;
    file_.reset(new FileUtil::AppendFile(filename));                            // unique_ptr智能指针ret()函数先释放原来指向的对象，在指向新new出来的对象
    return true;
  }
  return false;
}

string LogFile::getLogFileName(const string& basename, time_t* now)
{
  string filename;
  filename.reserve(basename.size() + 64);                                       // std::string::reserve()用于设置字符串的容量，这里字符串filename容量设置为至少basename.size + 64字节
  filename = basename;

  char timebuf[32];                                                             // 保存字符串格式的当前时间
  struct tm tm;
  *now = time(NULL);                                                            // 获取当前时间
  gmtime_r(now, &tm); // FIXME: localtime_r ?                                   -- 当前时间用struct tm结构中保存，gmtime()和gmtime_r()区别，gmtime_r()是线程安全的，gmtime()不是线程安全的
  strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);                    // 使用strftime将struct tm结构保存的当前时间转换成字符串格式 -- %Y年、%m月、%d日-%H时、%M分、%S秒
  filename += timebuf;

  filename += ProcessInfo::hostname();                                          // 获取主机名

  char pidbuf[32];
  snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());                   // 进程id
  filename += pidbuf;

  filename += ".log";

  return filename;                                                              // 日志文件名：basename + 当前时间 + 主机名 + 进程id + .log                                                           
}

