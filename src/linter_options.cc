//
// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "src/linter_options.h"

#include <iostream>
#include <string>
#include <vector>

namespace zetasql::linter {

bool LinterOptions::IsActive(int position) const {
  bool active = active_start_;
  for (int i = 0;
       i < static_cast<int>(switchs_.size()) && switchs_[i] < position; ++i)
    active = !active;
  return active;
}

void LinterOptions::Disable(int position) {
  bool active = active_start_;
  if (switchs_.size() & 1) active = !active;
  if (active) switchs_.push_back(position);
}

void LinterOptions::Enable(int position) {
  bool active = active_start_;
  if (switchs_.size() & 1) active = !active;
  if (!active) switchs_.push_back(position);
}
}  // namespace zetasql::linter
