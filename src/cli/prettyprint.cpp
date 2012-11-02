/*
 * prettyprint.cpp - Pretty-print output stream manipulator
 *
 *   Copyright 2010-2012 Jesús Torres <jmtorres@ull.es>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ostream>
#include <string>

#include <cli/detail/prettyprint.hpp>
#include <cli/prettyprint.hpp>

namespace cli { namespace prettyprint { namespace detail
{
    const int INDENT_DEFAULT_WIDTH = 4;

    //
    // Indexes in stream internal extensible array
    //

    const int PRETTYPRINT_ENABLED_FLAG_INDEX = std::ios_base::xalloc();
    const int INDENT_SPACE_INDEX = std::ios_base::xalloc();
    const int INDENT_WIDTH_INDEX = std::ios_base::xalloc();
}}}
