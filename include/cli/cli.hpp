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

#include <cerrno>
#include <iostream>
#include <map>
#include <string>

#include <boost/exception/detail/is_output_streamable.hpp>
#include <boost/function.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/is_nonmember_callable_builtin.hpp>
#include <boost/function_types/is_callable_builtin.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/int.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>

#include <cli/callbacks.hpp>
#include <cli/internals.hpp>
#include <cli/readline.hpp>

namespace cli
{
    using boost::false_type;

    //
    // Class CommandLineInterpreter
    //

    template <typename Parser>
    class CommandLineInterpreter
    {
        public:
            typedef CommandLineInterpreter<Parser> Type;

            //
            // By default, we expect Parser to be an object that models STL
            // function object concepts.
            //

            template <typename T, typename Enabled = void>
            struct ParserTraits {};

            template <typename T>
            struct ParserTraits<T, typename boost::disable_if<
                boost::function_types::is_nonmember_callable_builtin<T> >::type>
            {
                typedef typename T::arg4_type CommandArgumentsType;
                typedef typename T::result_type ParserErrorType;
            };

            //
            // But also, Parser can be a function, function pointer or
            // function reference.
            //

            template <typename T>
            struct ParserTraits<T, typename boost::enable_if<
                boost::function_types::is_nonmember_callable_builtin<T> >::type>
            {
                BOOST_STATIC_ASSERT((
                    boost::function_types::function_arity<T>::value == 4));

                typedef typename boost::mpl::at<
                    typename boost::function_types::parameter_types<T>::type,
                    boost::mpl::int_<3>
                >::type CommandArgumentsType;
                typedef typename boost::function_types::result_type<T>::type
                    ParserErrorType;
            };

            //
            // Define types related to parser type
            //

            typedef typename boost::remove_reference<
                typename ParserTraits<Parser>::CommandArgumentsType
            >::type CommandArgumentsType;
            typedef typename ParserTraits<Parser>::ParserErrorType
                ParserErrorType;
            typedef ParserErrorType (ParserSignature)(std::string::iterator&,
                std::string::iterator, std::string&, CommandArgumentsType&);

            //
            // Class constructors
            //

            CommandLineInterpreter(bool useReadline = true);
            CommandLineInterpreter(boost::shared_ptr<Parser> parser,
                bool useReadline = true);
            CommandLineInterpreter(Parser parser, bool useReadline = true);

            //
            // Methods to interpret command-line input
            //

            void loop();
            bool interpretOneLine(std::string line);

            //
            // I/O streams getters & setters
            //

            std::istream& getInStream() const
                { return *in_; }
            std::ostream& getOutStream() const
                { return *out_; }
            std::ostream& getErrStream() const
                { return *err_; }

            void setInStream(std::istream& in)
                { in_ = &in; readLine_.setInStream(in); }
            void setOutStream(std::ostream& out)
                { out_ = &out; readLine_.setOutStream(out); }
            void setErrStream(std::ostream& err)
                { err_ = &err; }

            void setIOStreams(std::istream& in, std::ostream& out,
                std::ostream& err = std::cerr);

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
                CommandArgumentsType const& arguments);
            virtual bool emptyLine();

            //
            // Hook methods invoked inside interpretOneLine()
            //

            virtual void preDoCommand(std::string& line) {};
            virtual bool postDoCommand(bool isFinished,
                const std::string& line);
            virtual bool parserError(ParserErrorType const& error,
                const std::string& line);

            //
            // Hook methods invoked once inside loop()
            //

            virtual void preLoop();
            virtual void postLoop();

            //
            // Method to print parser errors, if possible
            //

            template <typename Error>
            typename boost::enable_if<
                boost::is_output_streamable<Error> >::type
            printParserError(Error const& error);

            template <typename Error>
            typename boost::disable_if<
                boost::is_output_streamable<Error> >::type
            printParserError(Error const& error) {}

        private:
            std::istream* in_;
            std::ostream* out_;
            std::ostream* err_;
            readline::Readline readLine_;

            boost::shared_ptr<Parser> parserObject_;
            boost::function<ParserSignature> parserFunction_;

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
                (ParserErrorCallback, parserErrorCallback_)
                (PreLoopCallback, preLoopCallback_)
                (PostLoopCallback, postLoopCallback_)
            )

            typedef std::map<std::string,
                boost::function<DoCommandCallback> > DoCommandCallbacks;
            DoCommandCallbacks doCommandCallbacks_;

            //
            // Parser handling
            //

            template <typename P>
            typename boost::enable_if<
                boost::function_types::is_callable_builtin<P>, P*>::type
            parserFactory()
                { return NULL; }

            template <typename P>
            typename boost::disable_if<
                boost::function_types::is_callable_builtin<P>, P*>::type
            parserFactory()
                { return new P; }

            ParserErrorType parse(std::string::iterator& begin,
                std::string::iterator end, std::string& command,
                CommandArgumentsType& arguments);
    };

    template <typename Parser>
    CommandLineInterpreter<Parser>::CommandLineInterpreter(
        bool useReadline)
        : in_(&std::cin),
          out_(&std::cout),
          err_(&std::cerr),
          readLine_(useReadline),
          parserObject_(parserFactory<Parser>())
    {}

    template <typename Parser>
    CommandLineInterpreter<Parser>::CommandLineInterpreter(
        boost::shared_ptr<Parser> parser, bool useReadline)
        : in_(&std::cin),
          out_(&std::cout),
          err_(&std::cerr),
          readLine_(useReadline),
          parserObject_(parser)
    {}

    template <typename Parser>
    CommandLineInterpreter<Parser>::CommandLineInterpreter(
        Parser parser, bool useReadline)
        : in_(&std::cin),
          out_(&std::cout),
          err_(&std::cerr),
          readLine_(useReadline),
          parserFunction_(parser)
    {}

    template <typename Parser>
    void CommandLineInterpreter<Parser>::loop()
    {
        preLoop();

        std::string promptText;
        if (internals::isStreamTty(*in_) && internals::isStreamTty(*out_)) {
            *out_ << introText_ << std::endl;
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
    bool CommandLineInterpreter<Parser>::interpretOneLine(std::string line)
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
            std::string command;
            CommandArgumentsType arguments;
            ParserErrorType error = parse(begin, end, command, arguments);

            bool isFinished;
            if (error) {
                isFinished = parserError(error, line);
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

    template <typename Parser>
    bool CommandLineInterpreter<Parser>::doCommand(const std::string& command,
        CommandArgumentsType const& arguments)
    {
        typename DoCommandCallbacks::const_iterator it;
        it = doCommandCallbacks_.find(command);
        if (it == doCommandCallbacks_.end()) {
            return defaultDoCommandCallback_.empty() ?
                false : defaultDoCommandCallback_(command, arguments);
        }
        else {
            return it->second(command, arguments);
        }
    }

    template <typename Parser>
    bool CommandLineInterpreter<Parser>::emptyLine()
    {
        return emptyLineCallback_.empty() ? false : emptyLineCallback_();
    }

    template <typename Parser>
    bool CommandLineInterpreter<Parser>::postDoCommand(bool isFinished,
        const std::string& line)
    {
        return postDoCommandCallback_.empty() ?
            isFinished : postDoCommandCallback_(isFinished, line);
    }

    template <typename Parser>
    bool CommandLineInterpreter<Parser>::parserError(
        ParserErrorType const& error, const std::string& line)
    {
        if (parserErrorCallback_.empty()) {
            printParserError(error);
            return false;
        }
        return parserErrorCallback_(error, line);
    }

    template <typename Parser>
    void CommandLineInterpreter<Parser>::preLoop()
    {
        if (! preLoopCallback_.empty()) {
            preLoopCallback_();
        }
    }

    template <typename Parser>
    void CommandLineInterpreter<Parser>::postLoop()
    {
        if (! postLoopCallback_.empty()) {
            postLoopCallback_();
        }
    }

    template <typename Parser>
    void CommandLineInterpreter<Parser>::setIOStreams(std::istream& in,
        std::ostream& out, std::ostream& err)
    {
        setInStream(in);
        setOutStream(out);
        setErrStream(err);
    }

    template <typename Parser>
    void CommandLineInterpreter<Parser>::setHistoryFile(
        const std::string& fileName)
    {
        readLine_.clearHistory();
        readLine_.setHistoryFile(fileName);
    }

    template <typename Parser>
    template <template <typename> class Callback, typename Functor>
    void CommandLineInterpreter<Parser>::setCallback(Functor function)
    {
        CLI_CALLBACK_SIGNATURE_ASSERT_TPL(Callback, Functor);
        callback::SetCallbackImpl<Callback>::setCallback(*this, function);
    }

    template <typename Parser>
    template <template <typename> class Callback, typename Functor,
        typename Argument>
    void CommandLineInterpreter<Parser>::setCallback(Functor function,
        Argument argument)
    {
        CLI_CALLBACK_SIGNATURE_ASSERT_TPL(Callback, Functor);
        callback::SetCallbackImpl<Callback>::setCallback(*this, function,
            argument);
    }

    template <typename Parser>
    template <typename Error>
    typename boost::enable_if<boost::is_output_streamable<Error> >::type
    CommandLineInterpreter<Parser>::printParserError(Error const& error)
    {
        *err_ << ::program_invocation_short_name
              << ": "
              << error
              << std::endl;
    }

    template <typename Parser>
    typename CommandLineInterpreter<Parser>::ParserErrorType
    CommandLineInterpreter<Parser>::parse(std::string::iterator& begin,
        std::string::iterator end, std::string& command,
        CommandArgumentsType& arguments)
    {
        if (parserObject_) {
            return (*parserObject_)(begin, end, command, arguments);
        }
        else {
            return parserFunction_(begin, end, command, arguments);
        }
    }
}

#endif /* CLI_HPP_ */
