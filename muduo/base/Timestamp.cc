// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/Timestamp.h"

#include <sys/time.h>																		// gmtime_r
#include <stdio.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS																// C++中使用PRId64的话必须要#define__STDC_FORMAT_MACROS
#endif

#include <inttypes.h>																		// PRId64

using namespace muduo;

static_assert(sizeof(Timestamp) == sizeof(int64_t),
              "Timestamp is same size as int64_t");

string Timestamp::toString() const
{
  char buf[32] = {0};																		// 表示微秒占6个字符，dot占一个字符，末尾的/0占一个字符，所以秒部分最多占24个字符，完全足够使用了
  int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
  snprintf(buf, sizeof(buf)-1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);			// PRId64: int64_t表示64位有符号整数，在32位系统上是long long，在64位系统中是long，所以在32位系统中打印的格式是%lld，在64位系统中打印格式位%ld
  return buf;																				// 所以如果要跨平台打印in64_t就要使用一个统一的格式，也就是这里的PRId64z，其本质是字符串"lld"(32bit) or "ld"(64bit)
}

string Timestamp::toFormattedString(bool showMicroseconds) const
{
  char buf[64] = {0};
  time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); 	// time_t本质是long int
  struct tm tm_time;																		// 用于保存时间和日期
  gmtime_r(&seconds, &tm_time);																// 将time_t类型表示的时间分解填充到struct tm结构中，与之类似的gmtime(const time_t * timer)函数，不用提供自行定义的struct tm对象

  if (showMicroseconds)
  {
    int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
             microseconds);
  }
  else
  {
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }
  return buf;
}

Timestamp Timestamp::now()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t seconds = tv.tv_sec;
  return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}

