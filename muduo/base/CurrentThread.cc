// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/CurrentThread.h"

#include <cxxabi.h>                                                                 // __cxa_demangle
#include <execinfo.h>                                                               // backtrace backtrace_symbols
#include <stdlib.h>

namespace muduo
{
namespace CurrentThread
{                                                                                   // __thread是GCC内置的线程局部存储设施，__thread修饰的变量在每个线程中都有独立的一份，线程之间互不干扰     
__thread int t_cachedTid = 0;                                                       // 用来存放线程tid
__thread char t_tidString[32];                                                      // 用来存放tid字符串形式，比如"1234"
__thread int t_tidStringLength = 6;                                                 // tid字符串形式的长度，最大65535，所以长度为6
__thread const char* t_threadName = "unknown";                                      // 线程名，默认是unknown
static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");              // is_same用于判断两个类型是否一致

string stackTrace(bool demangle)                                                    // 参数demangle用来指示是否进行demangle
{
  string stack;
  const int max_frames = 200;
  void* frame[max_frames];
  int nptrs = ::backtrace(frame, max_frames);                                       // 获取堆栈追踪信息
  char** strings = ::backtrace_symbols(frame, nptrs);                               // 转换为字符串
  if (strings)
  {
    size_t len = 256;
    char* demangled = demangle ? static_cast<char*>(::malloc(len)) : nullptr;
    for (int i = 1; i < nptrs; ++i)  // skipping the 0-th, which is this function
    {
      if (demangle)
      {
        // https://panthema.net/2008/0901-stacktrace-demangled/
        // bin/exception_test(_ZN3Bar4testEv+0x79) [0x401909]
        char* left_par = nullptr;                                                   // left_par和plus用于划分出编译器改变之后的函数名
        char* plus = nullptr;
        for (char* p = strings[i]; *p; ++p)
        {
          if (*p == '(')
            left_par = p;
          else if (*p == '+')
            plus = p;
        }

        if (left_par && plus)
        {
          *plus = '\0';
          int status = 0;
          char* ret = abi::__cxa_demangle(left_par+1, demangled, &len, &status);     // 进行demangle 
          *plus = '+';
          if (status == 0)
          {
            demangled = ret;  // ret could be realloc()
            stack.append(strings[i], left_par+1);
            stack.append(demangled);
            stack.append(plus);
            stack.push_back('\n');
            continue;
          }
        }
      }
      // Fallback to mangled names
      stack.append(strings[i]);
      stack.push_back('\n');
    }
    free(demangled);
    free(strings);
  }
  return stack;
}

}  // namespace CurrentThread
}  // namespace muduo
