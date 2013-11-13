/*
 * readline.cpp - Simple C++ wrapper around libreadline
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

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/system/system_error.hpp>

#include <cli/detail/utility.hpp>
#include <cli/readline.hpp>

#include "fileno.hpp"

namespace cli { namespace readline
{
    using namespace boost;
    using namespace cli;

    //
    // Readline library file names to search for
    //

    const char* LIBREADLINE_FILENAMES[] = {
        "libreadline.so",
        "libreadline.so.5",
        "libreadline.so.6",
        NULL
    };

    //
    // Class ReadlineLibrary
    //

    ReadlineLibrary::ReadlineLibrary() : dl::DynamicLibrary(),
        rl_instream_(NULL), rl_outstream_(NULL)
    {
        // Try to load some readline libray
        for (const char** library = LIBREADLINE_FILENAMES; *library != NULL;
            ++library)
        {
            load(*library);
            if (isLoad()) {
                break;
            }
        }

        // The readline symbol existence is the only requirement to use the
        // readline library loaded. Other symbols provided by the library
        // will be resolved on demand.
        if (isLoad()) {
            resolveFunction(readline_, "readline");
            if (lastError() != std::errc::success) {
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

//
// Macros READLINELIBRARY_FUNCTION & READLINELIBRARY_VARIABLE
//
// Used internally by class ReadlineLibrary to resolve on demand the symbols
// provided by the library.
//

#define READLINELIBRARY_FUNCTION(FUNCTION, SYMBOL)          \
    if (FUNCTION.empty()) {                                 \
        resolveFunction(FUNCTION, SYMBOL);                  \
        if (lastError() != std::errc::success) {            \
            return;                                         \
        }                                                   \
    }

#define READLINELIBRARY_VARIABLE(VARIABLE_PTR, SYMBOL)      \
    if (*VARIABLE_PTR == NULL) {                            \
        resolve(VARIABLE_PTR, SYMBOL);                      \
        if (lastError() != std::errc::success) {            \
            return;                                         \
        }                                                   \
    }

    //
    // Methods for history management
    //

    void ReadlineLibrary::addHistory(const std::string& line)
    {
        READLINELIBRARY_FUNCTION(add_history_, "add_history");
        add_history_(line.c_str());
    }

    void ReadlineLibrary::readHistory(const std::string& fileName)
    {
        READLINELIBRARY_FUNCTION(read_history_, "read_history");
        const char* c_fileName = fileName.empty() ? NULL : fileName.c_str();
        int errorNo = read_history_(c_fileName);
        errorCode_ = std::error_code(errorNo, std::system_category());
    }

    void ReadlineLibrary::writeHistory(const std::string& fileName)
    {
        READLINELIBRARY_FUNCTION(write_history_, "write_history");
        const char* c_fileName = fileName.empty() ? NULL : fileName.c_str();
        int errorNo = write_history_(c_fileName);
        errorCode_ = std::error_code(errorNo, std::system_category());
    }

    void ReadlineLibrary::clearHistory()
    {
        READLINELIBRARY_FUNCTION(clear_history_, "clear_history");
        clear_history_();
    }

    //
    // I/0 streams setters
    //

    void ReadlineLibrary::inStream(std::istream& in)
    {
        READLINELIBRARY_VARIABLE(rl_instream_, "rl_instream");
        int fd_in = ::fileno(in);
        if (fd_in == -1) {
            errorCode_ = std::error_code(errno, std::system_category());
            return;
        }

        FILE* file_in = ::fdopen(fd_in, "r");
        if (file_in == NULL) {
            errorCode_ = std::error_code(errno, std::system_category());
            return;
        }

        *rl_instream_ = file_in;
        errorCode_.clear();
    }

    void ReadlineLibrary::outStream(std::ostream& out)
    {
        READLINELIBRARY_VARIABLE(rl_outstream_, "rl_outstream");
        int fd_out = ::fileno(out);
        if (fd_out == -1) {
            errorCode_ = std::error_code(errno, std::system_category());
            return;
        }

        FILE* file_out = ::fdopen(fd_out, "w");
        if (file_out == NULL) {
            errorCode_ = std::error_code(errno, std::system_category());
            return;
        }

        *rl_outstream_ = file_out;
        errorCode_.clear();
    }

    //
    // Class Readline
    //

    Readline::Readline(bool useLibrary)
        : readlineLibrary_(useLibrary ? new ReadlineLibrary() : NULL),
          in_(&std::cin), out_(&std::cout)
    {
        if (readlineLibrary_ &&
            (readlineLibrary_->lastError() != std::errc::success)) {
            readlineLibrary_.reset();
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
            if (isOk && (! utility::detail::isLineEmpty(line))) {
                readlineLibrary_->addHistory(line);
            }
            return isOk;
        }
        else {
            *out_ << prompt;
            std::getline(*in_, line);
            return in_->good();
        }
    }

    //
    // Standard I/O streams setters
    //

    void Readline::inStream(std::istream& in)
    {
        in_ = &in;
        if (readlineLibrary_) {
            readlineLibrary_->inStream(in);
            std::error_code errorCode = readlineLibrary_->lastError();
            if (errorCode != std::errc::success) {
                throw std::system_error(errorCode,
                    "unexpected error changing readline input stream");
            }
        }
    }

    void Readline::outStream(std::ostream& out)
    {
        out_ = &out;
        if (readlineLibrary_) {
            readlineLibrary_->outStream(out);
            std::error_code errorCode = readlineLibrary_->lastError();
            if (errorCode != std::errc::success) {
                throw std::system_error(errorCode,
                    "unexpected error changing readline output stream");
            }
        }
    }

    //
    // Members for history management
    //

    void Readline::historyFile(const std::string &fileName,
        bool loadInHistory)
    {
        if (readlineLibrary_) {
            historyFileName_ = fileName;
            if (loadInHistory) {
                readlineLibrary_->readHistory(fileName);
            }
        }
    }

    void Readline::clearHistory()
    {
        if (readlineLibrary_) {
            readlineLibrary_->clearHistory();
        }
    }
}}
