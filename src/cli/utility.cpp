/*
 * utility.cpp - Components useful to other parts of the library
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

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <locale>
#include <string>
#include <unistd.h>
#include <vector>

#include <boost/shared_array.hpp>

#include <cli/detail/utility.hpp>
#include <cli/utility.hpp>

#include "config.h"
#include "fileno.hpp"

namespace cli { namespace utility
{
    //
    // Retrieve the basename component of name that was used to invoke the
    // calling program
    //

    const char* getProgramInvocationShortName()
    {
#if defined(_GNU_SOURCE)
        return ::program_invocation_short_name;
#elif defined(HAS_GETPROGNAME)
        return getprogname();
#else
        static const char* unknown_program_name = "(unknown)";
        return unknown_program_name;
#endif
    }

    //
    // Functions for std::vector<std::string> to char*[] conversion
    //

    void deleteArgV(char** argv)
    {
        char** p = argv;
        while(*p != NULL) {
            delete[] *p;
            ++p;
        }
        delete[] argv;
    }

    char** stdVectorStringToArgV(const std::vector<std::string> &strings)
    {
        int length = strings.size();
        char** argv = new char*[length + 1];
        for (int i = 0; i < length; ++i) {
            argv[i] = new char[strings[i].size() + 1];
            ::strcpy(argv[i], strings[i].c_str());
        }
        argv[length] = NULL;
        return argv;
    }

    boost::shared_array<char*>
    stdVectorStringToSmartArgV(const std::vector<std::string> &strings)
    {
        return boost::shared_array<char*>(stdVectorStringToArgV(strings),
            deleteArgV);
    }

namespace detail
{
    bool isCharNoSpace(char c)
    {
        return ! std::isspace(c, std::locale());
    }

    bool isLineEmpty(const std::string& line)
    {
        std::string::const_iterator first;
        first = std::find_if(line.begin(), line.end(), isCharNoSpace);
        return first == line.end() ? true : false;
    }

    bool isStreamTty(const std::ios& stream)
    {
        int fd = ::fileno(stream);
        if (fd >= 0) {
            return ::isatty(fd) != 0 ? true : false;
        }
        return false;
    }
}}}
