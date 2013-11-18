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

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <cli/callbacks.hpp>
#include <cli/readline.hpp>
#include <cli/traits.hpp>
#include <cli/utility.hpp>

namespace cli
{
    //
    // Class CommandLineInterpreterBase
    //

    template <typename Parser>
    class CommandLineInterpreterBase
    {
        public:
            typedef CommandLineInterpreterBase<Parser> Type;

            typedef Parser ParserType;
            typedef typename cli::traits::ParserTraits<Parser>::ArgumentsType
                CommandArgumentsType;
            typedef typename cli::traits::ParserTraits<Parser>::ErrorType
                ParseErrorType;

            typedef bool (ParserSignature)(
                std::string::const_iterator&, std::string::const_iterator,
                std::string&, CommandArgumentsType&, ParseErrorType&);

            //
            // Class constructors
            //

            CommandLineInterpreterBase(bool useReadline = true);
            CommandLineInterpreterBase(std::istream& in, std::ostream& out,
                std::ostream& err = std::cerr, bool useReadline = true);

            CommandLineInterpreterBase(
                const boost::function<ParserSignature>& parser,
                bool useReadline = true);
            CommandLineInterpreterBase(
                const boost::function<ParserSignature>& parser,
                std::istream& in, std::ostream& out,
                std::ostream& err = std::cerr, bool useReadline = true);

            CommandLineInterpreterBase(
                boost::shared_ptr<Parser> parser,
                bool useReadline = true);
            CommandLineInterpreterBase(
                boost::shared_ptr<Parser> parser,
                std::istream& in, std::ostream& out,
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

            cli::callback::RunCommandCallback<ParserType> onRunCommand;
            cli::callback::ParseErrorCallback<ParserType> onParseError;
            cli::callback::EmptyLineCallback onEmptyLine;
            cli::callback::PreRunCommandCallback onPreRunCommand;
            cli::callback::PostRunCommandCallback onPostRunCommand;
            cli::callback::PreLoopCallback onPreLoop;
            cli::callback::PostLoopCallback onPostLoop;

        private:
            std::istream& in_;
            std::ostream& out_;
            std::ostream& err_;
            readline::Readline readLine_;

            std::string introText_;
            std::string promptText_;
            std::string lastCommand_;

            boost::shared_ptr<Parser> parserObject_;
            boost::function<ParserSignature> parser_;

            //
            // Hook methods invoked for command execution
            //

            virtual bool runCommand(const std::string& command,
                CommandArgumentsType const& arguments);
            virtual bool emptyLine();

            //
            // Hook methods invoked inside interpretOneLine()
            //

            virtual void preRunCommand(std::string& line) {};
            virtual bool postRunCommand(bool isFinished,
                const std::string& line);
            virtual bool parseError(ParseErrorType const& error,
                const std::string& line);

            //
            // Hook methods invoked once inside loop()
            //

            virtual void preLoop();
            virtual void postLoop();
    };

    template <typename Parser>
    CommandLineInterpreterBase<Parser>::CommandLineInterpreterBase(
        bool useReadline)
        : in_(std::cin),
          out_(std::cout),
          err_(std::cerr),
          readLine_(useReadline),
          parserObject_(new Parser),
          parser_(*parserObject_)
    {}

    template <typename Parser>
    CommandLineInterpreterBase<Parser>::CommandLineInterpreterBase(
        std::istream& in, std::ostream& out, std::ostream& err,
        bool useReadline)
        : in_(in),
          out_(out),
          err_(err),
          readLine_(useReadline),
          parserObject_(new Parser),
          parser_(*parserObject_)
    {}

    template <typename Parser>
    CommandLineInterpreterBase<Parser>::CommandLineInterpreterBase(
        const boost::function<ParserSignature>& parser, bool useReadline)
        : in_(std::cin),
          out_(std::cout),
          err_(std::cerr),
          readLine_(useReadline),
          parser_(parser)
    {}

    template <typename Parser>
    CommandLineInterpreterBase<Parser>::CommandLineInterpreterBase(
        const boost::function<ParserSignature>& parser, std::istream& in,
        std::ostream& out, std::ostream& err, bool useReadline)
        : in_(in),
          out_(out),
          err_(err),
          readLine_(useReadline),
          parser_(parser)
    {}

    template <typename Parser>
    CommandLineInterpreterBase<Parser>::CommandLineInterpreterBase(
        boost::shared_ptr<Parser> parser, bool useReadline)
        : in_(std::cin),
          out_(std::cout),
          err_(std::cerr),
          readLine_(useReadline),
          parserObject_(parser),
          parser_(*parser)
    {}

    template <typename Parser>
    CommandLineInterpreterBase<Parser>::CommandLineInterpreterBase(
        boost::shared_ptr<Parser> parser, std::istream& in,
        std::ostream& out, std::ostream& err, bool useReadline)
        : in_(in),
          out_(out),
          err_(err),
          readLine_(useReadline),
          parserObject_(parser),
          parser_(*parser)
    {}

    template <typename Parser>
    void CommandLineInterpreterBase<Parser>::loop()
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

    template <typename Parser>
    bool CommandLineInterpreterBase<Parser>::interpretOneLine(
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
            CommandArgumentsType arguments;
            ParseErrorType error;
            bool success = parser_(begin, end, command, arguments, error);

            bool isFinished;
            if (! success) {
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

    template <typename Parser>
    void CommandLineInterpreterBase<Parser>::historyFile(
        const std::string& fileName)
    {
        readLine_.clearHistory();
        readLine_.historyFile(fileName);
    }

    template <typename Parser>
    bool CommandLineInterpreterBase<Parser>::runCommand(
        const std::string& command, CommandArgumentsType const& arguments)
    {
        return onRunCommand ? onRunCommand.call(command, arguments) : false;
    }

    template <typename Parser>
    bool CommandLineInterpreterBase<Parser>::emptyLine()
    {
        return onEmptyLine ? onEmptyLine.call() : false;
    }

    template <typename Parser>
    bool CommandLineInterpreterBase<Parser>::postRunCommand(bool isFinished,
        const std::string& line)
    {
        return onPostRunCommand ?
            onPostRunCommand.call(isFinished, line) : isFinished;
    }

    template <typename Parser>
    void CommandLineInterpreterBase<Parser>::preLoop()
    {
        if (onPreLoop) {
            onPreLoop.call();
        }
    }

    template <typename Parser>
    void CommandLineInterpreterBase<Parser>::postLoop()
    {
        if (onPostLoop) {
            onPostLoop.call();
        }
    }

    template <typename Parser>
    bool CommandLineInterpreterBase<Parser>::parseError(
        ParseErrorType const& error, const std::string& line)
    {
        if (! onParseError) {
            err_ << cli::utility::programShortName()
                 << ": "
                 << cli::utility::parseErrorToStdString(error)
                 << std::endl;
            return false;
        }
        return onParseError.call(error, line);
    }
}

#endif /* BASE_HPP_ */
