#ifndef MUDUO_BASE_COPYABLE_H
#define MUDUO_BASE_COPYABLE_H

namespace muduo
{

/// A tag class emphasises the objects are copyable. -- 标记类，强调该类及其子类是可以拷贝的
/// The empty base class optimization applies. -- 
/// Any derived class of copyable should be a value type. -- 该类的任何基类应是值类型

/**
 * 值语义和对象语义
 * 值语义：对象被拷贝之后，拷贝出来的对象和原对象之间毫无关系
 * 基本数据类型：整型、浮点型、指针等都是值语义；
 * 自定义类型：不包含资源的自定义类型，系统提供的缺省拷贝构造函数与赋值操作符亦保证了值语义；包含资源的自定义类型，需要提供深拷贝操作的拷贝构造函数和赋值操作符，并在构造函数中获取资源，在析构函数中释放资源
 * 
 * 对象语义（指针语义、引用语义）：对象不能拷贝或者对象被拷贝之后，拷贝出来的对象和原对象之间共享底层资源，对任何一个改变都将改变另一个
 * 自定义类型：包含资源的自定义类型，没有提供拷贝构造函数和赋值操作符，或者在拷贝构造函数和赋值操作符中有意共享资源，则此时的对象具有指针语义
 * 
 * https://blog.csdn.net/chdhust/article/details/9927083
*/

class copyable
{
 protected:
  copyable() = default; // 让编译器自动添加函数体
  ~copyable() = default;
};

}  // namespace muduo

#endif  // MUDUO_BASE_COPYABLE_H
