// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/Condition.h"

#include <errno.h>                                                                                  // ETIMEDOUT，超时返回的错误

// returns true if time out, false otherwise.
bool muduo::Condition::waitForSeconds(double seconds)
{
  struct timespec abstime;                                                                          // 两个成员：秒和纳秒 -- 精确度是纳秒，与之对应的是struct timeval，也是两个成员，秒和微妙 -- 精确度是微秒
  // FIXME: use CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW to prevent time rewind.
  clock_gettime(CLOCK_REALTIME, &abstime);                                                          // 获取系统当前时间

  const int64_t kNanoSecondsPerSecond = 1000000000;                                                 // 10的9次方
  int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);                      // 将传入的等待时间seconds换算成纳秒

  abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec + nanoseconds) / kNanoSecondsPerSecond);
  abstime.tv_nsec = static_cast<long>((abstime.tv_nsec + nanoseconds) % kNanoSecondsPerSecond);

  MutexLock::UnassignGuard ug(mutex_);
  return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);         // 调用pthread_cond_timedwait()函数及时等待
}

