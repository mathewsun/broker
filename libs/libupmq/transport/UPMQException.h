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

#ifndef _UPMQ_EXCEPTIONS_UPMQEXCEPTION_H_
#define _UPMQ_EXCEPTIONS_UPMQEXCEPTION_H_

#ifdef _MSC_VER
#pragma warning(disable : 4250)
#endif

#include <cms/CMSException.h>
#include <decaf/lang/Exception.h>
#include <transport/Config.h>
#include <transport/ExceptionDefines.h>

namespace upmq {
namespace transport {

/*
 * Base class for all exceptions.
 */
class UPMQCPP_API UPMQException : public decaf::lang::Exception {
 public:
  /**
   * Default Constructor
   */
  UPMQException();

  /**
   * Copy Constructor
   *
   * @param ex
   *      The Exception whose internal data is copied into this instance.
   */
  UPMQException(const UPMQException &ex);

  /**
   * Copy Constructor
   *
   * @param ex
   *      The Exception whose internal data is copied into this instance.
   */
  UPMQException(const decaf::lang::Exception &ex);

  /**
   * Constructor - Initializes the file name and line number where
   * this message occurred.  Sets the message to report, using an
   * optional list of arguments to parse into the message.
   *
   * @param file
   *      The file name where exception occurs.
   * @param lineNumber
   *      The line number where the exception occurred.
   * @param msg
   *      The message to report.
   * @param ...
   *      The list of primitives that are formatted into the message.
   */
  UPMQException(const char *file, const int lineNumber, const char *msg, ...);

  /**
   * Constructor - Initializes the file name and line number where
   * this message occurred.  Sets the message to report, using an
   * optional list of arguments to parse into the message.
   *
   * @param file
   *      The file name where exception occurs
   * @param lineNumber
   *      The line number where the exception occurred.
   * @param cause
   *      The exception that was the cause for this one to be thrown.
   * @param msg
   *      The message to report
   * @param ...
   *      list of primitives that are formatted into the message
   */
  UPMQException(const char *file, const int lineNumber, const std::exception *cause, const char *msg, ...);

  virtual ~UPMQException() throw();

  /**
   * Clones this exception.  This is useful for cases where you need
   * to preserve the type of the original exception as well as the message.
   * All subclasses should override.
   *
   * @return Copy of this Exception object
   */
  virtual UPMQException *clone() const;

  /**
   * Converts this exception to a new CMSException
   *
   * @return a CMSException with the data from this exception
   */
  virtual cms::CMSException convertToCMSException() const;
};
}  // namespace transport
}  // namespace upmq

#endif /*_UPMQ_EXCEPTIONS_UPMQEXCEPTION_H_*/
