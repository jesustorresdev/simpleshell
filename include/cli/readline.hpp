/*
 * readline.hpp - Simple C++ wrapper around libreadline
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

#ifndef READLINE_HPP_
#define READLINE_HPP_

//#if defined(HAVE_READLINE_H)
//#include <readline/readline.h>
//#include <readline/history.h>
//#endif

#include <cstdio>
#include <iostream>
#include <string>

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

#include <cli/dl.hpp>

namespace cli { namespace readline
{
    //
    // Class ReadlineLibrary
    //

    class ReadlineLibrary : public dl::DynamicLibrary
    {
        public:

            ReadlineLibrary();

            //
            // Readline library function wrappers
            //

            bool readLine(std::string& line,
                const std::string& prompt = std::string()) const;

            void addHistory(const std::string& line) const
                { add_history_(line.c_str()); }

            void readHistory(const std::string& fileName);
            void writeHistory(const std::string& fileName);

            //
            // Readline library variable setters
            //

            void setInStream(std::istream& in);
            void setOutStream(std::ostream& out);

        private:
            boost::function<char* (const char*)> readline_;
            boost::function<void (const char*)> add_history_;
            boost::function<int (const char*)> read_history_;
            boost::function<int (const char*)> write_history_;

            FILE** rl_instream_;
            FILE** rl_outstream_;
    };

    //
    // Class Readline
    //

    class Readline
    {
        public:
            Readline(const std::string &historyFileName = std::string(),
                std::istream& in = std::cin, std::ostream& out = std::cout);
            ~Readline();

            bool readLine(std::string& line,
                const std::string& prompt = std::string()) const;

        private:
            boost::scoped_ptr<ReadlineLibrary> readlineLibrary_;

            std::string historyFileName_;
            std::istream& in_;
            std::ostream& out_;
    };
}}

#endif /* READLINE_HPP_ */
