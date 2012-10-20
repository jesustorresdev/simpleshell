/*
 * auxiliary.hpp - Auxiliary public functions
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

#ifndef AUXILIARY_HPP_
#define AUXILIARY_HPP_

#include <iostream>
#include <string>
#include <vector>

#include <boost/shared_array.hpp>

namespace cli { namespace auxiliary
{
    char** stdVectorStringToArgV(const std::vector<std::string> &strings);

    boost::shared_array<char*> stdVectorStringToSmartArgV(
        const std::vector<std::string> &strings);
}}

namespace std
{
    template <typename CharT, typename Traits, typename T, typename Alloc>
    basic_ostream<CharT, Traits>& operator<<(basic_ostream<CharT, Traits>& out,
        const vector<T, Alloc>& vector)
    {
        out << "[";

        typename std::vector<T, Alloc>::const_iterator i = vector.begin();
        if (i != vector.end()) {
            out << *i;
            for (i += 1; i < vector.end(); ++i) {
                out << ", " << *i;
            }
        }

        return out << "]";
    }
}

#endif /* AUXILIARY_HPP_ */
