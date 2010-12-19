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
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

#include <cli/exception.hpp>
#include <cli/auxiliary.hpp>
#include <cli/parser.hpp>
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

            // Callback functions signatures
            typedef bool (RunCommandCallback)(ClassType*, const Command&);
            typedef bool (RunEmptyLineCallback)(ClassType*);
            typedef void (PreRunCommandCallback)(ClassType*, std::string&);
            typedef bool (PostRunCommandCallback)
                (ClassType*, bool, const std::string&);
            typedef void (PreLoopCallback)(ClassType*);
            typedef void (PostLoopCallback)(ClassType*);

            CommandLineInterpreter(
                const std::string &historyFileName = std::string(),
                std::istream& in = std::cin,
                std::ostream& out = std::cout);

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

            template <typename Signature>
            void setCallback(const std::string& callbackName,
                const boost::function<Signature>& function);

        protected:
            virtual bool defaultRunCommand(const Command& command);
            virtual bool defaultRunEmptyLine();

            // Hook methods invoked inside interpretOneLine()
            virtual void defaultPreRunCommand(std::string& line) {};
            virtual bool defaultPostRunCommand(bool isFinished,
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

            boost::function<RunCommandCallback> runCommand_;
            boost::function<RunEmptyLineCallback> runEmptyLine_;
            boost::function<PreRunCommandCallback> preRunCommand_;
            boost::function<PostRunCommandCallback> postRunCommand_;
            boost::function<PreLoopCallback> preLoop_;
            boost::function<PostLoopCallback> postLoop_;

            //
            // Internal callback function setter
            //

            template <typename S1, typename S2>
            typename boost::enable_if<typename boost::is_same<S1, S2>::type,
                boost::function<S1>& >::type
            internalSetCallback(boost::function<S1>& function1,
                const boost::function<S2>& function2)
            {
                function1 = function2;
                return function1;
            }

            template <typename S1, typename S2>
            typename boost::disable_if<typename boost::is_same<S1, S2>::type,
                boost::function<S1>& >::type
            internalSetCallback(boost::function<S1>& function1,
                const boost::function<S2>& function2)
            {
                throw exception::IncompatibleSignatureException();
            }

            //
            // Dispatching of setCallback() calls to the parser
            //

            template <typename T>
            typename boost::enable_if<
                typename boost::is_base_of<parser::BaseParser,
                    ParserType>::type, T>::type
            parserSetCallback(const std::string& callbackName, T& function)
                { lineParser_->setCallback(callbackName, function); }

            template <typename T>
            typename boost::disable_if<
                typename boost::is_base_of<parser::BaseParser,
                    ParserType>::type, void>::type
            parserSetCallback(const std::string& callbackName, T& function)
                { throw exception::UnknownCallbackException(callbackName); }
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
          runCommand_(&ClassType::defaultRunCommand),
          runEmptyLine_(&ClassType::defaultRunEmptyLine),
          preRunCommand_(&ClassType::defaultPreRunCommand),
          postRunCommand_(&ClassType::defaultPostRunCommand),
          preLoop_(&ClassType::defaultPreLoop),
          postLoop_(&ClassType::defaultPostLoop)
    {}

    template <template <typename> class Parser, typename Command>
    void CommandLineInterpreter<Parser, Command>::loop()
    {
        preLoop_(this);

        if (auxiliary::isStreamTty(out_)) {
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

        postLoop_(this);
    }

    template <template <typename> class Parser, typename Command>
    bool CommandLineInterpreter<Parser, Command>::interpretOneLine(
        std::string line)
    {
        preRunCommand_(this, line);

        if (auxiliary::isLineEmpty(line)) {
            return runEmptyLine_(this);
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
            bool isFinished = runCommand_(this, command);
            return postRunCommand_(this, isFinished, line);
        }
        else {
            std::string no_parsed(begin, end);
            out_ << no_parsed
                 << ": " << translate("syntax error") << std::endl;
            return false;
        }
    }

    template <template <typename> class Parser, typename Command>
    template <typename Signature>
    void CommandLineInterpreter<Parser, Command>::setCallback(
        const std::string& callbackName,
        const boost::function<Signature>& function)
    {
        if (callbackName == "runCommand") {
            internalSetCallback(runCommand_, function);
        }
        else if (callbackName == "runEmptyLine") {
            internalSetCallback(runEmptyLine_, function);
        }
        else if (callbackName == "preRunCommand") {
            internalSetCallback(preRunCommand_, function);
        }
        else if (callbackName == "postRunCommand") {
            internalSetCallback(postRunCommand_, function);
        }
        else if (callbackName == "preLoop") {
            internalSetCallback(preLoop_, function);
        }
        else if (callbackName == "postLoop") {
            internalSetCallback(postLoop_, function);
        }
        else {
            parserSetCallback(callbackName, function);
        }
    }

    template <template <typename> class Parser, typename Command>
    bool CommandLineInterpreter<Parser, Command>::defaultRunCommand(
            const Command& command)
    {
        return false;
    }

    template <template <typename> class Parser, typename Command>
    bool CommandLineInterpreter<Parser, Command>::defaultRunEmptyLine()
    {
        if (! lastCommand_.empty()) {
            return interpretOneLine(lastCommand_);
        }
        return false;
    }

    template <template <typename> class Parser, typename Command>
    bool CommandLineInterpreter<Parser, Command>::defaultPostRunCommand(
        bool isFinished, const std::string& line)
    {
        return isFinished;
    }
}

#endif /* CLI_HPP_ */
