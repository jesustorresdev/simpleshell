/*
 * callbacks.hpp - Framework support of callback functions
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

#ifndef CALLBACKS_HPP_
#define CALLBACKS_HPP_

#include <string>
#include <map>
#include <vector>

#include <boost/function.hpp>

namespace cli { namespace callback
{
    //
    // Class CallbackSignature
    //
    // Trait class used to retrieve the function signature of a callback.
    //

    template <typename Callback>
    struct CallbackSignature
    {};

    //
    // Class CallbackBase
    //

    template <typename Callback>
    class CallbackBase
    {
        public:
            typedef CallbackBase<Callback> Type;
            typedef typename CallbackSignature<Callback>::ValueType Signature;

            const boost::function<Signature>& callable() const
               { return callable_; }

            void operator()(boost::function<Signature> const& callback)
               { callable_ = callback; }

            operator bool() const
               { return ! callable_.empty(); };

        private:
            boost::function<Signature> callable_;
    };

    //
    // Callback types for cli::CommandLineInterpreterBase class
    //

    template <>
    struct CallbackSignature<RunCommandCallback>
    {
        typedef bool (Type)(const std::string&,
            typename Interpreter::ArgumentsType const&);
    }

    template <typename Interpreter>
    class RunCommandCallback
        : public CallbackBase<RunCommandCallback<Interpreter> >
    {
        public:


            typedef CallbackBase<RunCommandCallback< Interpreter> > BaseType;

            using BaseType::operator();

            const boost::function<typename BaseType::Signature>& callable(
                const std::string& command) const
            {
                typename CallbacksList::const_iterator i;
                i = callbacks_.find(command);
                if (i == callbacks_.end()) {
                    return BaseType::operator bool() ?
                        BaseType::callable() : false;
                }
                    else {
                        return i->second;
                }
            }

            void operator()(const std::string& command,
                boost::function<typename BaseType::Signature> const& callback)
            {
                callbacks_[command] = callback;
            }

            operator bool() const
            {
                return BaseType::operator bool() || (! callbacks_.empty());
            }

        private:
            typedef std::map<std::string,
                boost::function<typename BaseType::Signature> >
            CallbacksList;

            CallbacksList callbacks_;
    };


    template <typename Callback>
    struct CallbackSignature
    {};

    template <>
    struct CallbackSignature<RunCommandCallback>


    template <typename Interpreter>
    struct EmptyLineCallback
        : public CallbackBase<EmptyLineCallback<Interpreter> >
    {
        typedef bool (Type)();
    };

    template <typename Interpreter>
    struct PreRunCommandCallback
        : public CallbackBase<PreRunCommandCallback<Interpreter> >
    {
        typedef void (Type)(std::string&);
    };

    template <typename Interpreter>
    struct PostRunCommandCallback
        : public CallbackBase<PostRunCommandCallback<Interpreter> >
    {
        typedef bool (Type)(bool, const std::string&);
    };

    template <typename Interpreter>
    struct PreLoopCallback
        : public CallbackBase<PreLoopCallback<Interpreter> >
    {
        typedef void (Type)();
    };

    template <typename Interpreter>
    struct PostLoopCallback
        : public CallbackBase<PostLoopCallback<Interpreter> >
    {
        typedef void (Type)();
    };

    template <typename Interpreter>
    struct ParseErrorCallback
        : public CallbackBase<ParseErrorCallback<Interpreter> >
    {
        typedef bool (Type)(typename Interpreter::ParseErrorType const&,
            const std::string&);
    };

    //
    // Callback types for cli::ShellInterpreter class
    //

    template <typename Interpreter>
    struct VariableLookupCallback
        : public CallbackBase<VariableLookupCallback<Interpreter> >
    {
        typedef std::string (Type)(const std::string&);
    };

    template <typename Interpreter>
    struct PathnameExpansionCallback
        : public CallbackBase<PathnameExpansionCallback<Interpreter> >
    {
        typedef std::vector<std::string> (Type)(const std::string&);
    };
}}

#endif /* CALLBACKS_HPP_ */
