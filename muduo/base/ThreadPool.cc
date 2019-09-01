// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/ThreadPool.h"

#include "muduo/base/Exception.h"

#include <assert.h>
#include <stdio.h>

using namespace muduo;

ThreadPool::ThreadPool(const string& nameArg)
  : mutex_(),
    notEmpty_(mutex_),
    notFull_(mutex_),
    name_(nameArg),
    maxQueueSize_(0),
    running_(false)
{
}

ThreadPool::~ThreadPool()
{
  if (running_)                                                                 // 析构函数中，如果线程池正在运行，就停止
  {
    stop();
  }
}

void ThreadPool::start(int numThreads)
{
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(numThreads);                                                 // 设置存放线程对象的vector的容量
  for (int i = 0; i < numThreads; ++i)
  {
    char id[32];
    snprintf(id, sizeof id, "%d", i+1);
    threads_.emplace_back(new muduo::Thread(
          std::bind(&ThreadPool::runInThread, this), name_+id));                // 创建Thread线程对象，并将其添加到容器中，线程执行的函数是runInThread
    threads_[i]->start();
  }
  if (numThreads == 0 && threadInitCallback_)                                   // 如果指定线程池的大小为0，并且threadInitCallback_不为空的话，就执行threadInitCallback_
  {
    threadInitCallback_();
  }
}

void ThreadPool::stop()
{
  {
  MutexLockGuard lock(mutex_);
  running_ = false;
  notEmpty_.notifyAll();
  }
  for (auto& thr : threads_)
  {
    thr->join();
  }
}

size_t ThreadPool::queueSize() const
{
  MutexLockGuard lock(mutex_);
  return queue_.size();
}

void ThreadPool::run(Task task)                                                     // 向任务队列中添加任务
{
  if (threads_.empty())                                                             // 如果线程池中没有线程，那么就在添加任务的线程中执行该任务
  {
    task();
  }
  else
  {
    MutexLockGuard lock(mutex_);
    while (isFull())                                                                // 当任务队列已满时，阻塞等待唤醒
    {
      notFull_.wait();
    }
    assert(!isFull());

    queue_.push_back(std::move(task));
    notEmpty_.notify();                                                             // 唤醒线程池中的线程，取任务执行
  }
}

ThreadPool::Task ThreadPool::take()
{
  MutexLockGuard lock(mutex_);
  // always use a while-loop, due to spurious wakeup
  while (queue_.empty() && running_)                                                // 当任务队列为空，线程池处于运行状态时，线程阻塞，等待唤醒
  {
    notEmpty_.wait();
  }
  Task task;
  if (!queue_.empty())
  {
    task = queue_.front();
    queue_.pop_front();
    if (maxQueueSize_ > 0)
    {
      notFull_.notify();                                                            // 取走任务后，任务队列不满，唤醒添加任务的线程
    }
  }
  return task;
}

bool ThreadPool::isFull() const
{
  mutex_.assertLocked();
  return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread()
{
  try
  {
    if (threadInitCallback_)
    {
      threadInitCallback_();
    }
    while (running_)
    {
      Task task(take());                                                            // 从任务队列中取任务
      if (task)
      {
        task();
      }
    }
  }
  catch (const Exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
    abort();
  }
  catch (const std::exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch (...)
  {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
    throw; // rethrow
  }
}

