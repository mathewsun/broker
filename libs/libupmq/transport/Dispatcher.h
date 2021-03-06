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

#ifndef UPMQ_CORE_DISPATCHER_H_
#define UPMQ_CORE_DISPATCHER_H_

#include <decaf/lang/Pointer.h>
#include <transport/Command.h>
#include <transport/Config.h>

namespace upmq {
namespace transport {

/**
 * Interface for an object responsible for dispatching messages to
 * consumers.
 */
class UPMQCPP_API Dispatcher {
 public:
  virtual ~Dispatcher(){};

  /**
   * Dispatches a message to a particular consumer.
   *
   * @param message
   *      The message to be dispatched to a waiting consumer.
   */
  virtual void dispatch(const decaf::lang::Pointer<upmq::transport::Command> &message) = 0;
};
}  // namespace transport
}  // namespace upmq

#endif /*UPMQ_CORE_DISPATCHER_H_*/
