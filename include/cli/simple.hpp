/*
 * simple.hpp - Simple line-oriented command interpreter which imitates the
 *              cmd.Cmd python class behavior
 *
 *   Copyright 2010-2013 Jesús Torres <jmtorres@ull.es>
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

#ifndef COMMAND_HPP_
#define COMMAND_HPP_

#include <string>

#include <cli/base.hpp>
#include <cli/traits.hpp>

namespace cli { namespace parser { namespace simpleparser
{
    struct SimpleParser
    {
        bool operator()(std::string::const_iterator& begin,
            std::string::const_iterator end, std::string& command,
            std::string& arguments, std::string& error);
    };
}}}

namespace cli
{
    using namespace cli::parser::simpleparser;

    namespace traits
    {
        template <>
        struct ParserTraits<SimpleParser>
        {
            typedef std::string ArgumentsType;
            typedef std::string ErrorType;
        };
    }

    typedef CommandLineInterpreterBase<SimpleParser> SimpleInterpreter;
}

#endif /* COMMAND_HPP_ */
