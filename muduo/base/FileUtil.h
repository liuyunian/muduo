// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_BASE_FILEUTIL_H
#define MUDUO_BASE_FILEUTIL_H

#include "muduo/base/noncopyable.h"
#include "muduo/base/StringPiece.h"
#include <sys/types.h>  // for off_t

namespace muduo
{
namespace FileUtil
{

// read small file < 64KB                                                           // 读取小文件 < 64k字节
class ReadSmallFile : noncopyable
{
 public:
  ReadSmallFile(StringArg filename);
  ~ReadSmallFile();

  // return errno
  template<typename String>
  int readToString(int maxSize,                                                     // maxSize读取文件内容的最大数
                   String* content,                                                 // 用于存放读取文件内容
                   int64_t* fileSize,                                               // 记录文件大小
                   int64_t* modifyTime,                                             // 记录文件的修改时间
                   int64_t* createTime);                                            // 记录文件的创建时间

  /// Read at maxium kBufferSize into buf_
  // return errno
  int readToBuffer(int* size);                                                      // 读取文件内容到buffer缓冲区

  const char* buffer() const { return buf_; }

  static const int kBufferSize = 64*1024;                                           // buffer缓冲区的大小

 private:
  int fd_;
  int err_;
  char buf_[kBufferSize];
};

// read the file content, returns errno if error happens.
template<typename String>
int readFile(StringArg filename,
             int maxSize,
             String* content,
             int64_t* fileSize = NULL,
             int64_t* modifyTime = NULL,
             int64_t* createTime = NULL)
{
  ReadSmallFile file(filename);
  return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
}

// not thread safe                                                                  -- 非线程安全
class AppendFile : noncopyable                                                      // 向文件中追加内容
{
 public:
  explicit AppendFile(StringArg filename);

  ~AppendFile();

  void append(const char* logline, size_t len);

  void flush();

  off_t writtenBytes() const { return writtenBytes_; }                              // 获取已向文件写入的字节数

 private:

  size_t write(const char* logline, size_t len);

  FILE* fp_;
  char buffer_[64*1024];
  off_t writtenBytes_;                                                              // 记录已向日志文件中写入的字节数
};

}  // namespace FileUtil
}  // namespace muduo

#endif  // MUDUO_BASE_FILEUTIL_H

