/*
 * Copyright 2014-present IVK JSC. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _DECAF_UTIL_LOGGING_SIMPLELOGGER_H_
#define _DECAF_UTIL_LOGGING_SIMPLELOGGER_H_

#include <decaf/util/Config.h>
#include <string>

namespace decaf {
namespace util {
namespace logging {

class DECAF_API SimpleLogger {
 public:
  /**
   * Constructor
   */
  SimpleLogger(const std::string &name_);

  /**
   * Destructor
   */
  virtual ~SimpleLogger();

  /**
   * Log a Mark Block Level Log
   */
  virtual void mark(const std::string &message);

  /**
   * Log a Debug Level Log
   */
  virtual void debug(const std::string &file, const int line, const std::string &message);

  /**
   * Log a Informational Level Log
   */
  virtual void info(const std::string &file, const int line, const std::string &message);

  /**
   * Log a Warning Level Log
   */
  virtual void warn(const std::string &file, const int line, const std::string &message);

  /**
   * Log a Error Level Log
   */
  virtual void error(const std::string &file, const int line, const std::string &message);

  /**
   * Log a Fatal Level Log
   */
  virtual void fatal(const std::string &file, const int line, const std::string &message);

  /**
   * No-frills log.
   */
  virtual void log(const std::string &message);

 private:
  // Name of this Logger
  std::string name;
};
}  // namespace logging
}  // namespace util
}  // namespace decaf

#endif /*_DECAF_UTIL_LOGGING_SIMPLELOGGER_H_*/
