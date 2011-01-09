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
            typedef CommandLineInterpreter<Parser, Command> Type;

            typedef Parser<std::string::iterator> ParserType;
            typedef Command CommandType;

            //
            // Callback functions types
            //

            typedef typename callbacks::DoCommandCallback<Type>::Type
                DoCommandCallback;
            typedef typename callbacks::EmptyLineCallback<Type>::Type
                EmptyLineCallback;
            typedef typename callbacks::PreDoCommandCallback<Type>::Type
                PreDoCommandCallback;
            typedef typename callbacks::PostDoCommandCallback<Type>::Type
                PostDoCommandCallback;
            typedef typename callbacks::PreLoopCallback<Type>::Type
                PreLoopCallback;
            typedef typename callbacks::PostLoopCallback<Type>::Type
                PostLoopCallback;

            //
            // Class constructors
            //

            CommandLineInterpreter(bool useReadline = true);
            CommandLineInterpreter(std::istream& in, std::ostream& out,
                bool useReadline = true);

            //
            // Methods to interpret command-line input
            //

            void loop();
            bool interpretOneLine(std::string line);

            //
            // Class attribute getters
            //

            std::istream& getInStream() const
                { return in_; }
            std::ostream& getOutStream() const
                { return out_; }

            std::string getLastCommand() const
                { return lastCommand_; }

            //
            // Class attribute setters
            //

            void setIntroText(const std::string& intro)
                { introText_ = intro; }

            void setPromptText(const std::string& prompt)
                { promptText_ = prompt; }

            void setHistoryFile(const std::string& fileName);

            //
            // Callback functions setter
            //

            template <template <typename> class Callback, typename Functor>
            void setCallback(Functor function);

        protected:

            //
            // Hook methods invoked for command execution
            //

            virtual bool doCommand(const Command& command);
            virtual bool emptyLine();

            //
            // Hook methods invoked inside interpretOneLine()
            //

            virtual void preDoCommand(std::string& line) {};
            virtual bool postDoCommand(bool isFinished,
                const std::string& line);

            //
            // Hook methods invoked once inside loop()
            //

            virtual void preLoop();
            virtual void postLoop();

        private:
            std::istream& in_;
            std::ostream& out_;
            readline::Readline readLine_;

            boost::scoped_ptr<ParserType> lineParser_;

            std::string introText_;
            std::string promptText_;
            std::string lastCommand_;

            //
            // Callback functions objects
            //

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
        bool useReadline)
        : in_(std::cin),
          out_(std::cout),
          readLine_(std::cin, std::cout, useReadline),
          lineParser_(new ParserType)
    {}

    template <template <typename> class Parser, typename Command>
    CommandLineInterpreter<Parser, Command>::CommandLineInterpreter(
        std::istream& in, std::ostream& out, bool useReadline)
        : in_(in),
          out_(out),
          readLine_(in, out, useReadline),
          lineParser_(new ParserType)
    {}

    template <template <typename> class Parser, typename Command>
    void CommandLineInterpreter<Parser, Command>::loop()
    {
        preLoop();

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

        postLoop();
    }

    template <template <typename> class Parser, typename Command>
    bool CommandLineInterpreter<Parser, Command>::interpretOneLine(
        std::string line)
    {
        preDoCommand(line);

        if (internals::isLineEmpty(line)) {
            return emptyLine();
        }
        else {
            lastCommand_ = line;
        }

        Command command;
        std::string::iterator begin = line.begin();
        std::string::iterator end = line.end();
        typename ParserType::skipper_type skipperParser;
        bool success = qi::phrase_parse(begin, end, *lineParser_,
            skipperParser, command);
        if (success) {
            bool isFinished = doCommand(command);
            return postDoCommand(isFinished, line);
        }

        return false;
    }

    template <template <typename> class Parser, typename Command>
    bool CommandLineInterpreter<Parser, Command>::doCommand(
        const Command& command)
    {
        return doCommandCallback_.empty() ?
            false : doCommandCallback_(command);
    }

    template <template <typename> class Parser, typename Command>
    bool CommandLineInterpreter<Parser, Command>::emptyLine()
    {
        if (! emptyLineCallback_.empty()) {
            return emptyLineCallback_();
        }
        else if (! lastCommand_.empty()) {
            return interpretOneLine(lastCommand_);
        }
        return false;
    }

    template <template <typename> class Parser, typename Command>
    bool CommandLineInterpreter<Parser, Command>::postDoCommand(
        bool isFinished, const std::string& line)
    {
        return postDoCommandCallback_.empty() ?
            isFinished : postDoCommandCallback_(isFinished, line);
    }

    template <template <typename> class Parser, typename Command>
    void CommandLineInterpreter<Parser, Command>::preLoop()
    {
        if (! preLoopCallback_.empty()) {
            preLoopCallback_();
        }
    }

    template <template <typename> class Parser, typename Command>
    void CommandLineInterpreter<Parser, Command>::postLoop()
    {
        if (! postLoopCallback_.empty()) {
            postLoopCallback_();
        }
    }

    template <template <typename> class Parser, typename Command>
    template <template <typename> class Callback, typename Functor>
    void CommandLineInterpreter<Parser, Command>::setCallback(Functor function)
    {
        callbacks::SetCallbackImpl<Callback>::setCallback(*this, function);
    }

    template <template <typename> class Parser, typename Command>
    void CommandLineInterpreter<Parser, Command>::setHistoryFile(
        const std::string& fileName)
    {
        readLine_.clearHistory();
        readLine_.setHistoryFile(fileName);
    }
}

#endif /* CLI_HPP_ */
