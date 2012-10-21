/*
 * base.hpp - Base class for from which all line-oriented command interpreters
 *            acquire its core functionality
 *
 * Simple framework for writing line-oriented command interpreters
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

#ifndef BASE_HPP_
#define BASE_HPP_

#include <cerrno>
#include <iostream>
#include <map>
#include <string>

#include <boost/function.hpp>

#include <cli/callbacks.hpp>
#include <cli/internals.hpp>
#include <cli/readline.hpp>

namespace cli
{
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
            // Methods to interpret command-line input
            //

            void loop();
            bool interpretOneLine(std::string line);

            //
            // I/O streams getters
            //

            std::istream& getInStream() const
                { return in_; }
            std::ostream& getOutStream() const
                { return out_; }
            std::ostream& getErrStream() const
                { return err_; }

            //
            // Other attributes getters & setters
            //

            const std::string& getLastCommand() const
                { return lastCommand_; }

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

            virtual bool doCommand(const std::string& command,
            	Arguments const& arguments);
            virtual bool emptyLine();

            //
            // Hook methods invoked inside interpretOneLine()
            //

            virtual void preDoCommand(std::string& line) {};
            virtual bool postDoCommand(bool isFinished,
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
            // Callback function objects
            //

            CLI_DECLARE_CALLBACKS_TPL(
                Type,
                (DoCommandCallback, defaultDoCommandCallback_)
                (EmptyLineCallback, emptyLineCallback_)
                (PreDoCommandCallback, preDoCommandCallback_)
                (PostDoCommandCallback, postDoCommandCallback_)
                (ParseErrorCallback, parseErrorCallback_)
                (PreLoopCallback, preLoopCallback_)
                (PostLoopCallback, postLoopCallback_)
            )

  	    	typedef std::map<std::string,
                boost::function<DoCommandCallback> > DoCommandCallbacks;
            DoCommandCallbacks doCommandCallbacks_;

            template <int M>
            struct SetCallbackImpl<cli::callback::DoCommandCallback, 1, M>
            {
                template <typename T, typename Functor>
                static void setCallback(T& interpreter, Functor function,
                    const std::string& command)
                    { interpreter.doCommandCallbacks_[command] = function; }
            };

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
        preLoop();

        std::string promptText;
        if (internals::isStreamTty(in_) && internals::isStreamTty(out_)) {
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
        preDoCommand(line);

        if (internals::isLineEmpty(line)) {
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
                isFinished = doCommand(command, arguments);
                isFinished = postDoCommand(isFinished, line);
                if (isFinished)
                    return true;
            }
        }
        return false;
    }

    template <typename Arguments>
    bool CommandLineInterpreterBase<Arguments>::doCommand(
        const std::string& command, Arguments const& arguments)
    {
        typename DoCommandCallbacks::const_iterator i;
        i = doCommandCallbacks_.find(command);
        if (i == doCommandCallbacks_.end()) {
            return defaultDoCommandCallback_.empty() ?
                false : defaultDoCommandCallback_(command, arguments);
        }
        else {
            return i->second(command, arguments);
        }
    }

    template <typename Arguments>
    bool CommandLineInterpreterBase<Arguments>::emptyLine()
    {
        return emptyLineCallback_.empty() ? false : emptyLineCallback_();
    }

    template <typename Arguments>
    bool CommandLineInterpreterBase<Arguments>::postDoCommand(bool isFinished,
        const std::string& line)
    {
        return postDoCommandCallback_.empty() ?
            isFinished : postDoCommandCallback_(isFinished, line);
    }

    template <typename Arguments>
    bool CommandLineInterpreterBase<Arguments>::parseError(
        ParseError const& error, const std::string& line)
    {
        if (parseErrorCallback_.empty()) {
            err_ << ::program_invocation_short_name
                  << ": "
                  << error.what()
                  << std::endl;
            return false;
        }
        return parseErrorCallback_(error, line);
    }

    template <typename Arguments>
    void CommandLineInterpreterBase<Arguments>::preLoop()
    {
        if (! preLoopCallback_.empty()) {
            preLoopCallback_();
        }
    }

    template <typename Arguments>
    void CommandLineInterpreterBase<Arguments>::postLoop()
    {
        if (! postLoopCallback_.empty()) {
            postLoopCallback_();
        }
    }

    template <typename Arguments>
    void CommandLineInterpreterBase<Arguments>::setHistoryFile(
        const std::string& fileName)
    {
        readLine_.clearHistory();
        readLine_.setHistoryFile(fileName);
    }

    template <typename Arguments>
    template <template <typename> class Callback, typename Functor>
    void CommandLineInterpreterBase<Arguments>::setCallback(Functor function)
    {
        CLI_CALLBACK_SIGNATURE_ASSERT_TPL(Callback, Functor);
        SetCallbackImpl<Callback, 0, 0>::setCallback(*this, function);
    }

    template <typename Arguments>
    template <template <typename> class Callback, typename Functor,
        typename Argument>
    void CommandLineInterpreterBase<Arguments>::setCallback(Functor function,
        Argument argument)
    {
        CLI_CALLBACK_SIGNATURE_ASSERT_TPL(Callback, Functor);
        SetCallbackImpl<Callback, 1, 0>::setCallback(*this,
            function, argument);
    }
}

#endif /* BASE_HPP_ */
