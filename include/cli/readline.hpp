/*
 * readline.hpp - Simple C++ wrapper around libreadline
 *
 *   Copyright 2010-2016 Jesús Torres <jmtorres@ull.es>
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
#include <functional>
#include <iostream>
#include <memory>
#include <string>

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

            void addHistory(const std::string& line);
            void readHistory(const std::string& fileName);
            void writeHistory(const std::string& fileName);
            void clearHistory();

            //
            // Readline library I/O streams setters
            //

            void inStream(std::istream& in);
            void outStream(std::ostream& out);

        private:
            std::function<char* (const char*)> readline_;
            std::function<void (const char*)> add_history_;
            std::function<int (const char*)> read_history_;
            std::function<int (const char*)> write_history_;
            std::function<void ()> clear_history_;

            FILE** rl_instream_;
            FILE** rl_outstream_;
    };

    //
    // Class Readline
    //

    class Readline
    {
        public:

            Readline(bool useLibrary = true);

            ~Readline();

            bool readLine(std::string& line,
                const std::string& prompt = std::string()) const;

            bool isUsingLibrary()
                { return static_cast<bool>(readlineLibrary_); }

            //
            // I/O streams setters
            //

            void inStream(std::istream& in);
            void outStream(std::ostream& out);

            //
            // History management
            //

            void historyFile(const std::string &fileName,
                bool loadInHistory = true);
            void clearHistory();

        private:
            std::unique_ptr<ReadlineLibrary> readlineLibrary_;
            std::istream* in_;
            std::ostream* out_;

            std::string historyFileName_;
    };
}}

#endif /* READLINE_HPP_ */
