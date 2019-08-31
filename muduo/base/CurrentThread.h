// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_CURRENTTHREAD_H
#define MUDUO_BASE_CURRENTTHREAD_H

#include "muduo/base/Types.h"

namespace muduo
{
namespace CurrentThread                             // CurrentThread命名空间，这里使用有点像类，.h文件中声明函数，源文件中定义函数，注意这里的函数定义分散在不同的源文件中
{
  // internal                                       -- 内部使用
  extern __thread int t_cachedTid;
  extern __thread char t_tidString[32];
  extern __thread int t_tidStringLength;
  extern __thread const char* t_threadName;
  void cacheTid();                                  // 缓存tid，函数在Thread.cc中定义 

  inline int tid()
  {
    if (__builtin_expect(t_cachedTid == 0, 0))      /* __builtin_expect(表达式, 标志)，标志位0表示执行该分支的可能性小，标志位1表示可能性大
                                                     * 作用是：允许程序员将最有可能执行的分支告诉编译器
                                                     * 解决的痛点是：CPU取值执行时有流水线机制，也就是执行指令的同时会取下一条指令，但是如果正在执行的指令是跳转指令那么预先取到的下一条指令就不一定有用
                                                     * 因为跳转之后需要取新位置的指令执行，所以说跳转指令降低了CPU的效率
                                                     * 
                                                     * 在程序中，分支语句会产生跳转指令，所以如果人为的告诉编译器最有可能执行的分支，那编译器在编译时可以调整编译出的汇编代码，从而使得跳转指令不生效的概率加大
                                                    */

                                                    // 这里这个分支预测其执行的可能性小的的原因是：只需要在第一次调用tid()时通过cacheTid获取到tid，之后在调用tid()是就不需要再调用cacheTid获取了
    {
      cacheTid();
    }
    return t_cachedTid;
  }

  inline const char* tidString() // for logging     // 返回tid字符串形式，用在日志打印中
  {
    return t_tidString;
  }

  inline int tidStringLength() // for logging       // 返回tid字符串形式的长度，用在日志打印中
  {
    return t_tidStringLength;
  }

  inline const char* name()
  {
    return t_threadName;
  }

  bool isMainThread();                              // 判断是不是主线程，在Thread.cc中实现

  void sleepUsec(int64_t usec);  // for testing     // 线程休眠，在Thread.cc中实现

  string stackTrace(bool demangle);
}  // namespace CurrentThread
}  // namespace muduo

#endif  // MUDUO_BASE_CURRENTTHREAD_H
