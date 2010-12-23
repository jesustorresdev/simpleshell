/*
 * readline.cpp - Simple C++ wrapper around libreadline
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

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/system/error_code.hpp>

#include <cli/internals.hpp>
#include <cli/readline.hpp>

#include "fileno.hpp"

namespace cli { namespace readline
{
    using namespace boost;
    using namespace cli;

    const char LIBREADLINE_FILENAME[] = "libreadline.so";

    //
    // Class ReadlineLibrary
    //

    ReadlineLibrary::ReadlineLibrary()
        : dl::DynamicLibrary(LIBREADLINE_FILENAME)
    {
        if (isOpen()) {
            readline_ = getFunction<char* (const char*)>("readline");
            if (getLastError() != system::errc::success) {
                return;
            }

            add_history_ = getFunction<void (const char*)>("add_history");
            if (getLastError() != system::errc::success) {
                return;
            }

            read_history_ = getFunction<int (const char*)>("read_history");
            if (getLastError() != system::errc::success) {
                return;
            }

            write_history_ = getFunction<int (const char*)>("write_history");
            if (getLastError() != system::errc::success) {
                return;
            }

            rl_instream_ = getVariable<FILE*>("rl_instream");
            if (getLastError() != system::errc::success) {
                return;
            }

            rl_outstream_ = getVariable<FILE*>("rl_outstream");
            if (getLastError() != system::errc::success) {
                return;
            }
        }
    }

    bool ReadlineLibrary::readLine(std::string& line,
        const std::string& prompt) const
    {
        char* c_line = readline_(prompt.c_str());
        if (c_line != NULL) {
            line = c_line;
            free(c_line);
            return true;
        }
        return false;
    }

    void ReadlineLibrary::readHistory(const std::string& fileName)
    {
        const char* c_fileName = fileName.empty() ? NULL : fileName.c_str();
        int errorNo = read_history_(c_fileName);
        errorCode_ = system::error_code(errorNo, system::system_category());
    }

    void ReadlineLibrary::writeHistory(const std::string& fileName)
    {
        const char* c_fileName = fileName.empty() ? NULL : fileName.c_str();
        int errorNo = write_history_(c_fileName);
        errorCode_ = system::error_code(errorNo, system::system_category());
    }

    void ReadlineLibrary::setInStream(std::istream& in)
    {
        int fd_in = ::fileno(in);
        if (fd_in == -1) {
            errorCode_ = system::error_code(errno, system::system_category());
            return;
        }

        FILE* file_in = ::fdopen(fd_in, "r");
        if (file_in == NULL) {
            errorCode_ = system::error_code(errno, system::system_category());
            return;
        }

        *rl_instream_ = file_in;
        errorCode_.clear();
    }

    void ReadlineLibrary::setOutStream(std::ostream& out)
    {
        int fd_out = ::fileno(out);
        if (fd_out == -1) {
            errorCode_ = system::error_code(errno, system::system_category());
            return;
        }

        FILE* file_out = ::fdopen(fd_out, "w");
        if (file_out == NULL) {
            errorCode_ = system::error_code(errno, system::system_category());
            return;
        }

        *rl_outstream_ = file_out;
        errorCode_.clear();
    }

    //
    // Class Readline
    //

    Readline::Readline(const std::string &historyFileName, std::istream& in,
        std::ostream& out)
        : readlineLibrary_(new ReadlineLibrary()),
          historyFileName_(historyFileName), in_(in), out_(out)
    {
        if (readlineLibrary_->getLastError() != system::errc::success) {
            readlineLibrary_.reset();
        }

        // Set the input and output streams for readline
        if (readlineLibrary_ && internals::isStreamTty(in)) {
            readlineLibrary_->setInStream(in);
            if (readlineLibrary_->getLastError() != system::errc::success) {
                readlineLibrary_.reset();
            }
        }
        if (readlineLibrary_ && internals::isStreamTty(out)) {
            readlineLibrary_->setOutStream(out);
            if (readlineLibrary_->getLastError() != system::errc::success) {
                readlineLibrary_.reset();
            }
        }

        // Load the readline history file
        if (readlineLibrary_ && (! historyFileName.empty())) {
            readlineLibrary_->readHistory(historyFileName);
        }
    }

    Readline::~Readline()
    {
        if (readlineLibrary_ && (! historyFileName_.empty())) {
            readlineLibrary_->writeHistory(historyFileName_);
        }
    }

    bool Readline::readLine(std::string& line, const std::string& prompt) const
    {
        if (readlineLibrary_) {
            bool isOk = readlineLibrary_->readLine(line, prompt);
            if (isOk && (! internals::isLineEmpty(line))) {
                readlineLibrary_->addHistory(line);
            }
            return isOk;
        }
        else {
            if (internals::isStreamTty(out_)) {
                out_ << prompt;
            }
            std::getline(in_, line);
            return in_.good();
        }
    }
}}
