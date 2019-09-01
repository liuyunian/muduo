// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREADPOOL_H
#define MUDUO_BASE_THREADPOOL_H

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"
#include "muduo/base/Types.h"

#include <deque>
#include <vector>

namespace muduo
{

class ThreadPool : noncopyable                                          // 线程池类，固定大小的线程池
{
 public:
  typedef std::function<void ()> Task;                                  // 定义任务执行函数的格式void(void)

  explicit ThreadPool(const string& nameArg = string("ThreadPool"));    // 用字符串作为线程池名初始化线程池类，默认名称为ThreadPool
  ~ThreadPool();

  // Must be called before start().                                     -- 在线程池运行之前必须调用
  void setMaxQueueSize(int maxSize) { maxQueueSize_ = maxSize; }        // 设置任务队列的最大值
  void setThreadInitCallback(const Task& cb)                            // 设置线程初始化之后的回调函数，线程初始化之后，可能先去执行一段逻辑，之后再等待从任务队列中取任务执行
  { threadInitCallback_ = cb; }

  void start(int numThreads);                                           // 开始运行线程池，参数numThreads执行线程池中线程的个数
  void stop();                                                          // 线程池停止运行

  const string& name() const                                            // 获取线程池的名字
  { return name_; }

  size_t queueSize() const;                                             // 获取任务队列的大小

  // Could block if maxQueueSize > 0
  // There is no move-only version of std::function in C++ as of C++14.
  // So we don't need to overload a const& and an && versions
  // as we do in (Bounded)BlockingQueue.
  // https://stackoverflow.com/a/25408989
  void run(Task f);                                                     // 向任务队列添加任务

 private:
  bool isFull() const REQUIRES(mutex_);                                 // 判断任务队列是否已满
  void runInThread();                                                   // 线程池创建线程之后，线程的入口函数
  Task take();                                                          // 从任务队列中取任务执行

  mutable MutexLock mutex_;
  Condition notEmpty_ GUARDED_BY(mutex_);                               // 任务队列非空条件变量
  Condition notFull_ GUARDED_BY(mutex_);                                // 任务队列非满条件变量
  string name_;                                                         // 线程池名称
  Task threadInitCallback_;                                             // 线程池初始化之后的回调函数
  std::vector<std::unique_ptr<muduo::Thread>> threads_;                 // 用于存放线程对象
  std::deque<Task> queue_ GUARDED_BY(mutex_);                           // 任务队列，容器采用的是std::deque
  size_t maxQueueSize_;                                                 // 任务队列的最大值
  bool running_;                                                        // 标记线程池是否在运行
};

}  // namespace muduo

#endif  // MUDUO_BASE_THREADPOOL_H
