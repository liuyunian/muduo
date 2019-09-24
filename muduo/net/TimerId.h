// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_TIMERID_H
#define MUDUO_NET_TIMERID_H

#include "muduo/base/copyable.h"

namespace muduo
{
namespace net
{

class Timer;                                                // 前向声明

///
/// An opaque identifier, for canceling Timer.              -- 一个不透明的类，也就是向外部暴露，用于取消定时器
///
class TimerId : public muduo::copyable                      // 可以拷贝
{
 public:
  TimerId()
    : timer_(NULL),
      sequence_(0)
  {
  }

  TimerId(Timer* timer, int64_t seq)
    : timer_(timer),
      sequence_(seq)
  {
  }

  // default copy-ctor, dtor and assignment are okay        -- 采用默认的拷贝构造函数，析构函数和赋值运算符函数

  friend class TimerQueue;                                  // 友元类 -- 在TimerQueue中可以访问TimerId类的private成员

 private:
  Timer* timer_;        // 定时器对象指针
  int64_t sequence_;    // 定时器序号
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_TIMERID_H
