// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREAD_H
#define MUDUO_BASE_THREAD_H

#include "muduo/base/Atomic.h"
#include "muduo/base/CountDownLatch.h"
#include "muduo/base/Types.h"

#include <functional>
#include <memory>
#include <pthread.h>

namespace muduo
{

class Thread : noncopyable                                      // 封装线程类，不可拷贝
{ 
 public:
  typedef std::function<void ()> ThreadFunc;                    // 类型定义

  explicit Thread(ThreadFunc, const string& name = string());   // 构造函数，两个参数：线程入口函数，线程名
  // FIXME: make it movable in C++11                            -- 构造成可移动的
  ~Thread();

  void start();                                                 // 线程开始执行
  int join(); // return pthread_join()

  bool started() const { return started_; }
  // pthread_t pthreadId() const { return pthreadId_; }
  pid_t tid() const { return tid_; }
  const string& name() const { return name_; }

  static int numCreated() { return numCreated_.get(); }

 private:
  void setDefaultName();

  bool       started_;                                          // 表征线程是否开始执行
  bool       joined_;                                           // 表征线程是否被join
  pthread_t  pthreadId_;                                        // 线程id
  pid_t      tid_;                                              // 线程真实id
  ThreadFunc func_;                                             // 线程所要执行的任务函数
  string     name_;                                             // 线程名
  CountDownLatch latch_;                                        // 线程锁 ??

  static AtomicInt32 numCreated_;                               // 32位原子类型，用来记录该类实例的个数，也就是记录多少个线程
};

}  // namespace muduo
#endif  // MUDUO_BASE_THREAD_H
