// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_ATOMIC_H
#define MUDUO_BASE_ATOMIC_H

#include "muduo/base/noncopyable.h"

#include <stdint.h>

namespace muduo
{

namespace detail
{
template<typename T>                                                        // 模板类
class AtomicIntegerT : noncopyable                                          // 封装具有原子操作的整型，不可拷贝
{
 public:
  AtomicIntegerT()
    : value_(0)
  {
  }

  // uncomment if you need copying and assignment
  //
  // AtomicIntegerT(const AtomicIntegerT& that)
  //   : value_(that.get())
  // {}
  //
  // AtomicIntegerT& operator=(const AtomicIntegerT& that)
  // {
  //   getAndSet(that.get());
  //   return *this;
  // }

  T get()                                                                       // 获取值
  {
    // in gcc >= 4.7: __atomic_load_n(&value_, __ATOMIC_SEQ_CST)          	    // C++11提供的操作方式
    return __sync_val_compare_and_swap(&value_, 0, 0);                          // C++11之前GCC提供的原子操作方式，原子比较和交换操作
  }

  T getAndAdd(T x)                                                      	    // 获取值之后加x，注意返回的是加x之前的值
  {
    // in gcc >= 4.7: __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST)
    return __sync_fetch_and_add(&value_, x);                            	    // 原子自增操作
  }

  T addAndGet(T x)                                                      	    // 先加后获取，返回的是加x之后的值
  {
    return getAndAdd(x) + x;
  }

  T incrementAndGet()                                                           // 先加1后返回值
  {
    return addAndGet(1);
  }

  T decrementAndGet()
  {
    return addAndGet(-1);
  }

  void add(T x)
  {
    getAndAdd(x);
  }

  void increment()
  {
    incrementAndGet();
  }

  void decrement()
  {
    decrementAndGet();
  }

  T getAndSet(T newValue)                                                       // 设置为新值，注意返回的是设置之前的值
  {
    // in gcc >= 4.7: __atomic_exchange_n(&value, newValue, __ATOMIC_SEQ_CST)
    return __sync_lock_test_and_set(&value_, newValue);
  }

 private:
  volatile T value_;                                                            // 记录类对应的实际的值，volatile指定value_的值从内存中读取而不是寄存器中的备份
};
}  // namespace detail

typedef detail::AtomicIntegerT<int32_t> AtomicInt32;                            // 32位的具有原子操作的整型
typedef detail::AtomicIntegerT<int64_t> AtomicInt64;                            // 64位的具有原子操作的整型

}  // namespace muduo

#endif  // MUDUO_BASE_ATOMIC_H
