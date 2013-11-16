/*
 * base.hpp - Base class for from which all line-oriented command interpreters
 *            acquire its core functionality
 *
 * Simple framework for writing line-oriented command interpreters
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

#ifndef BASE_HPP_
#define BASE_HPP_

#include <iostream>
#include <string>

#include <cli/callbacks.hpp>
#include <cli/readline.hpp>
#include <cli/utility.hpp>

namespace cli
{
    using namespace cli::callback;

    //
    // Class ParseError
    //
    // Type used to return parse errors to the interpreter.
    //

    struct ParseError
    {
        ParseError(bool fail = false)
            : fail_(fail), what_()
        {}

        ParseError(const std::string& what)
            : fail_(true), what_(what)
        {}

        virtual ~ParseError() throw() {}

        virtual ParseError& operator= (const ParseError& other)
            { fail_ = other.fail_; what_ = other.what_; return *this; }

        virtual operator bool() const
            { return fail_; }

        virtual const char* what() const
            { return what_.c_str(); }

        protected:
            bool fail_;
            std::string what_;
    };

    //
    // Class CommandLineInterpreterBase
    //

    template <typename Arguments>
    class CommandLineInterpreterBase
    {
        public:
            typedef CommandLineInterpreterBase<Arguments> Type;
            typedef Arguments ArgumentsType;
            typedef ParseError ParseErrorType;

            //
            // Class constructors
            //

            CommandLineInterpreterBase(bool useReadline = true);
            CommandLineInterpreterBase(std::istream& in, std::ostream& out,
                std::ostream& err = std::cerr, bool useReadline = true);

            virtual ~CommandLineInterpreterBase() {};

            //
            // Members to interpret command-line input
            //

            void loop();
            bool interpretOneLine(std::string line);

            //
            // Members to manage the command history
            //

            const std::string& lastCommand() const
                { return lastCommand_; }

            void historyFile(const std::string& fileName);

            //
            // Members to configure the user interface
            //

            void introText(const std::string& intro)
                { introText_ = intro; }
            void promptText(const std::string& prompt)
                { promptText_ = prompt; }

            //
            // Accessors of callback functions
            //

            RunCommandCallback<Type> onRunCommand;
            EmptyLineCallback onEmptyLine;
            PreRunCommandCallback onPreRunCommand;
            PostRunCommandCallback onPostRunCommand;
            PreLoopCallback onPreLoop;
            PostLoopCallback onPostLoop;
            ParseErrorCallback<Type> onParseError;

        protected:

            //
            // Hook methods invoked for command execution
            //

            virtual bool runCommand(const std::string& command,
            	Arguments const& arguments);
            virtual bool emptyLine();

            //
            // Hook methods invoked inside interpretOneLine()
            //

            virtual void preRunCommand(std::string& line) {};
            virtual bool postRunCommand(bool isFinished,
                const std::string& line);
            virtual bool parseError(ParseError const& error,
                const std::string& line);

            //
            // Hook methods invoked once inside loop()
            //

            virtual void preLoop();
            virtual void postLoop();

        private:
            std::istream& in_;
            std::ostream& out_;
            std::ostream& err_;
            readline::Readline readLine_;

            std::string introText_;
            std::string promptText_;
            std::string lastCommand_;

            //
            // Parser handling
            //

            virtual ParseError& parse(std::string::const_iterator& begin,
                std::string::const_iterator end, std::string& command,
                Arguments& arguments) = 0;
    };

    template <typename Arguments>
    CommandLineInterpreterBase<Arguments>::CommandLineInterpreterBase(
        bool useReadline)
        : in_(std::cin),
          out_(std::cout),
          err_(std::cerr),
          readLine_(useReadline)
    {}

    template <typename Arguments>
    CommandLineInterpreterBase<Arguments>::CommandLineInterpreterBase(
        std::istream& in, std::ostream& out, std::ostream& err,
        bool useReadline)
        : in_(in),
          out_(out),
          err_(err),
          readLine_(useReadline)
    {}

    template <typename Arguments>
    void CommandLineInterpreterBase<Arguments>::loop()
    {
        using utility::detail::isStreamTty;

        preLoop();

        std::string promptText;
        if (isStreamTty(in_) && isStreamTty(out_)) {
            out_ << introText_ << std::endl;
            promptText = promptText_;
        }

        std::string line;
        while (true) {
            bool isOk = readLine_.readLine(line, promptText);
            if (! isOk)
                break;
            bool isFinished = interpretOneLine(line);
            if (isFinished)
                break;
        }

        postLoop();
    }

    template <typename Arguments>
    bool CommandLineInterpreterBase<Arguments>::interpretOneLine(
        std::string line)
    {
        preRunCommand(line);

        if (utility::detail::isLineEmpty(line)) {
            return emptyLine();
        }
        else {
            lastCommand_ = line;
        }

        std::string::const_iterator begin = line.begin();
        std::string::const_iterator end = line.end();
        while (begin != end) {
            std::string command;
            Arguments arguments;
            ParseError error = parse(begin, end, command, arguments);

            bool isFinished;
            if (error) {
                isFinished = parseError(error, line);
                return isFinished;
            }
            else {
                isFinished = runCommand(command, arguments);
                isFinished = postRunCommand(isFinished, line);
                if (isFinished)
                    return true;
            }
        }
        return false;
    }

    template <typename Arguments>
    bool CommandLineInterpreterBase<Arguments>::runCommand(
        const std::string& command, Arguments const& arguments)
    {
        return onRunCommand ? onRunCommand.call(command, arguments) : false;
    }

    template <typename Arguments>
    bool CommandLineInterpreterBase<Arguments>::emptyLine()
    {
        return onEmptyLine ? onEmptyLine.call() : false;
    }

    template <typename Arguments>
    bool CommandLineInterpreterBase<Arguments>::postRunCommand(bool isFinished,
        const std::string& line)
    {
        return onPostRunCommand ?
            onPostRunCommand.call(isFinished, line) : isFinished;
    }

    template <typename Arguments>
    void CommandLineInterpreterBase<Arguments>::preLoop()
    {
        if (onPreLoop) {
            onPreLoop.call();
        }
    }

    template <typename Arguments>
    void CommandLineInterpreterBase<Arguments>::postLoop()
    {
        if (onPostLoop) {
            onPostLoop.call();
        }
    }

    template <typename Arguments>
    bool CommandLineInterpreterBase<Arguments>::parseError(
        ParseError const& error, const std::string& line)
    {
        if (! onParseError) {
            err_ << cli::utility::programShortName()
                 << ": "
                 << error.what()
                 << std::endl;
            return false;
        }
        return onParseError.call(error, line);
    }

    template <typename Arguments>
    void CommandLineInterpreterBase<Arguments>::historyFile(
        const std::string& fileName)
    {
        readLine_.clearHistory();
        readLine_.historyFile(fileName);
    }
}

#endif /* BASE_HPP_ */
