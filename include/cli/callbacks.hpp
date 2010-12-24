/*
 * callbacks.hpp - Framework support of callback functions
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

#ifndef CALLBACKS_HPP_
#define CALLBACKS_HPP_

#include <cli/exceptions.hpp>

namespace cli { namespace callbacks
{
    //
    // Base template for callback function setter implementation
    //

    template <template <typename> class Callback>
    struct SetCallbackImpl
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
        {
            throw cli::exceptions::UnknownCallbackException(
                Callback<Interpreter>::name());
        }
    };

    //
    // RunCommandCallback
    //

    template <typename Interpreter>
    struct RunCommandCallback
    {
        typedef bool (Type)(Interpreter*,
            const typename Interpreter::CommandType&);

        static const char* name() { return "RunCommand"; }
    };

    template<>
    struct SetCallbackImpl<RunCommandCallback>
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
            { interpreter.runCommand_ = function; }
    };

    //
    // RunEmptyLineCallback
    //

    template <typename Interpreter>
    struct RunEmptyLineCallback
    {
        typedef bool (Type)(Interpreter*);

        static const char* name() { return "RunEmptyLine"; }
    };

    template<>
    struct SetCallbackImpl<RunEmptyLineCallback>
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
            { interpreter.runEmptyLine_ = function; }
    };

    //
    // PreRunCommandCallback
    //

    template <typename Interpreter>
    struct PreRunCommandCallback
    {
        typedef void (Type)(Interpreter*, std::string&);

        static const char* name() { return "PreRunCommand"; }
    };

    template<>
    struct SetCallbackImpl<PreRunCommandCallback>
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
            { interpreter.preRunCommand_ = function; }
    };

    //
    // PostRunCommandCallback
    //

    template <typename Interpreter>
    struct PostRunCommandCallback
    {
        typedef bool (Type)(Interpreter*, bool, const std::string&);

        static const char* name() { return "PostRunCommand"; }
    };

    template<>
    struct SetCallbackImpl<PostRunCommandCallback>
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
            { interpreter.postRunCommand_ = function; }
    };

    //
    // PreLoopCallback
    //

    template <typename Interpreter>
    struct PreLoopCallback
    {
        typedef void (Type)(Interpreter*);

        static const char* name() { return "PreLoop"; }
    };

    template<>
    struct SetCallbackImpl<PreLoopCallback>
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
            { interpreter.preLoop_ = function; }
    };

    //
    // PostLoopCallback
    //

    template <typename Interpreter>
    struct PostLoopCallback
    {
        typedef void (Type)(Interpreter*);

        static const char* name() { return "PostLoop"; }
    };

    template<>
    struct SetCallbackImpl<PostLoopCallback>
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
            { interpreter.postLoop_ = function; }
    };
}}

#endif /* CALLBACKS_HPP_ */
