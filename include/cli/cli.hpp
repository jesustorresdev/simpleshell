/*
 * cli.hpp - Simple framework for writing line-oriented command interpreters
 *
 *   Copyright 2010-2011 Jesús Torres <jmtorres@ull.es>
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

#include <boost/shared_ptr.hpp>

#include <cli/callbacks.hpp>
#include <cli/internals.hpp>
#include <cli/readline.hpp>

namespace cli
{
    //
    // Class CommandLineInterpreter
    //

    template <template<typename> class Parser>
    class CommandLineInterpreter
    {
        public:
            typedef CommandLineInterpreter<Parser> Type;

            typedef Parser<std::string::iterator> ParserType;
            typedef typename ParserType::ReturnType CommandType;

            //
            // Class constructors
            //

            CommandLineInterpreter(bool useReadline = true);
            CommandLineInterpreter(std::istream& in, std::ostream& out,
                bool useReadline = true);
            CommandLineInterpreter(boost::shared_ptr<ParserType> parser,
                bool useReadline = true);
            CommandLineInterpreter(ParserType* parser,
                bool useReadline = true);
            CommandLineInterpreter(boost::shared_ptr<ParserType> parser,
                std::istream& in, std::ostream& out, bool useReadline = true);
            CommandLineInterpreter(ParserType* parser, std::istream& in,
                std::ostream& out, bool useReadline = true);

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
            template <template <typename> class Callback, typename Functor,
                typename Argument>
            void setCallback(Functor function, Argument argument);

        protected:

            //
            // Hook methods invoked for command execution
            //

            virtual bool doCommand(CommandType const& command);
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

            //
            // Command-line parser factory
            //

            virtual ParserType* parserFactory();

        private:
            std::istream& in_;
            std::ostream& out_;
            readline::Readline readLine_;

            boost::shared_ptr<ParserType> lineParser_;

            std::string introText_;
            std::string promptText_;
            std::string lastCommand_;

            CLI_DECLARE_CALLBACKS(
                Type,
                (callback::DoCommandCallback, doCommandCallback_)
                (callback::EmptyLineCallback, emptyLineCallback_)
                (callback::PreDoCommandCallback, preDoCommandCallback_)
                (callback::PostDoCommandCallback, postDoCommandCallback_)
                (callback::PreLoopCallback, preLoopCallback_)
                (callback::PostLoopCallback, postLoopCallback_)
            )

            //
            // No-op memory deallocator
            //

            struct noOpDelete
            {
                void operator()(ParserType*) {}
            };
    };

    template <template <typename> class Parser>
    CommandLineInterpreter<Parser>::CommandLineInterpreter(
        bool useReadline)
        : in_(std::cin),
          out_(std::cout),
          readLine_(std::cin, std::cout, useReadline),
          lineParser_(parserFactory())
    {}

    template <template <typename> class Parser>
    CommandLineInterpreter<Parser>::CommandLineInterpreter(
        std::istream& in,
        std::ostream& out,
        bool useReadline)
        : in_(in),
          out_(out),
          readLine_(in, out, useReadline),
          lineParser_(parserFactory())
    {}

    template <template <typename> class Parser>
    CommandLineInterpreter<Parser>::CommandLineInterpreter(
        boost::shared_ptr<ParserType> parser,
        bool useReadline)
        : in_(std::cin),
          out_(std::cout),
          readLine_(std::cin, std::cout, useReadline),
          lineParser_(parser)
    {}

    template <template <typename> class Parser>
    CommandLineInterpreter<Parser>::CommandLineInterpreter(
        ParserType* parser,
        bool useReadline)
        : in_(std::cin),
          out_(std::cout),
          readLine_(std::cin, std::cout, useReadline),
          lineParser_(parser, noOpDelete())
    {}

    template <template <typename> class Parser>
    CommandLineInterpreter<Parser>::CommandLineInterpreter(
        boost::shared_ptr<ParserType> parser,
        std::istream& in,
        std::ostream& out,
        bool useReadline)
        : in_(in),
          out_(out),
          readLine_(in, out, useReadline),
          lineParser_(parser)
    {}

    template <template <typename> class Parser>
    CommandLineInterpreter<Parser>::CommandLineInterpreter(
        ParserType* parser,
        std::istream& in,
        std::ostream& out,
        bool useReadline)
        : in_(in),
          out_(out),
          readLine_(in, out, useReadline),
          lineParser_(parser, noOpDelete())
    {}

    template <template <typename> class Parser>
    typename CommandLineInterpreter<Parser>::ParserType*
    CommandLineInterpreter<Parser>::parserFactory()
    {
        return new ParserType;
    }

    template <template <typename> class Parser>
    void CommandLineInterpreter<Parser>::loop()
    {
        preLoop();

        if (internals::isStreamTty(in_) && internals::isStreamTty(out_)) {
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

    template <template <typename> class Parser>
    bool CommandLineInterpreter<Parser>::interpretOneLine(
        std::string line)
    {
        preDoCommand(line);

        if (internals::isLineEmpty(line)) {
            return emptyLine();
        }
        else {
            lastCommand_ = line;
        }

        std::string::iterator begin = line.begin();
        std::string::iterator end = line.end();
        while (begin != end) {
            CommandType command;
            bool success = (*lineParser_)(begin, end, command);
            if (! success)
                break;

            bool isFinished = doCommand(command);
            isFinished = postDoCommand(isFinished, line);
            if (isFinished)
                return true;
        }

        return false;
    }

    template <template <typename> class Parser>
    bool CommandLineInterpreter<Parser>::doCommand(
        CommandType const& command)
    {
        return doCommandCallback_.empty() ?
            false : doCommandCallback_(command);
    }

    template <template <typename> class Parser>
    bool CommandLineInterpreter<Parser>::emptyLine()
    {
        if (! emptyLineCallback_.empty()) {
            return emptyLineCallback_();
        }
        else if (! lastCommand_.empty()) {
            return interpretOneLine(lastCommand_);
        }
        return false;
    }

    template <template <typename> class Parser>
    bool CommandLineInterpreter<Parser>::postDoCommand(
        bool isFinished, const std::string& line)
    {
        return postDoCommandCallback_.empty() ?
            isFinished : postDoCommandCallback_(isFinished, line);
    }

    template <template <typename> class Parser>
    void CommandLineInterpreter<Parser>::preLoop()
    {
        if (! preLoopCallback_.empty()) {
            preLoopCallback_();
        }
    }

    template <template <typename> class Parser>
    void CommandLineInterpreter<Parser>::postLoop()
    {
        if (! postLoopCallback_.empty()) {
            postLoopCallback_();
        }
    }

    template <template <typename> class Parser>
    void CommandLineInterpreter<Parser>::setHistoryFile(
        const std::string& fileName)
    {
        readLine_.clearHistory();
        readLine_.setHistoryFile(fileName);
    }

    template <template <typename> class Parser>
    template <template <typename> class Callback, typename Functor>
    void CommandLineInterpreter<Parser>::setCallback(Functor function)
    {
        CLI_CALLBACK_SIGNATURE_ASSERT(Callback, Functor);
        callback::SetCallbackImpl<Callback>::setCallback(*this, function);
    }

    template <template <typename> class Parser>
    template <template <typename> class Callback, typename Functor,
        typename Argument>
    void CommandLineInterpreter<Parser>::setCallback(Functor function,
        Argument argument)
    {
        CLI_CALLBACK_SIGNATURE_ASSERT(Callback, Functor);
        callback::SetCallbackImpl<Callback>::setCallback(*this, function,
            argument);
    }
}

#endif /* CLI_HPP_ */
