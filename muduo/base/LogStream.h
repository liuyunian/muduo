// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_LOGSTREAM_H
#define MUDUO_BASE_LOGSTREAM_H

#include "muduo/base/noncopyable.h"
#include "muduo/base/StringPiece.h"
#include "muduo/base/Types.h"
#include <assert.h>
#include <string.h> // memcpy

namespace muduo
{

namespace detail
{

const int kSmallBuffer = 4000;                                                      // 小缓冲区的大小为4000字节
const int kLargeBuffer = 4000*1000;                                                 // 大缓冲区的大小为4000*1000字节

template<int SIZE>                                                                  // 类模板可以接收非类型模板参数
class FixedBuffer : noncopyable                                                     // 固定缓冲区类
{
 public:
  FixedBuffer()
    : cur_(data_)                                                                   // 可添加元素位置指向数组首位置
  {
    setCookie(cookieStart);
  }

  ~FixedBuffer()
  {
    setCookie(cookieEnd);
  }

  void append(const char* /*restrict*/ buf, size_t len)
  {
    // FIXME: append partially
    if (implicit_cast<size_t>(avail()) > len)                                       // 如果data_能容纳下传入参数的长度
    {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
  }

  const char* data() const { return data_; }
  int length() const { return static_cast<int>(cur_ - data_); }

  // write to data_ directly
  char* current() { return cur_; }                                                  // 返回cur_所指向的位置
  int avail() const { return static_cast<int>(end() - cur_); }                      // 返回data_数组可用空间大小
  void add(size_t len) { cur_ += len; }                                             // 向后移动cur_所指向的位置

  void reset() { cur_ = data_; }                                                    // 重置cur_指向的位置
  void bzero() { memZero(data_, sizeof data_); }

  // for used by GDB
  const char* debugString();
  void setCookie(void (*cookie)()) { cookie_ = cookie; }
  // for used by unit test
  string toString() const { return string(data_, length()); }
  StringPiece toStringPiece() const { return StringPiece(data_, length()); }

 private:
  const char* end() const { return data_ + sizeof data_; }
  // Must be outline function for cookies.
  static void cookieStart();
  static void cookieEnd();

  void (*cookie_)();
  char data_[SIZE];                                                                 // 容器是字符数组
  char* cur_;                                                                       // 指向数组可添加元素位置
};

}  // namespace detail

class LogStream : noncopyable                                                       // LogStream日志流操作类
{
  typedef LogStream self;
 public:
  typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;                         // 小缓冲区类型

  self& operator<<(bool v)                                                          // LogStream类内重载<<操作符函数 -- bool类型
  {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
  }

  self& operator<<(short);                                                          // LogStream类内重载<<操作符函数 -- short类型
  self& operator<<(unsigned short);                                                 // LogStream类内重载<<操作符函数 -- unsigned short类型
  self& operator<<(int);                                                            // LogStream类内重载<<操作符函数 -- int类型
  self& operator<<(unsigned int);                                                   // LogStream类内重载<<操作符函数 -- unsigned int类型
  self& operator<<(long);                                                           // LogStream类内重载<<操作符函数 -- long类型
  self& operator<<(unsigned long);                                                  // LogStream类内重载<<操作符函数 -- unsigned long类型
  self& operator<<(long long);                                                      // LogStream类内重载<<操作符函数 -- long long类型
  self& operator<<(unsigned long long);                                             // LogStream类内重载<<操作符函数 -- unsigned long long类型

  self& operator<<(const void*);                                                    // LogStream类内重载<<操作符函数 -- 指针类型

  self& operator<<(float v)                                                         // LogStream类内重载<<操作符函数 -- float类型
  {
    *this << static_cast<double>(v);                                                // 转发成double类型之后调用operator<<(double);
    return *this;
  }
  self& operator<<(double);                                                         // LogStream类内重载<<操作符函数 -- double类型
  // self& operator<<(long double);

  self& operator<<(char v)                                                          // LogStream类内重载<<操作符函数 -- char类型
  {
    buffer_.append(&v, 1);
    return *this;
  }

  // self& operator<<(signed char);
  // self& operator<<(unsigned char);

  self& operator<<(const char* str)                                                 // LogStream类内重载<<操作符函数 -- 字符串（char数组）类型
  {
    if (str)
    {
      buffer_.append(str, strlen(str));
    }
    else
    {
      buffer_.append("(null)", 6);
    }
    return *this;
  }

  self& operator<<(const unsigned char* str)                                        // LogStream类内重载<<操作符函数 -- unsigned char数组类型
  {
    return operator<<(reinterpret_cast<const char*>(str));                          // 转换成字符串（char数组）类型之后调用operator<<(const char* str)
  }

  self& operator<<(const string& v)                                                 // LogStream类内重载<<操作符函数 -- std::string类型
  {
    buffer_.append(v.c_str(), v.size());
    return *this;
  }

  self& operator<<(const StringPiece& v)                                            // LogStream类内重载<<操作符函数 -- StringPiece类型
  {
    buffer_.append(v.data(), v.size());
    return *this;
  }

  self& operator<<(const Buffer& v)                                                 // LogStream类内重载<<操作符函数 -- FixedBuffer<detail::kSmallBuffer>类型
  {
    *this << v.toStringPiece();
    return *this;
  }

  void append(const char* data, int len) { buffer_.append(data, len); }
  const Buffer& buffer() const { return buffer_; }
  void resetBuffer() { buffer_.reset(); }

 private:
  void staticCheck();

  template<typename T>
  void formatInteger(T);

  Buffer buffer_;

  static const int kMaxNumericSize = 32;                                                // 数字所占的最大位数
};

class Fmt // : noncopyable
{
 public:
  template<typename T>
  Fmt(const char* fmt, T val);

  const char* data() const { return buf_; }
  int length() const { return length_; }

 private:
  char buf_[32];
  int length_;
};

inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
{
  s.append(fmt.data(), fmt.length());
  return s;
}

// Format quantity n in SI units (k, M, G, T, P, E).
// The returned string is atmost 5 characters long.
// Requires n >= 0
string formatSI(int64_t n);

// Format quantity n in IEC (binary) units (Ki, Mi, Gi, Ti, Pi, Ei).
// The returned string is atmost 6 characters long.
// Requires n >= 0
string formatIEC(int64_t n);

}  // namespace muduo

#endif  // MUDUO_BASE_LOGSTREAM_H
