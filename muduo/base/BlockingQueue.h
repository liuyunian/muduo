// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_BLOCKINGQUEUE_H
#define MUDUO_BASE_BLOCKINGQUEUE_H

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

#include <deque>
#include <assert.h>

namespace muduo
{

template<typename T>                                                                        // 模板类
class BlockingQueue : noncopyable                                                           // 无界阻塞队列
{
 public:
  BlockingQueue()
    : mutex_(),
      notEmpty_(mutex_),
      queue_()
  {
  }

  void put(const T& x)                                                                      // 往队列中放置一个元素，const引用
  {
    MutexLockGuard lock(mutex_);
    queue_.push_back(x);
    notEmpty_.notify(); // wait morphing saves us                                           -- 队列非空，唤醒消费
    // http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
  }

  void put(T&& x)                                                                           // 函数重载，往队列中放置一个元素，右值引用，用于接收右值
  {
    MutexLockGuard lock(mutex_);
    queue_.push_back(std::move(x));                                                         // std::move()将左值转换为右值
    notEmpty_.notify();
  }

  T take()                                                                                  // 从队列中拿出一个元素
  {
    MutexLockGuard lock(mutex_);
    // always use a while-loop, due to spurious wakeup
    while (queue_.empty())
    {
      notEmpty_.wait();
    }
    assert(!queue_.empty());
    T front(std::move(queue_.front()));
    queue_.pop_front();
    return std::move(front);
  }

  size_t size() const                                                                       // 获取队列中元素的个数
  {
    MutexLockGuard lock(mutex_);
    return queue_.size();
  }

 private:
  mutable MutexLock mutex_;                                                                 // 互斥量，用于保护队列
  Condition         notEmpty_ GUARDED_BY(mutex_);                                           // 使用条件变量实现，队列非空
  std::deque<T>     queue_ GUARDED_BY(mutex_);                                              // 容器使用STL中的std::deque容器
};

}  // namespace muduo

#endif  // MUDUO_BASE_BLOCKINGQUEUE_H
