// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_TIMER_H
#define MUDUO_NET_TIMER_H

#include "muduo/base/Atomic.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"

namespace muduo
{
namespace net
{

///
/// Internal class for timer event.                                 // 提供timer event使用的内部类，不对外暴露，
///
class Timer : noncopyable
{
 public:
  Timer(TimerCallback cb, Timestamp when, double interval)
    : callback_(std::move(cb)),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0),                                      // 间隔时间大于0，表示需要重复触发
      sequence_(s_numCreated_.incrementAndGet())
  { }

  void run() const
  {
    callback_();
  }

  Timestamp expiration() const  { return expiration_; }             // 获取到期时间
  bool repeat() const { return repeat_; }
  int64_t sequence() const { return sequence_; }                    // 

  void restart(Timestamp now);                                      // 重新开始定时

  static int64_t numCreated() { return s_numCreated_.get(); }       // ??

 private:
  const TimerCallback callback_;    // 定时器到
  Timestamp expiration_;            // 到期时间
  const double interval_;           // 时间间隔
  const bool repeat_;               // 是否是重复定时器
  const int64_t sequence_;          // 定时器序号，每个定时器都有一个唯一的序号

  static AtomicInt64 s_numCreated_; // 记录当前创建定时器的个数
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_TIMER_H
