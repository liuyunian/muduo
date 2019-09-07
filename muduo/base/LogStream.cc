// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/LogStream.h"

#include <algorithm>
#include <limits>
#include <type_traits>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>


using namespace muduo;
using namespace muduo::detail;

// TODO: better itoa.
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wtautological-compare"
#else
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

namespace muduo
{
namespace detail
{

const char digits[] = "9876543210123456789";                                            // 十进制数字数组，
const char* zero = digits + 9;                                                  
static_assert(sizeof(digits) == 20, "wrong number of digits");                          // 算上尾部的\0是20字节

const char digitsHex[] = "0123456789ABCDEF";                                            // 十六进制数字数组
static_assert(sizeof digitsHex == 17, "wrong number of digitsHex");                     // 算上尾部的\0是17字节

// Efficient Integer to String Conversions, by Matthew Wilson.                          -- 高效的将整型转换成字符串函数
template<typename T>
size_t convert(char buf[], T value)                                                     //  
{
  T i = value;
  char* p = buf;

  do
  {
    int lsd = static_cast<int>(i % 10);                                                 // 取最后一位
    i /= 10;
    *p++ = zero[lsd];                                                                   // *p = zero[lsd]; ++p;
  } while (i != 0);                                                                     // 从数字的低位取到高位

  if (value < 0)                                                                        // 如果为负数，最后填一个负号
  { 
    *p++ = '-';
  }
  *p = '\0';
  std::reverse(buf, p);                                                                 // 字符串翻转

  return p - buf;
}

size_t convertHex(char buf[], uintptr_t value)                                          // 将16进制数转换成字符串，存放在buf中，思路同十进制整型的转换
{
  uintptr_t i = value;
  char* p = buf;

  do
  {
    int lsd = static_cast<int>(i % 16);
    i /= 16;
    *p++ = digitsHex[lsd];
  } while (i != 0);

  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

}  // namespace detail

/*
 Format a number with 5 characters, including SI units.
 [0,     999]
 [1.00k, 999k]
 [1.00M, 999M]
 [1.00G, 999G]
 [1.00T, 999T]
 [1.00P, 999P]
 [1.00E, inf)
*/
std::string formatSI(int64_t s)
{
  double n = static_cast<double>(s);
  char buf[64];
  if (s < 1000)
    snprintf(buf, sizeof(buf), "%" PRId64, s);
  else if (s < 9995)
    snprintf(buf, sizeof(buf), "%.2fk", n/1e3);
  else if (s < 99950)
    snprintf(buf, sizeof(buf), "%.1fk", n/1e3);
  else if (s < 999500)
    snprintf(buf, sizeof(buf), "%.0fk", n/1e3);
  else if (s < 9995000)
    snprintf(buf, sizeof(buf), "%.2fM", n/1e6);
  else if (s < 99950000)
    snprintf(buf, sizeof(buf), "%.1fM", n/1e6);
  else if (s < 999500000)
    snprintf(buf, sizeof(buf), "%.0fM", n/1e6);
  else if (s < 9995000000)
    snprintf(buf, sizeof(buf), "%.2fG", n/1e9);
  else if (s < 99950000000)
    snprintf(buf, sizeof(buf), "%.1fG", n/1e9);
  else if (s < 999500000000)
    snprintf(buf, sizeof(buf), "%.0fG", n/1e9);
  else if (s < 9995000000000)
    snprintf(buf, sizeof(buf), "%.2fT", n/1e12);
  else if (s < 99950000000000)
    snprintf(buf, sizeof(buf), "%.1fT", n/1e12);
  else if (s < 999500000000000)
    snprintf(buf, sizeof(buf), "%.0fT", n/1e12);
  else if (s < 9995000000000000)
    snprintf(buf, sizeof(buf), "%.2fP", n/1e15);
  else if (s < 99950000000000000)
    snprintf(buf, sizeof(buf), "%.1fP", n/1e15);
  else if (s < 999500000000000000)
    snprintf(buf, sizeof(buf), "%.0fP", n/1e15);
  else
    snprintf(buf, sizeof(buf), "%.2fE", n/1e18);
  return buf;
}

/*
 [0, 1023]
 [1.00Ki, 9.99Ki]
 [10.0Ki, 99.9Ki]
 [ 100Ki, 1023Ki]
 [1.00Mi, 9.99Mi]
*/
std::string formatIEC(int64_t s)
{
  double n = static_cast<double>(s);
  char buf[64];
  const double Ki = 1024.0;
  const double Mi = Ki * 1024.0;
  const double Gi = Mi * 1024.0;
  const double Ti = Gi * 1024.0;
  const double Pi = Ti * 1024.0;
  const double Ei = Pi * 1024.0;

  if (n < Ki)
    snprintf(buf, sizeof buf, "%" PRId64, s);
  else if (n < Ki*9.995)
    snprintf(buf, sizeof buf, "%.2fKi", n / Ki);
  else if (n < Ki*99.95)
    snprintf(buf, sizeof buf, "%.1fKi", n / Ki);
  else if (n < Ki*1023.5)
    snprintf(buf, sizeof buf, "%.0fKi", n / Ki);

  else if (n < Mi*9.995)
    snprintf(buf, sizeof buf, "%.2fMi", n / Mi);
  else if (n < Mi*99.95)
    snprintf(buf, sizeof buf, "%.1fMi", n / Mi);
  else if (n < Mi*1023.5)
    snprintf(buf, sizeof buf, "%.0fMi", n / Mi);

  else if (n < Gi*9.995)
    snprintf(buf, sizeof buf, "%.2fGi", n / Gi);
  else if (n < Gi*99.95)
    snprintf(buf, sizeof buf, "%.1fGi", n / Gi);
  else if (n < Gi*1023.5)
    snprintf(buf, sizeof buf, "%.0fGi", n / Gi);

  else if (n < Ti*9.995)
    snprintf(buf, sizeof buf, "%.2fTi", n / Ti);
  else if (n < Ti*99.95)
    snprintf(buf, sizeof buf, "%.1fTi", n / Ti);
  else if (n < Ti*1023.5)
    snprintf(buf, sizeof buf, "%.0fTi", n / Ti);

  else if (n < Pi*9.995)
    snprintf(buf, sizeof buf, "%.2fPi", n / Pi);
  else if (n < Pi*99.95)
    snprintf(buf, sizeof buf, "%.1fPi", n / Pi);
  else if (n < Pi*1023.5)
    snprintf(buf, sizeof buf, "%.0fPi", n / Pi);

  else if (n < Ei*9.995)
    snprintf(buf, sizeof buf, "%.2fEi", n / Ei );
  else
    snprintf(buf, sizeof buf, "%.1fEi", n / Ei );
  return buf;
}

}  // namespace muduo

template<int SIZE>
const char* FixedBuffer<SIZE>::debugString()
{
  *cur_ = '\0';
  return data_;
}

template<int SIZE>
void FixedBuffer<SIZE>::cookieStart()
{
}

template<int SIZE>
void FixedBuffer<SIZE>::cookieEnd()
{
}

void LogStream::staticCheck()
{
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10,           // std::numeric_limits<double>::digits10 -- double类型数据在十进制下的最大位数 -- 15位
                "kMaxNumericSize is large enough");
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10,      // std::numeric_limits<long double>::digits10 -- long double类型数据在十进制下的最大位数 -- 15位
                "kMaxNumericSize is large enough");
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10,             // std::numeric_limits<long>::digits10 -- long类型数据在十进制下的最大位数 -- 9位
                "kMaxNumericSize is large enough");
  static_assert(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10,        // std::numeric_limits<long long>::digits10 -- long long类型数据在十进制下的最大位数
                "kMaxNumericSize is large enough");
}

template<typename T>
void LogStream::formatInteger(T v)
{
  if (buffer_.avail() >= kMaxNumericSize)                                               // 如果缓冲区的可用空间大于等于数字所占的最大位数
  {
    size_t len = convert(buffer_.current(), v);                                         // 将整型数据转换成字符串类型
    buffer_.add(len);                                                                   // 移动指针cur_，让其指向空闲的位置
  }
}

LogStream& LogStream::operator<<(short v)
{
  *this << static_cast<int>(v);                                                         // 将short类型转化成int类型，再调用operator<<(int v)
  return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
  *this << static_cast<unsigned int>(v);                                                // 将unsigned short类型转化成unsigned int类型，再调用operator<<(unsigned int v)
  return *this;
}

LogStream& LogStream::operator<<(int v)
{
  formatInteger(v);                                                                     // 格式化整型
  return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
  formatInteger(v);                                                                     // 处理方式同int类型
  return *this;
}

LogStream& LogStream::operator<<(long v)
{
  formatInteger(v);                                                                     // 处理方式同int类型
  return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
  formatInteger(v);                                                                     // 处理方式同int类型
  return *this;
}

LogStream& LogStream::operator<<(long long v)
{
  formatInteger(v);                                                                     // 处理方式同int类型
  return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
  formatInteger(v);                                                                     // 处理方式同int类型
  return *this;
}

LogStream& LogStream::operator<<(const void* p)                                         // 输出指针变量的值，格式：0x124e123
{
  uintptr_t v = reinterpret_cast<uintptr_t>(p);                                         // 强制类型，将void *转换成uintptr_t
  if (buffer_.avail() >= kMaxNumericSize)
  {
    char* buf = buffer_.current();
    buf[0] = '0';
    buf[1] = 'x';
    size_t len = convertHex(buf+2, v);                                                  // 将16进制数转换成字符串
    buffer_.add(len+2);
  }
  return *this;
}

// FIXME: replace this with Grisu3 by Florian Loitsch.
LogStream& LogStream::operator<<(double v)
{
  if (buffer_.avail() >= kMaxNumericSize)
  {
    int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);                 // 使用snprintf将double浮点数转换成字符串
    buffer_.add(len);
  }
  return *this;
}

template<typename T>
Fmt::Fmt(const char* fmt, T val)
{
  static_assert(std::is_arithmetic<T>::value == true, "Must be arithmetic type");

  length_ = snprintf(buf_, sizeof buf_, fmt, val);
  assert(static_cast<size_t>(length_) < sizeof buf_);
}

// Explicit instantiations

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);
