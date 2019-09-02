// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREADLOCALSINGLETON_H
#define MUDUO_BASE_THREADLOCALSINGLETON_H

#include "muduo/base/noncopyable.h"

#include <assert.h>
#include <pthread.h>

namespace muduo
{

template<typename T>
class ThreadLocalSingleton : noncopyable                                        // 该类的意义是线程特定的单例类
{
 public:
  ThreadLocalSingleton() = delete;
  ~ThreadLocalSingleton() = delete;

  static T& instance()                                                          // 获取唯一的T实例
  {
    if (!t_value_)                                                              // 如果之前没有实例化T
    {
      t_value_ = new T();
      deleter_.set(t_value_);                                                   // 将Deleter对象创建的key与t_value_所指向T实例关联
    }
    return *t_value_;
  }

  static T* pointer()                                                           // 获取唯一的T实例指针
  {
    return t_value_;
  }

 private:
  static void destructor(void* obj)                                             // pthread_key_create()中注册的用于销毁线程特定数据的函数
  {
    assert(obj == t_value_);
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
    T_must_be_complete_type dummy; (void) dummy;
    delete t_value_;
    t_value_ = 0;
  }

  class Deleter
  {
   public:
    Deleter()
    {
      pthread_key_create(&pkey_, &ThreadLocalSingleton::destructor);
    }

    ~Deleter()
    {
      pthread_key_delete(pkey_);
    }

    void set(T* newObj)
    {
      assert(pthread_getspecific(pkey_) == NULL);
      pthread_setspecific(pkey_, newObj);
    }

    pthread_key_t pkey_;
  };

  static __thread T* t_value_;                                                  // 用来存放线程特定的单例类的唯一实例
  static Deleter deleter_;                                                      // 用于析构T的实例
};

template<typename T>
__thread T* ThreadLocalSingleton<T>::t_value_ = 0;                              // 定义并初始化

template<typename T>
typename ThreadLocalSingleton<T>::Deleter ThreadLocalSingleton<T>::deleter_;    // 定义并初始化

}  // namespace muduo
#endif  // MUDUO_BASE_THREADLOCALSINGLETON_H
