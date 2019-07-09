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
 
#ifndef _DECAF_LANG_EXCEPTIONS_INVALIDSTATEEXCEPTION_H_
#define _DECAF_LANG_EXCEPTIONS_INVALIDSTATEEXCEPTION_H_

#include <decaf/lang/Exception.h>

namespace decaf {
namespace lang {
namespace exceptions {

/*
 * Thrown when an operation is requested, but the state of the object
 * servicing the request is not correct for that request.
 *
 * @since 1.0
 */

DECAF_DECLARE_EXCEPTION(DECAF_API, InvalidStateException, Exception)

}  // namespace exceptions
}  // namespace lang
}  // namespace decaf

#endif /*_DECAF_LANG_EXCEPTIONS_INVALIDSTATEEXCEPTION_H_*/