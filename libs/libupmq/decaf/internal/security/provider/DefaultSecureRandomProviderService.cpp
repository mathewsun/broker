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

#include "DefaultSecureRandomProviderService.h"

#ifdef HAVE_PTHREAD_H
#include <decaf/internal/security/unix/SecureRandomImpl.h>
#else
#include <decaf/internal/security/windows/SecureRandomImpl.h>
#endif

using namespace decaf;
using namespace decaf::security;
using namespace decaf::internal;
using namespace decaf::internal::security;
using namespace decaf::internal::security::provider;

////////////////////////////////////////////////////////////////////////////////
DefaultSecureRandomProviderService::DefaultSecureRandomProviderService(const Provider *provider, const std::string &algorithmName)
    : ProviderService(provider, "SecureRandom", algorithmName) {}

////////////////////////////////////////////////////////////////////////////////
DefaultSecureRandomProviderService::~DefaultSecureRandomProviderService() {}

////////////////////////////////////////////////////////////////////////////////
SecuritySpi *DefaultSecureRandomProviderService::newInstance() { return new SecureRandomImpl(); }
