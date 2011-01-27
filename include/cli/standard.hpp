/*
 * standard.hpp - Some auxiliary functions that must go in std namespace
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

#ifndef STANDARD_HPP_
#define STANDARD_HPP_

#include <iostream>
#include <vector>

namespace std
{
    template <typename CharT, typename Traits, typename T, typename Alloc>
    basic_ostream<CharT, Traits>& operator<<(basic_ostream<CharT, Traits>& out,
        const vector<T, Alloc>& vector)
    {
        out << "[";

        typename std::vector<T, Alloc>::const_iterator iter = vector.begin();
        if (iter != vector.end()) {
            out << *iter;
            for (iter += 1; iter < vector.end(); ++iter) {
                out << ", " << *iter;
            }
        }

        return out << "]";
    }
}

#endif /* STANDARD_HPP_ */