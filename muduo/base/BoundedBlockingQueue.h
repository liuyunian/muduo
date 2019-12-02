// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H
#define MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

#include <boost/circular_buffer.hpp>
#include <assert.h>

namespace muduo
{

template<typename T>
class BoundedBlockingQueue : noncopyable                        // 无界阻塞队列
{
 public:
  explicit BoundedBlockingQueue(int maxSize)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      queue_(maxSize)
  {
  }

  void put(const T& x)                                      // 往队列中放置一个元素
  {
    MutexLockGuard lock(mutex_);
    while (queue_.full())                                   // 如果队列已满，阻塞等待唤醒
    {
      notFull_.wait();
    }
    assert(!queue_.full());
    queue_.push_back(x);
    notEmpty_.notify();                                     // 成功放置一个元素之后，队列非空，唤醒一个消费者线程
  }

  void put(T&& x)                                           // 函数重载
  {
    MutexLockGuard lock(mutex_);
    while (queue_.full())
    {
      notFull_.wait();
    }
    assert(!queue_.full());
    queue_.push_back(std::move(x));
    notEmpty_.notify();
  }

  T take()                                                  // 从队列中取走一个元素
  {
    MutexLockGuard lock(mutex_);                            // 加锁保护
    while (queue_.empty())                                  // 队列为空时，阻塞休眠，等待唤醒
    {
      notEmpty_.wait();
    }
    assert(!queue_.empty());
    T front(std::move(queue_.front()));
    queue_.pop_front();
<<<<<<< HEAD
    notFull_.notify();                                      // 取出一个元素之后，队列非满，唤醒一个生产者线程
    return std::move(front);
=======
    notFull_.notify();
    return front;
>>>>>>> 0f3cb975510ea11d984f0fb03c8f5ea776c64f3a
  }

  bool empty() const                                        // 判断队列是否满
  {
    MutexLockGuard lock(mutex_);
    return queue_.empty();
  }

  bool full() const                                         // 判断队列是否满
  {
    MutexLockGuard lock(mutex_);
    return queue_.full();
  }

  size_t size() const                                       // 队列存放元素的个数
  {
    MutexLockGuard lock(mutex_);
    return queue_.size();
  }

  size_t capacity() const                                   // 队列的容量
  {
    MutexLockGuard lock(mutex_);
    return queue_.capacity();
  }

 private:
  mutable MutexLock          mutex_;
  Condition                  notEmpty_ GUARDED_BY(mutex_);  // 表示队列非空的条件变量
  Condition                  notFull_ GUARDED_BY(mutex_);   // 表示队列非满的条件变量
  boost::circular_buffer<T>  queue_ GUARDED_BY(mutex_);     // 容器使用boost库中的环形缓冲区
};

}  // namespace muduo

#endif  // MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H
