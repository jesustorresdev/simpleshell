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
    template <typename Return, typename... Arguments>
    class Callback
    {
        public:
            typedef Callback<Return, Arguments...> Type;
            typedef Return (Signature)(Arguments...);

            Return call(Arguments... arguments) const
               { return callback_(arguments...); }

            void operator()(const boost::function<Signature>& callback)
               { callback_ = callback; }

            operator bool() const
               { return ! callback_.empty(); };

        private:
            boost::function<Signature> callback_;
    };

    //
    // Callback types for cli::CommandLineInterpreterBase class
    //

    template <typename Interpreter>
    class RunCommandCallback
        : public Callback<bool, const std::string&,
            typename Interpreter::ArgumentsType const&>
    {
        public:
            typedef typename Interpreter::ArgumentsType ArgumentsType;
            typedef Callback<bool, const std::string&,
                ArgumentsType const&> BaseType;
            typedef typename BaseType::Signature Signature;

            bool call(const std::string& command,
                ArgumentsType const& arguments) const
            {
                typename std::map<std::string,
                    boost::function<Signature> >::const_iterator i;
                i = callbacks_.find(command);
                if (i == callbacks_.end()) {
                    return BaseType::call(command, arguments);
                }
                else {
                    return i->second(command, arguments);
                }
            }

            using BaseType::operator();

            void operator()(const std::string& command,
                boost::function<Signature> const& callback)
            {
                callbacks_[command] = callback;
            }

        private:
            std::map<std::string, boost::function<Signature> > callbacks_;
    };

    typedef Callback<bool> EmptyLineCallback;
    typedef Callback<void, std::string&> PreRunCommandCallback;
    typedef Callback<bool, bool, const std::string&> PostRunCommandCallback;
    typedef Callback<void> PreLoopCallback;
    typedef Callback<void> PostLoopCallback;

    template <typename Interpreter>
    struct ParseErrorCallback
        : public Callback<bool, typename Interpreter::ParseErrorType const&,
            const std::string&>
    {};

    //
    // Callback types for cli::ShellInterpreter class
    //

    typedef Callback<std::string, const std::string&> VariableLookupCallback;
    typedef Callback<std::vector<std::string>, const std::string&>
        PathnameExpansionCallback;
}}

#endif /* CALLBACKS_HPP_ */
