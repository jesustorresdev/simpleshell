/*
 * auxiliary.cpp - Auxiliary public functions
 *
 *   Copyright 2010 Jesús Torres <jmtorres@ull.es>
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

#include <string>
#include <vector>

#include <boost/shared_array.hpp>

#include <cli/auxiliary.hpp>

#include "fileno.hpp"

namespace cli { namespace auxiliary
{

    boost::shared_array<const char*>
    stdVectorStringToArgV(const std::vector<std::string> &strings)
    {
        int length = strings.size();
        boost::shared_array<const char*> argv =
            boost::shared_array<const char*>(new const char*[length + 1]);
        for (int i = 0; i < length; i++) {
            argv[i] = strings[i].c_str();
        }
        argv[length] = NULL;
        return argv;
    }

}}
