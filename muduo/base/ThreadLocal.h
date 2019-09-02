// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREADLOCAL_H
#define MUDUO_BASE_THREADLOCAL_H

#include "muduo/base/Mutex.h"
#include "muduo/base/noncopyable.h"

#include <pthread.h>

namespace muduo
{

template<typename T>
class ThreadLocal : noncopyable                                         // 对线程特定数据进行封装
{
 public:
  ThreadLocal()
  {
    MCHECK(pthread_key_create(&pkey_, &ThreadLocal::destructor));       // 构造函数中创建key
  }

  ~ThreadLocal()
  {
    MCHECK(pthread_key_delete(pkey_));                                  // 析构函数中取消key与特定数据的关联
  }

  T& value()                                                            // 获取pkey_中保存的key关联的线程特定数据
  {
    T* perThreadValue = static_cast<T*>(pthread_getspecific(pkey_));
    if (!perThreadValue)                                                // 表示没有值与key关联，此时调用pthread_setspecific进行关联
    {
      T* newObj = new T();
      MCHECK(pthread_setspecific(pkey_, newObj));
      perThreadValue = newObj;
    }
    return *perThreadValue;
  }

 private:

  static void destructor(void *x)                                       // 用于析构线程特定数据的函数，通过pthread_key_create()进行注册
  {
    T* obj = static_cast<T*>(x);
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];      // 保证传入的T类型是完整类型
    T_must_be_complete_type dummy; (void) dummy;
    delete obj;
  }

 private:
  pthread_key_t pkey_;                                                  // 保存与线程特定数据关联的key
};

}  // namespace muduo

#endif  // MUDUO_BASE_THREADLOCAL_H
