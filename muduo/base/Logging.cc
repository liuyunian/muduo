// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/Logging.h"

#include "muduo/base/CurrentThread.h"
#include "muduo/base/Timestamp.h"
#include "muduo/base/TimeZone.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

namespace muduo
{

/*
class LoggerImpl
{
 public:
  typedef Logger::LogLevel LogLevel;
  LoggerImpl(LogLevel level, int old_errno, const char* file, int line);
  void finish();

  Timestamp time_;
  LogStream stream_;
  LogLevel level_;
  int line_;
  const char* fullname_;
  const char* basename_;
};
*/

__thread char t_errnobuf[512];                                                                      // 线程特定数据，用于存放errno对应的字符串说明
__thread char t_time[64];                                                                           // 线程特定数据，用于存放格式化之后的时间戳
__thread time_t t_lastSecond;

const char* strerror_tl(int savedErrno)                                                             // 获取errno对应的字符串说明
{
  return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);                                     // strerror_r用于获取errno字符串说明
}

Logger::LogLevel initLogLevel()                                                                     // 初始化日志级别
{
  if (::getenv("MUDUO_LOG_TRACE"))                                                                  // 如果定义了环境变量MUDUO_LOG_TRACE，日志级别设置为TRACE
    return Logger::TRACE;
  else if (::getenv("MUDUO_LOG_DEBUG"))                                                             // 如果定义了环境变量MUDUO_LOG_DEBUG，日志级别设置为DEBUG
    return Logger::DEBUG;
  else
    return Logger::INFO;                                                                            // 没有进行环境变量定义的话，默认的日志级别为INFO
}

Logger::LogLevel g_logLevel = initLogLevel();                                                       // 保存设置的日志级别    

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "WARN  ",
  "ERROR ",
  "FATAL ",
};                                                                                                  // 日志级别，字符串数组，用枚举元素的值作为数组index

// helper class for known string length at compile time                                             -- 帮助类，为了知道字符串的长度和编译时间
class T                                                                                             // 将字符串和字符串长度封装在一起
{
 public:
  T(const char* str, unsigned len)
    :str_(str),
     len_(len)
  {
    assert(strlen(str) == len_);
  }

  const char* str_;
  const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v)                                                     // 重载全局的<<操作符??
{
  s.append(v.str_, v.len_);
  return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)                             // 函数重载
{
  s.append(v.data_, v.size_);
  return s;
}

void defaultOutput(const char* msg, int len)                                                        // 默认输出到stdout标准输出
{
  size_t n = fwrite(msg, 1, len, stdout);
  //FIXME check n
  (void)n;                                                                                          // 好多地方都有这样的写法，这是因为变量n没有使用，所以编译时会有警告，加此写法可以防止警告
}

void defaultFlush()                                                                                 // 默认刷新的也是标准输出
{
  fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
TimeZone g_logTimeZone;

}  // namespace muduo

using namespace muduo;

Logger::Impl::Impl(LogLevel level, int savedErrno, const SourceFile& file, int line)                // Impl构造函数
  : time_(Timestamp::now()),                                                                        // 时间戳设置为当前时间
    stream_(),
    level_(level), 
    line_(line),
    basename_(file)
{
  formatTime();                                                                                     // 格式化时间
  CurrentThread::tid();                                                                             // 获取当前线程的tid
  stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());                       // 输出线程tid
  stream_ << T(LogLevelName[level], 6);                                                             // 输出日志级别
  if (savedErrno != 0)                                                                              // errno不为0时输出"errno说明 (errno=xxx)"
  {
    stream_ << strerror_tl(savedErrno) << " (errno=" << savedErrno << ") ";
  }
}

void Logger::Impl::formatTime()
{
  int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
  time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / Timestamp::kMicroSecondsPerSecond);
  int microseconds = static_cast<int>(microSecondsSinceEpoch % Timestamp::kMicroSecondsPerSecond);
  if (seconds != t_lastSecond)
  {
    t_lastSecond = seconds;
    struct tm tm_time;
    if (g_logTimeZone.valid())
    {
      tm_time = g_logTimeZone.toLocalTime(seconds);
    }
    else
    {
      ::gmtime_r(&seconds, &tm_time); // FIXME TimeZone::fromUtcTime
    }

    int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
        tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
        tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    assert(len == 17); (void)len;
  }

  if (g_logTimeZone.valid())
  {
    Fmt us(".%06d ", microseconds);
    assert(us.length() == 8);
    stream_ << T(t_time, 17) << T(us.data(), 8);
  }
  else
  {
    Fmt us(".%06dZ ", microseconds);
    assert(us.length() == 9);
    stream_ << T(t_time, 17) << T(us.data(), 9);
  }
}

void Logger::Impl::finish()
{
  stream_ << " - " << basename_ << ':' << line_ << '\n';                                            // 在日志信息的最后添加所在的源文件和行号
}

Logger::Logger(SourceFile file, int line)
  : impl_(INFO, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
  : impl_(level, 0, file, line)
{
  impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
  : impl_(level, 0, file, line)
{
}

Logger::Logger(SourceFile file, int line, bool toAbort)
  : impl_(toAbort?FATAL:ERROR, errno, file, line)
{
}

Logger::~Logger()
{
  impl_.finish();
  const LogStream::Buffer& buf(stream().buffer());
  g_output(buf.data(), buf.length());
  if (impl_.level_ == FATAL)                                                                        // 如果日志级别是致命错误
  {
    g_flush();                                                                                      // 立刻刷新，这样将输出缓冲区中的内容输出到stdout或者文件
    abort();                                                                                        // 终止程序
  }
}

void Logger::setLogLevel(Logger::LogLevel level)
{
  g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
  g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
  g_flush = flush;
}

void Logger::setTimeZone(const TimeZone& tz)
{
  g_logTimeZone = tz;
}

/**
 * 函数调用流程
 * 以 LOG_INFO << "logInThread"; 为例分析
 * 
    LOG_INFO << "logInThread";
    = muduo::Logger(__FILE__, __LINE__).stream()
        logger() -- 构造函数
        Impl() -- 构造函数
            formatTime()
            CurrentThread::tid();
            stream_ << tid
            stream_ << loglevel
                LogStream& operator<<(LogStream& s, T v)
                    s.append();
                        ...
                T(const char* str, unsigned len) -- 构造函数
            stream_ << errno
                LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
                    s.append();
                        ...
                SourceFile() -- 构造函数
        stream()
        LogStream& operator<<(LogStream& s, const Fmt& fmt)
            ...
        ~Logger() -- 构造产生的是Logger临时对象，所以该语句执行完之后会析构Logger对象
            impl_.finish();
            LogStream::buffer()
                ...
            g_output() -- 输出函数，可以输出到stdout或者文件
 */