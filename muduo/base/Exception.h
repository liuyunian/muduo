// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_EXCEPTION_H
#define MUDUO_BASE_EXCEPTION_H

#include "muduo/base/Types.h"
#include <exception>

namespace muduo
{

class Exception : public std::exception         // 异常类，继承自std::exception
{
 public:
  Exception(string what);                       // 参数字符串
  ~Exception() noexcept override = default;     // noexcept异常指示符，对外宣称不抛出任何异常，override表明该函数覆盖父类对应的函数，这里也就是覆盖父类的析构函数

  // default copy-ctor and operator= are okay.

  const char* what() const noexcept override    // 用于输出异常信息，三个修饰词：const不修改成员变量，noexcept不抛出任何异常、override覆盖父类的同名函数
  {
    return message_.c_str();
  }

  const char* stackTrace() const noexcept       // 输出异常发生时的堆栈信息
  {
    return stack_.c_str();
  }

 private:
  string message_;
  string stack_;
};

}  // namespace muduo

#endif  // MUDUO_BASE_EXCEPTION_H
