// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_SINGLETON_H
#define MUDUO_BASE_SINGLETON_H

#include "muduo/base/noncopyable.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h> // atexit

namespace muduo
{

namespace detail
{
// This doesn't detect inherited member functions!                                              -- 这不会检测继承的成员函数
// http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions
template<typename T>
struct has_no_destroy                                                                           // 使用SFINAE检测是否有特定的成员函数
{
  template <typename C> static char test(decltype(&C::no_destroy));
  template <typename C> static int32_t test(...);
  const static bool value = sizeof(test<T>(0)) == 1;
};
}  // namespace detail

template<typename T>                                                                             // 模板类
class Singleton : noncopyable                                                                    // 用于其他的类改造成单例类，返回传入参数
{
 public:
  Singleton() = delete;                                                                         // 禁用构造函数
  ~Singleton() = delete;                                                                        // 禁用析构函数

  static T& instance()
  {
    pthread_once(&ponce_, &Singleton::init);                                                    // Singleto::init函数只被执行一次
    assert(value_ != NULL);
    return *value_;
  }

 private:
  static void init()
  {
    value_ = new T();                                                                           // 创建T实例，该实例是唯一一个实例
    if (!detail::has_no_destroy<T>::value)
    {
      ::atexit(destroy);                                                                        // 使用atexit()函数注册进程退出时要执行的函数 -- destory
    }
  }

  static void destroy()
  {
    typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];                              // 限制T必须是完成的类型
                                                                                                /**
                                                                                                 * class A;
                                                                                                 * A * p_a = nullptr;
                                                                                                 * 上面也是可以编译通过的，但是类A不是完整的类型，对于这样的类，不能作为Singleton模板参数
                                                                                                */
    T_must_be_complete_type dummy; (void) dummy;

    delete value_;
    value_ = NULL;
  }

 private:
  static pthread_once_t ponce_;                                                                 // pthread_once参数
  static T*             value_;                                                                 // 指向T唯一的实例
};

template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;                                        // 初始化为PTHREAD_ONCE_INIT，保证了在多线程环境下只被执行一次

template<typename T>
T* Singleton<T>::value_ = NULL;

}  // namespace muduo

#endif  // MUDUO_BASE_SINGLETON_H
