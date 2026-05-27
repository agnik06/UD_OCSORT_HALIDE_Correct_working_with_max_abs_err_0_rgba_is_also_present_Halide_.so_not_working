/*!
 *  Copyright (c) 2015 by Contributors
 * \file logger.h
 * \brief defines logging macros of dmlc
 *  allows use of GLOG, fall back to internal
 *  implementation when disabled
 */
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <cstdio>
#include <cstdlib>
#include <string>
#include <string.h>
#include <vector>
#include <stdexcept>

#ifdef __android__
#include <android/log.h>
#endif
#pragma once
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifndef LOG_TAG
#define LOG_TAG "VIZ_UNIFIED_DET_" TOSTRING(__UNIFIED_DETECTOR_VERSION__)
#endif

namespace unified_detector {
/*!
 * \brief exception class that will be thrown by
 *  default logger if DMLC_LOG_FATAL_THROW == 1
 */
struct Error : public std::runtime_error {
  /*!
   * \brief constructor
   * \param s the error message
   */
  explicit Error(const std::string &s) : std::runtime_error(s) {}
};
}  // namespace unified_detector

// use a light version of glog
#include <assert.h>
#include <iostream>
#include <sstream>
#include <ctime>

//TODO: Replace Old version of logger
#ifdef __android__
#include <android/log.h>
#ifndef APPNAME
#define APPNAME "ImageSearchJNI_UNIFIED_DET_" TOSTRING(__UNIFIED_DETECTOR_VERSION__)
#endif
#ifndef LOGV
#define LOGV(fmt, ...)                                              \
  __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "%s: %d: " fmt, \
                      __FILENAME__, __LINE__, ##__VA_ARGS__);
#endif
#else
// #elif __terminal_logging__
#define LOGV(...)                            \
  printf("%s-%d: ", __FILENAME__, __LINE__); \
  printf(__VA_ARGS__);                       \
  printf("\n");
// #else
// #define LOGV(...) ;
#endif


#if defined(_MSC_VER)
#pragma warning(disable : 4722)
#endif

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__,'/') + 1 : __FILE__)

namespace unified_detector {

class LogCheckError {
 public:
  LogCheckError() : str(nullptr) {}
  explicit LogCheckError(const std::string& str_) : str(new std::string(str_)) {}
  ~LogCheckError() { if (str != nullptr) delete str; }
  operator bool() {return str != nullptr; }
  std::string* str;
};

#define DEFINE_CHECK_FUNC(name, op)                               \
  template <typename X, typename Y>                               \
  inline LogCheckError LogCheck##name(const X& x, const Y& y) {   \
    if (x op y) return LogCheckError();                           \
    std::ostringstream os;                                        \
    os << " (" << x << " vs. " << y << ") ";  /* CHECK_XX(x, y) requires x and y can be serialized to string. Use CHECK(x OP y) otherwise. NOLINT(*) */ \
    return LogCheckError(os.str());                               \
  }                                                               \
  inline LogCheckError LogCheck##name(int x, int y) {             \
    return LogCheck##name<int, int>(x, y);                        \
  }

#define CHECK_BINARY_OP(name, op, x, y)                                 \
  if (unified_detector::LogCheckError _check_err = unified_detector::LogCheck##name(x, y))    \
    unified_detector::LogMessageFatal(__FILE__, __LINE__).stream()                 \
      << "Check failed: " << #x " " #op " " #y << *(_check_err.str)

DEFINE_CHECK_FUNC(_LT, <)
DEFINE_CHECK_FUNC(_GT, >)
DEFINE_CHECK_FUNC(_LE, <=)
DEFINE_CHECK_FUNC(_GE, >=)
DEFINE_CHECK_FUNC(_EQ, ==)
DEFINE_CHECK_FUNC(_NE, !=)

// Always-on checking
#define CHECK(x)                                            \
  if (!(x))                                                 \
    unified_detector::LogMessageFatal(__FILE__, __LINE__).stream()     \
      << "Check failed: " #x << ' '
#define CHECK_LT(x, y) CHECK_BINARY_OP(_LT, <, x, y)
#define CHECK_GT(x, y) CHECK_BINARY_OP(_GT, >, x, y)
#define CHECK_LE(x, y) CHECK_BINARY_OP(_LE, <=, x, y)
#define CHECK_GE(x, y) CHECK_BINARY_OP(_GE, >=, x, y)
#define CHECK_EQ(x, y) CHECK_BINARY_OP(_EQ, ==, x, y)
#define CHECK_NE(x, y) CHECK_BINARY_OP(_NE, !=, x, y)
#define CHECK_NOTNULL(x) \
  ((x) == NULL ? unified_detector::LogMessageFatal(__FILE__, __LINE__).stream() << "Check  notnull: "  #x << ' ', (x) : (x)) // NOLINT(*)
// Debug-only checking.
#ifdef NDEBUG
#define DCHECK(x) \
  while (false) CHECK(x)
#define DCHECK_LT(x, y) \
  while (false) CHECK((x) < (y))
#define DCHECK_GT(x, y) \
  while (false) CHECK((x) > (y))
#define DCHECK_LE(x, y) \
  while (false) CHECK((x) <= (y))
#define DCHECK_GE(x, y) \
  while (false) CHECK((x) >= (y))
#define DCHECK_EQ(x, y) \
  while (false) CHECK((x) == (y))
#define DCHECK_NE(x, y) \
  while (false) CHECK((x) != (y))
#else
#define DCHECK(x) CHECK(x)
#define DCHECK_LT(x, y) CHECK((x) < (y))
#define DCHECK_GT(x, y) CHECK((x) > (y))
#define DCHECK_LE(x, y) CHECK((x) <= (y))
#define DCHECK_GE(x, y) CHECK((x) >= (y))
#define DCHECK_EQ(x, y) CHECK((x) == (y))
#define DCHECK_NE(x, y) CHECK((x) != (y))
#endif  // NDEBUG

#ifndef __android__
#define LOG_INFO unified_detector::LogMessage(__FILENAME__, __LINE__)
#define LOG_ERROR LOG_INFO
#define LOG_WARNING LOG_INFO
#define LOG_FATAL unified_detector::LogMessageFatal(__FILENAME__, __LINE__)
#define LOG_QFATAL LOG_FATAL
#endif

#ifdef __android__
#define LOG_INFO unified_detector::LogMessage(ANDROID_LOG_INFO, __FILENAME__, __LINE__)
#define LOG_ERROR unified_detector::LogMessage(ANDROID_LOG_ERROR, __FILENAME__, __LINE__)
#define LOG_WARNING unified_detector::LogMessage(ANDROID_LOG_WARN, __FILENAME__, __LINE__)
#define LOG_FATAL unified_detector::LogMessageFatal(__FILENAME__, __LINE__)
#define LOG_QFATAL LOG_FATAL
#endif

// Poor man version of VLOG
#define VLOG(x) LOG_INFO.stream()<<(x);

#define LOG(severity) LOG_##severity.stream()
#define LG LOG_INFO.stream()
#define LOG_IF(severity, condition) \
  !(condition) ? (void)0 : unified_detector::LogMessageVoidify() & LOG(severity)

#ifdef NDEBUG
#define LOG_DFATAL LOG_ERROR
#define DFATAL ERROR
#define DLOG(severity) true ? (void)0 : unified_detector::LogMessageVoidify() & LOG(severity)
#define DLOG_IF(severity, condition) \
  (true || !(condition)) ? (void)0 : unified_detector::LogMessageVoidify() & LOG(severity)
#else
#define LOG_DFATAL LOG_FATAL
#define DFATAL FATAL
#define DLOG(severity) LOG(severity)
#define DLOG_IF(severity, condition) LOG_IF(severity, condition)
#endif

// Poor man version of LOG_EVERY_N
#define LOG_EVERY_N(severity, n) LOG(severity)

class DateLogger {
 public:
  DateLogger() {
#if defined(_MSC_VER)
    _tzset();
#endif
  }
  const char* HumanDate() {
#if defined(_MSC_VER)
  __time64_t time_value = _time64(nullptr);
  _strtime_s(buffer_, sizeof(buffer_), &time_value);
#else
    time_t time_value = time(NULL);
    struct tm *pnow;
#if !defined(_WIN32)
    struct tm now;
    pnow = localtime_r(&time_value, &now);
#else
    pnow = localtime(&time_value);  // NOLINT(*)
#endif
    snprintf(buffer_, sizeof(buffer_), "%02d:%02d:%02d",
             pnow->tm_hour, pnow->tm_min, pnow->tm_sec);
#endif
    return buffer_;
  }

 private:
  char buffer_[9];
};

#ifndef __android__
class LogMessage {
 public:
  LogMessage(const char* file, int line)
      :
#ifdef __ANDROID__
        log_stream_(std::cout)
#else
        log_stream_(std::cerr)
#endif
  {
    log_stream_ << "[" << pretty_date_.HumanDate() << "] " << file << ":"
                << line << ": ";
  }
  ~LogMessage() { log_stream_ << '\n'; }
  std::ostream& stream() { return log_stream_; }

 protected:
  std::ostream& log_stream_;

 private:
  DateLogger pretty_date_;
  LogMessage(const LogMessage&);
  void operator=(const LogMessage&);
};
#endif

#ifdef __android__
class LogMessage {
 public:
  LogMessage(int prio, const char* file, int line):
    priority(prio){
    log_stream_ << "[" << pretty_date_.HumanDate() << "] " << file << ":"
                << line << ": ";
  }
  LogMessage(const char* file, int line):
    priority(ANDROID_LOG_VERBOSE){
    log_stream_ << "[" << pretty_date_.HumanDate() << "] " << file << ":"
                << line << ": ";
  }
  ~LogMessage() {
    log_stream_ << '\n';
    __android_log_print(priority, LOG_TAG, "%s", log_stream_.str().c_str());
  }
  std::ostringstream& stream() { return log_stream_; }

 protected:
  std::ostringstream log_stream_;

 private:
  int priority;
  DateLogger pretty_date_;
  LogMessage(const LogMessage&);
  void operator=(const LogMessage&);
};
#endif

class LogMessageFatal {
 public:
  LogMessageFatal(const char* file, int line) {
    log_stream_ << "[" << pretty_date_.HumanDate() << "] " << file << ":"
                << line << ": ";
  }
#if defined(_MSC_VER) && _MSC_VER < 1900
  ~LogMessageFatal() {
#else
  ~LogMessageFatal() noexcept(false) {
#endif
    LOG(ERROR) << log_stream_.str();
    throw Error(log_stream_.str());
  }
  std::ostringstream &stream() { return log_stream_; }

 private:
  std::ostringstream log_stream_;
  DateLogger pretty_date_;
  LogMessageFatal(const LogMessageFatal&);
  void operator=(const LogMessageFatal&);
};

// This class is used to explicitly ignore values in the conditional
// logging macros.  This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".
class LogMessageVoidify {
 public:
  LogMessageVoidify() {}
  // This has to be an operator with a precedence lower than << but
  // higher than "?:". See its usage.
  void operator&(std::ostream&) {}
};

}  // namespace unified_detector

#endif  // unified_detector_LOGGER_H_

