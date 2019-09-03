// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_COUNTDOWNLATCH_H
#define MUDUO_BASE_COUNTDOWNLATCH_H

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

namespace muduo
{

class CountDownLatch : noncopyable              // 倒计数门闩类，两种用法：用于所有子线程等待主线程发起“开始运行”命令；或者用于主线程等待子线程初始化完毕再继续运行
{
 public:

  explicit CountDownLatch(int count);           // 初始化传入一个计数器

  void wait();                                  // 等待计数器减为0

  void countDown();                             // 计数器减1

  int getCount() const;                         // 获取计数器的值

 private:
  mutable MutexLock mutex_;
  Condition condition_ GUARDED_BY(mutex_);
  int count_ GUARDED_BY(mutex_);                // 计数器
};

}  // namespace muduo
#endif  // MUDUO_BASE_COUNTDOWNLATCH_H
