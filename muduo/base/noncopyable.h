#ifndef MUDUO_BASE_NONCOPYABLE_H
#define MUDUO_BASE_NONCOPYABLE_H

namespace muduo
{

class noncopyable                               // 同copyable，标记类，作为基类，继承的子类不能拷贝
{
 public:
  noncopyable(const noncopyable&) = delete;     // 禁用拷贝构造函数
  void operator=(const noncopyable&) = delete;  // 禁用赋值运算符函数

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

}  // namespace muduo

#endif  // MUDUO_BASE_NONCOPYABLE_H
