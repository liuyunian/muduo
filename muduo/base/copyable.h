#ifndef MUDUO_BASE_COPYABLE_H
#define MUDUO_BASE_COPYABLE_H

namespace muduo
{

/// A tag class emphasises the objects are copyable.      	-- 标记类，强调该类及其子类是可以拷贝的
/// The empty base class optimization applies.           	-- 用于优化应用
/// Any derived class of copyable should be a value type. 	-- 该类的任何基类应是值类型

class copyable
{
 protected:
  copyable() = default;                           			// 让编译器自动添加函数体
  ~copyable() = default;
};

}  // namespace muduo

#endif  // MUDUO_BASE_COPYABLE_H
