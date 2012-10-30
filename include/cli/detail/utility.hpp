/*
 * utility.hpp - Private components useful to other parts of the library
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

#ifndef DETAIL_UTILITY_HPP_
#define DETAIL_UTILITY_HPP_

#include <ios>
#include <string>

#include <boost/system/error_code.hpp>

namespace cli { namespace utility { namespace detail
{
    bool isLineEmpty(const std::string& line);
    bool isStreamTty(const std::ios& stream);

}}}

namespace std
{
    using namespace boost::system;
}

#endif /* DETAIL_UTILITY_HPP_ */
