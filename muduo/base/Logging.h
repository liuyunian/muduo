// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_LOGGING_H
#define MUDUO_BASE_LOGGING_H

#include "muduo/base/LogStream.h"
#include "muduo/base/Timestamp.h"

namespace muduo
{

class TimeZone;

class Logger                                                                            // 日志类
{
 public:
  enum LogLevel                                                                         // 枚举类型，定义日志级别，范围限制在muduo::Logger
  {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,                                                                     // 这种方式很有意思，6种级别，这里的NUM_LOG_LEVELS正好为6
  };

  // compile time calculation of basename of source file                                -- 源文件基名编译时间计算
  class SourceFile                                                                      // 用于接收__FILE__宏定义的值
  {
   public:
    template<int N>                                                                     // 构造函数还能是模板函数？？，参数是char数组
    SourceFile(const char (&arr)[N])                                                    // 两个构造函数有什么区别呢？
      : data_(arr),
        size_(N-1)
    {
      const char* slash = strrchr(data_, '/'); // builtin function
      if (slash)
      {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }

    explicit SourceFile(const char* filename)
      : data_(filename)
    {
      const char* slash = strrchr(filename, '/');
      if (slash)
      {
        data_ = slash + 1;
      }
      size_ = static_cast<int>(strlen(data_));
    }

    const char* data_;
    int size_;
  };

  Logger(SourceFile file, int line);                                                    // 构造函数：file文件名、line代码行号
  Logger(SourceFile file, int line, LogLevel level);                                    // level日志级别
  Logger(SourceFile file, int line, LogLevel level, const char* func);                  // func函数名
  Logger(SourceFile file, int line, bool toAbort);                                      // 是否调用abort终止程序，abort()会进行核心转储，这样有助于调试
  ~Logger();

  LogStream& stream() { return impl_.stream_; }                                         // 返回impl对象中的stream_成员

  static LogLevel logLevel();
  static void setLogLevel(LogLevel level);                                              // 设置日志级别

  typedef void (*OutputFunc)(const char* msg, int len);
  typedef void (*FlushFunc)();
  static void setOutput(OutputFunc);
  static void setFlush(FlushFunc);
  static void setTimeZone(const TimeZone& tz);

 private:

class Impl                                                                              // 内部类 -- 该类存在的意义是什么？
{
 public:
  typedef Logger::LogLevel LogLevel;
  Impl(LogLevel level, int old_errno, const SourceFile& file, int line);                // 构造函数
  void formatTime();
  void finish();

  Timestamp time_;
  LogStream stream_;                                                                    // LogStream对象
  LogLevel level_;
  int line_;
  SourceFile basename_;                                                                 // 用于记录日志信息所在的文件
};

  Impl impl_;                                                                           // 唯一的成员变量，Impl类对象

};

extern Logger::LogLevel g_logLevel;                                                     // 保存设置的日志级别

inline Logger::LogLevel Logger::logLevel()                                              // 获取日志级别
{
  return g_logLevel;
}

//
// CAUTION: do not write:
//
// if (good)
//   LOG_INFO << "Good news";
// else
//   LOG_WARN << "Bad news";
//
// this expends to
//
// if (good)
//   if (logging_INFO)
//     logInfoStream << "Good news";
//   else
//     logWarnStream << "Bad news";
//                                                                                      -- 在代码中做日志打印时实际使用的就是如下的宏定义
#define LOG_TRACE if (muduo::Logger::logLevel() <= muduo::Logger::TRACE) \
  muduo::Logger(__FILE__, __LINE__, muduo::Logger::TRACE, __func__).stream()            // 编译器内置宏，__FILE__在源文件中插入当前文件的源文件名；__LINE__在源文件中插入当前源文件行号；__func__所在的函数名
#define LOG_DEBUG if (muduo::Logger::logLevel() <= muduo::Logger::DEBUG) \
  muduo::Logger(__FILE__, __LINE__, muduo::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
  muduo::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN muduo::Logger(__FILE__, __LINE__, muduo::Logger::WARN).stream()
#define LOG_ERROR muduo::Logger(__FILE__, __LINE__, muduo::Logger::ERROR).stream()
#define LOG_FATAL muduo::Logger(__FILE__, __LINE__, muduo::Logger::FATAL).stream()
#define LOG_SYSERR muduo::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL muduo::Logger(__FILE__, __LINE__, true).stream()

const char* strerror_tl(int savedErrno);

// Taken from glog/logging.h
//
// Check that the input is non NULL.  This very useful in constructor                   -- 检查输入参数不为NULL，这对于构造函数初始化列表非常有用
// initializer lists.

#define CHECK_NOTNULL(val) \
  ::muduo::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr)           // 检查参数ptr不是NULL，如果ptr为NULL，输出FATAL日志信息
{
  if (ptr == NULL)
  {
   Logger(file, line, Logger::FATAL).stream() << names;
  }
  return ptr;
}

}  // namespace muduo

#endif  // MUDUO_BASE_LOGGING_H
