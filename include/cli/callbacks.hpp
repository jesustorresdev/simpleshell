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
    // DoCommandCallback
    //

    template <typename Interpreter>
    struct DoCommandCallback
    {
        typedef bool (Type)(Interpreter*,
            const typename Interpreter::CommandType&);

        static const char* name() { return "DoCommandCallback"; }
    };

    template<>
    struct SetCallbackImpl<DoCommandCallback>
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
            { interpreter.doCommandCallback_ = function; }
    };

    //
    // EmptyLineCallback
    //

    template <typename Interpreter>
    struct EmptyLineCallback
    {
        typedef bool (Type)(Interpreter*);

        static const char* name() { return "EmptyLineCallback"; }
    };

    template<>
    struct SetCallbackImpl<EmptyLineCallback>
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
            { interpreter.emptyLineCallback_ = function; }
    };

    //
    // PreDoCommandCallback
    //

    template <typename Interpreter>
    struct PreDoCommandCallback
    {
        typedef void (Type)(Interpreter*, std::string&);

        static const char* name() { return "PreDoCommandCallback"; }
    };

    template<>
    struct SetCallbackImpl<PreDoCommandCallback>
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
            { interpreter.preDoCommandCallback_ = function; }
    };

    //
    // PostDoCommandCallback
    //

    template <typename Interpreter>
    struct PostDoCommandCallback
    {
        typedef bool (Type)(Interpreter*, bool, const std::string&);

        static const char* name() { return "PostDoCommandCallback"; }
    };

    template<>
    struct SetCallbackImpl<PostDoCommandCallback>
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
            { interpreter.postDoCommandCallback_ = function; }
    };

    //
    // PreLoopCallback
    //

    template <typename Interpreter>
    struct PreLoopCallback
    {
        typedef void (Type)(Interpreter*);

        static const char* name() { return "PreLoopCallback"; }
    };

    template<>
    struct SetCallbackImpl<PreLoopCallback>
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
            { interpreter.preLoopCallback_ = function; }
    };

    //
    // PostLoopCallback
    //

    template <typename Interpreter>
    struct PostLoopCallback
    {
        typedef void (Type)(Interpreter*);

        static const char* name() { return "PostLoopCallback"; }
    };

    template<>
    struct SetCallbackImpl<PostLoopCallback>
    {
        template <typename Interpreter, typename Functor>
        static void setCallback(Interpreter& interpreter, Functor function)
            { interpreter.postLoopCallback_ = function; }
    };
}}

#endif /* CALLBACKS_HPP_ */
