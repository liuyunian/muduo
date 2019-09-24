// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include <set>
#include <vector>

#include "muduo/base/Mutex.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/Channel.h"

namespace muduo
{
namespace net
{

class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.                                             -- 一个高效的定时器队列，定期器管理类
/// No guarantee that the callback will be on time.                         -- 不能保证回调函数准时执行 
///
class TimerQueue : noncopyable
{
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  ///
  /// Schedules the callback to be run at given time,
  /// repeats if @c interval > 0.0.
  ///
  /// Must be thread safe. Usually be called from other threads.            // 必须是线程安全的，通常情况下会被其他线程调用
  TimerId addTimer(TimerCallback cb,
                   Timestamp when,
                   double interval);                                        // 向定时器队列中添加定时器，返回的是一个TimerId实例

  void cancel(TimerId timerId);                                             // 取消指定的定时器

 private:

  // FIXME: use unique_ptr<Timer> instead of raw pointers.                  // 用std::unique_ptr<Timer>代替Timer *生指针
  // This requires heterogeneous comparison lookup (N3465) from C++14
  // so that we can find an T* in a set<unique_ptr<T>>.
  typedef std::pair<Timestamp, Timer*> Entry;                               // 采用数据对方式存储一个定时器，以Timestamp时间戳排序
  typedef std::set<Entry> TimerList;                                        // 选用set容器存储定时器，采用pair + set可以存储时间戳相同的定时器，而std::map不允许key相同
  typedef std::pair<Timer*, int64_t> ActiveTimer;
  typedef std::set<ActiveTimer> ActiveTimerSet;                             // 活跃的定时器集合，和TimerList都保存了所有的定时器，不同是排序方式不同

  // 只可能在所属的IO线程中调用，因而不用加锁
  void addTimerInLoop(Timer* timer);
  void cancelInLoop(TimerId timerId);

  // called when timerfd alarms
  void handleRead();
  // move out all expired timers
  std::vector<Entry> getExpired(Timestamp now);                             // 获取当前时间超时的定时器
  void reset(const std::vector<Entry>& expired, Timestamp now);             // 

  bool insert(Timer* timer);

  EventLoop* loop_;                     // 记录所属EventLoop
  const int timerfd_;                   // 定时器文件描述符
  Channel timerfdChannel_;              // timerfd所属的Channel，用于响应定时器事件
  // Timer list sorted by expiration    -- 定时器列表，按照到期时间排序
  TimerList timers_;

  // for cancel()
  ActiveTimerSet activeTimers_;         // timers_与activeTimers_中保存的是相同的数据，activeTimers_按照到期时间排序
  bool callingExpiredTimers_; /* atomic */
  ActiveTimerSet cancelingTimers_;      // 保存被取消的定时器
};

}  // namespace net
}  // namespace muduo
#endif  // MUDUO_NET_TIMERQUEUE_H
