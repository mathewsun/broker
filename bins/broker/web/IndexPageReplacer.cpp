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

#include "IndexPageReplacer.h"
#include "Defines.h"

IndexPageReplacer::IndexPageReplacer(std::string pageName, TemplateParamReplacer *templateReplacer) : TemplateParamReplacer(std::move(pageName)), _templateReplacer(templateReplacer) {
  addReplacer(MakeStringify(indexH1), (TemplateParamReplacer::Callback)&IndexPageReplacer::h1Replacer);
  addReplacer(MakeStringify(indexContent), (TemplateParamReplacer::Callback)&IndexPageReplacer::contentReplacer);
}

std::string IndexPageReplacer::h1Replacer() { return _templateReplacer->getH1(); }

std::string IndexPageReplacer::contentReplacer() { return _templateReplacer->replace(); }

void IndexPageReplacer::setChildReplacer(TemplateParamReplacer *templateReplacer) { _templateReplacer = templateReplacer; }