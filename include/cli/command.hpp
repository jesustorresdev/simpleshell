/*
 * ccommand.hpp - Simple line-oriented command interpreter which imitates the
 *                cmd.Cmd python class behavior
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

#ifndef COMMAND_HPP_
#define COMMAND_HPP_

#include <string>

#include <cli/base.hpp>

namespace cli
{
    //
    // Class CommandInterpreter
    //

    class CommandInterpreter
        : public CommandLineInterpreterBase<std::string>
    {
        //
        // Parser handling
        //

        ParseError& parse(std::string::const_iterator& begin,
            std::string::const_iterator end, std::string& command,
            std::string& arguments);

        private:
            ParseError parseError_;
    };
}

#endif /* COMMAND_HPP_ */
