// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/Thread.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/base/Exception.h"
#include "muduo/base/Logging.h"

#include <type_traits>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>                                                                  // prctl
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace muduo
{
namespace detail
{

/**
在linux下每一个进程都一个进程id，类型pid_t，可以由getpid（）获取
POSIX线程也有线程id，类型pthread_t，可以由pthread_self（）获取，线程id由线程库维护。但是各个进程独立，所以会有不同进程中线程号相同节的情况
那么这样就会存在一个问题，我的进程p1中的线程t1要与进程p2中的线程t2通信怎么办，进程id不可以，线程id又可能重复，所以这里会有一个真实的线程id唯一标识，tid。
glibc没有实现gettid的函数，所以我们可以通过linux下的系统调用syscall(SYS_gettid)来获得
*/

pid_t gettid()                                                                          // 获取真实的线程id
{
  return static_cast<pid_t>(::syscall(SYS_gettid));                                     // 使用syscall(SYS_gettid)获取线程真实的id    
}

void afterFork()
{
  muduo::CurrentThread::t_cachedTid = 0;
  muduo::CurrentThread::t_threadName = "main";
  CurrentThread::tid();
  // no need to call pthread_atfork(NULL, NULL, &afterFork);
}

class ThreadNameInitializer                                                             // 用于多线程中fork子进程的情况
{
 public:
  ThreadNameInitializer()
  {
    muduo::CurrentThread::t_threadName = "main";
    CurrentThread::tid();
    pthread_atfork(NULL, NULL, &afterFork);
  }
};

ThreadNameInitializer init;

struct ThreadData                                                                       // 线程数据结构，作为pthread_create()参数，线程入口函数的参数
{
  typedef muduo::Thread::ThreadFunc ThreadFunc;                             
  ThreadFunc func_;                                                                     
  string name_;
  pid_t* tid_;
  CountDownLatch* latch_;

  ThreadData(ThreadFunc func,
             const string& name,
             pid_t* tid,
             CountDownLatch* latch)
    : func_(std::move(func)),
      name_(name),
      tid_(tid),
      latch_(latch)
  { }

  void runInThread()                                                                    // 从该函数中执行线程任务函数func_
  {
    *tid_ = muduo::CurrentThread::tid();                                                // 获取当前线程的tid，存放在Thread成员变量tid_中
    tid_ = NULL;                                                                        // ThreadData结构中的指针成员tid_置为空指针
    latch_->countDown();                                                                // ??
    latch_ = NULL;                                                                      // ??

    muduo::CurrentThread::t_threadName = name_.empty() ? "muduoThread" : name_.c_str(); // 这里的name_怎么会为空呢？？在此之前muduo::CurrentThread::t_threadName为unknown
    ::prctl(PR_SET_NAME, muduo::CurrentThread::t_threadName);                           // 设置进程的名字为t_hreadName
    try
    {
      func_();
      muduo::CurrentThread::t_threadName = "finished";
    }
    catch (const Exception& ex)
    {
      muduo::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
      abort();
    }
    catch (const std::exception& ex)
    {
      muduo::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    }
    catch (...)
    {
      muduo::CurrentThread::t_threadName = "crashed";
      fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
      throw; // rethrow
    }
  }
};

void* startThread(void* obj)
{
  ThreadData* data = static_cast<ThreadData*>(obj);
  data->runInThread();
  delete data;                                                                              // data是new出来的，所以要delete
  return NULL;
}

}  // namespace detail

void CurrentThread::cacheTid()
{
  if (t_cachedTid == 0)
  {
    t_cachedTid = detail::gettid();                                                         // 通过gettid获取真实的线程id -- tid
    t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);     // tid的字符串形式
  }
}

bool CurrentThread::isMainThread()
{
  return tid() == ::getpid();                                                               // 线程tid == 进程id，那么该线程为主线程
}

void CurrentThread::sleepUsec(int64_t usec)
{
  struct timespec ts = { 0, 0 };
  ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
  ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
  ::nanosleep(&ts, NULL);
}

AtomicInt32 Thread::numCreated_;                                                            // 静态成员变量numCreated_初始化                   

Thread::Thread(ThreadFunc func, const string& n)
  : started_(false),
    joined_(false),
    pthreadId_(0),
    tid_(0),
    func_(std::move(func)),
    name_(n),
    latch_(1)
{
  setDefaultName();
}

Thread::~Thread()
{
  if (started_ && !joined_)
  {
    pthread_detach(pthreadId_);                                                             // 线程分离，由系统回收线程资源
  }
}

void Thread::setDefaultName()                                                               // 设置线程默认名字
{
  int num = numCreated_.incrementAndGet();
  if (name_.empty())
  {
    char buf[32];
    snprintf(buf, sizeof buf, "Thread%d", num);                                             // sizeof可以不加括号使用吗，线程默认名的给格式：Thread1、Thread2...
    name_ = buf;
  }
}

void Thread::start()
{
  assert(!started_);
  started_ = true;
  // FIXME: move(func_)
  detail::ThreadData* data = new detail::ThreadData(func_, name_, &tid_, &latch_);
  if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))                        // 创建线程，采用默认线程属性，创建成功返回0，否则返回错误码
  {
    started_ = false;
    delete data; // or no delete?
    LOG_SYSFATAL << "Failed in pthread_create";
  }
  else                                                                                      // 线程创建成功
  {
    latch_.wait();                                                                          // ??
    assert(tid_ > 0);                                                                       // 线程创建成功之后，tid_必然 > 0
  }
}

int Thread::join()
{
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, NULL);
}

}  // namespace muduo
