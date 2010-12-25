/*
 * cli.hpp - Simple framework for writing line-oriented command interpreters
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

#ifndef CLI_HPP_
#define CLI_HPP_

#include <iostream>
#include <string>

#include <libintl.h>    // TODO: To use Boost.Locale when available
#define translate(str) ::gettext(str)

#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/spirit/include/qi.hpp>

#include <cli/callbacks.hpp>
#include <cli/internals.hpp>
#include <cli/readline.hpp>

namespace cli
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    //
    // Class CommandLineInterpreter
    //

    template <template<typename> class Parser, typename Command>
    class CommandLineInterpreter
    {
        public:
            typedef CommandLineInterpreter<Parser, Command> ClassType;

            typedef Parser<std::string::iterator> ParserType;
            typedef Command CommandType;

            // Callback functions types
            typedef typename callbacks::DoCommandCallback<ClassType>::Type
                DoCommandCallback;
            typedef typename callbacks::EmptyLineCallback<ClassType>::Type
                EmptyLineCallback;
            typedef typename callbacks::PreDoCommandCallback<ClassType>::Type
                PreDoCommandCallback;
            typedef typename callbacks::PostDoCommandCallback<ClassType>::Type
                PostDoCommandCallback;
            typedef typename callbacks::PreLoopCallback<ClassType>::Type
                PreLoopCallback;
            typedef typename callbacks::PostLoopCallback<ClassType>::Type
                PostLoopCallback;

            // Constructor
            CommandLineInterpreter(
                const std::string &historyFileName = std::string(),
                std::istream& in = std::cin,
                std::ostream& out = std::cout);

            // Methods to interpret command-line input
            void loop();
            bool interpretOneLine(std::string line);

            // Getters
            std::istream& getInStream() const
                { return in_; }
            std::ostream& getOutStream() const
                { return out_; }

            std::string getLastCommand() const
                { return lastCommand_; }

            // Setters
            void setIntroText(const std::string& intro)
                { introText_ = intro; }

            void setPromptText(const std::string& prompt)
                { promptText_ = prompt; }

            template <template <typename> class Callback, typename Functor>
            void setCallback(Functor function);

        protected:
            virtual bool defaultDoCommand(const Command& command);
            virtual bool defaultEmptyLine();

            // Hook methods invoked inside interpretOneLine()
            virtual void defaultPreDoCommand(std::string& line) {};
            virtual bool defaultPostDoCommand(bool isFinished,
                const std::string& line);

            // Hook methods invoked once inside loop()
            virtual void defaultPreLoop() {};
            virtual void defaultPostLoop() {};

        private:
            std::istream& in_;
            std::ostream& out_;
            readline::Readline readLine_;

            boost::scoped_ptr<ParserType> lineParser_;

            std::string introText_;
            std::string promptText_;
            std::string lastCommand_;

            // Callback functions objects
            boost::function<DoCommandCallback> doCommandCallback_;
            boost::function<EmptyLineCallback> emptyLineCallback_;
            boost::function<PreDoCommandCallback> preDoCommandCallback_;
            boost::function<PostDoCommandCallback> postDoCommandCallback_;
            boost::function<PreLoopCallback> preLoopCallback_;
            boost::function<PostLoopCallback> postLoopCallback_;

            template <template <typename> class Callback>
            template <typename Interpreter, typename Functor>
            friend void cli::callbacks::SetCallbackImpl<Callback>::
                setCallback(Interpreter&, Functor);
    };

    template <template <typename> class Parser, typename Command>
    CommandLineInterpreter<Parser, Command>::CommandLineInterpreter(
        const std::string &historyFileName,
        std::istream& in,
        std::ostream& out)
        : in_(in),
          out_(out),
          readLine_(historyFileName, in, out),
          lineParser_(new ParserType),
          doCommandCallback_(&ClassType::defaultDoCommand),
          emptyLineCallback_(&ClassType::defaultEmptyLine),
          preDoCommandCallback_(&ClassType::defaultPreDoCommand),
          postDoCommandCallback_(&ClassType::defaultPostDoCommand),
          preLoopCallback_(&ClassType::defaultPreLoop),
          postLoopCallback_(&ClassType::defaultPostLoop)
    {}

    template <template <typename> class Parser, typename Command>
    void CommandLineInterpreter<Parser, Command>::loop()
    {
        preLoopCallback_(this);

        if (internals::isStreamTty(out_)) {
            if (! introText_.empty()) {
                out_ << introText_ << std::endl;
            }
        }

        std::string line;
        while (true) {
            bool isOk = readLine_.readLine(line, promptText_);
            if (! isOk)
                break;
            bool isFinished = interpretOneLine(line);
            if (isFinished)
                break;
        }

        postLoopCallback_(this);
    }

    template <template <typename> class Parser, typename Command>
    bool CommandLineInterpreter<Parser, Command>::interpretOneLine(
        std::string line)
    {
        preDoCommandCallback_(this, line);

        if (internals::isLineEmpty(line)) {
            return emptyLineCallback_(this);
        }
        else {
            lastCommand_ = line;
        }

        Command command;
        std::string::iterator begin = line.begin();
        std::string::iterator end = line.end();
        bool success = qi::phrase_parse(begin, end, *lineParser_,
            ascii::space, command);
        if (success && begin == end) {
            bool isFinished = doCommandCallback_(this, command);
            return postDoCommandCallback_(this, isFinished, line);
        }
        else {
            std::string no_parsed(begin, end);
            out_ << no_parsed
                 << ": " << translate("syntax error") << std::endl;
            return false;
        }
    }

    template <template <typename> class Parser, typename Command>
    bool CommandLineInterpreter<Parser, Command>::defaultDoCommand(
            const Command& command)
    {
        return false;
    }

    template <template <typename> class Parser, typename Command>
    bool CommandLineInterpreter<Parser, Command>::defaultEmptyLine()
    {
        if (! lastCommand_.empty()) {
            return interpretOneLine(lastCommand_);
        }
        return false;
    }

    template <template <typename> class Parser, typename Command>
    bool CommandLineInterpreter<Parser, Command>::defaultPostDoCommand(
        bool isFinished, const std::string& line)
    {
        return isFinished;
    }

    template <template <typename> class Parser, typename Command>
    template <template <typename> class Callback, typename Functor>
    void CommandLineInterpreter<Parser, Command>::setCallback(Functor function)
    {
        callbacks::SetCallbackImpl<Callback>::setCallback(*this, function);
    }
}

#endif /* CLI_HPP_ */
