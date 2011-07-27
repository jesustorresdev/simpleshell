/*
 * callbacks.hpp - Framework support of callback functions
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

#ifndef CALLBACKS_HPP_
#define CALLBACKS_HPP_

#include <string>
#include <vector>

#include <boost/function.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/type_traits.hpp>

#include <cli/exceptions.hpp>

//
// Macro CLI_DECLARE_CALLBACKS
//
// Helps to add the support for callbacks to a class declaration.
//

#define CLI_DECLARE_CALLBACKS_FILLER_0(X, Y)    \
    ((X, Y)) CLI_DECLARE_CALLBACKS_FILLER_1
#define CLI_DECLARE_CALLBACKS_FILLER_1(X, Y)    \
    ((X, Y)) CLI_DECLARE_CALLBACKS_FILLER_0
#define CLI_DECLARE_CALLBACKS_FILLER_0_END
#define CLI_DECLARE_CALLBACKS_FILLER_1_END

#define CLI_DECLARE_CALLBACK_MEMBER(r, TYPE, DECLARATION_TUPLE)     \
    typedef typename cli::callback::                                \
        BOOST_PP_TUPLE_ELEM(2, 0, DECLARATION_TUPLE)<TYPE>::Type    \
        BOOST_PP_TUPLE_ELEM(2, 0, DECLARATION_TUPLE);               \
    boost::function<BOOST_PP_TUPLE_ELEM(2, 0, DECLARATION_TUPLE)>   \
        BOOST_PP_TUPLE_ELEM(2, 1, DECLARATION_TUPLE);

#define CLI_DECLARE_CALLBACKS(TYPE, DECLARATIONS_SEQ)               \
    BOOST_PP_SEQ_FOR_EACH(                                          \
        CLI_DECLARE_CALLBACK_MEMBER,                                \
        TYPE,                                                       \
        BOOST_PP_CAT(                                               \
            CLI_DECLARE_CALLBACKS_FILLER_0 DECLARATIONS_SEQ, _END)) \
    template <template <typename> class Callback>                   \
    friend class cli::callback::SetCallbackImpl;

//
// Macro CLI_CALLBACK_SIGNATURE_ASSERT
//
// Tests if the function signature of FUNCTOR match the expected by CALLBACK.
//

#define CLI_CALLBACK_SIGNATURE_ASSERT(CALLBACK, FUNCTOR)                \
    BOOST_MPL_ASSERT((boost::is_convertible<FUNCTOR,                    \
        boost::function<typename CALLBACK<Type>::Type> >))


namespace cli { namespace callback
{
    //
    // Base template for callback function setter implementation
    //

    template <template <typename> class Callback>
    struct SetCallbackImpl
    {
        template <typename T, typename Functor>
        static void setCallback(T& interpreter, Functor function)
        {
            throw cli::exception::UnknownCallbackException(
                Callback<T>::name());
        }
    };

    //
    // DoCommandCallback
    //

    template <typename T>
    struct DoCommandCallback
    {
        typedef bool (Type)(const std::string&,
            typename T::CommandDetailsType const&);
        static const char* name() { return "DoCommandCallback"; }
    };

    template <>
    struct SetCallbackImpl<DoCommandCallback>
    {
        template <typename T, typename Functor>
        static void setCallback(T& interpreter, Functor function)
            { interpreter.defaultDoCommandCallback_ = function; }
        template <typename T, typename Functor>
        static void setCallback(T& interpreter, Functor function,
            const std::string& command)
            { interpreter.doCommandCallbacks_[command] = function; }
    };

    //
    // EmptyLineCallback
    //

    template <typename T>
    struct EmptyLineCallback
    {
        typedef bool (Type)();
        static const char* name() { return "EmptyLineCallback"; }
    };

    template <>
    struct SetCallbackImpl<EmptyLineCallback>
    {
        template <typename T, typename Functor>
        static void setCallback(T& interpreter, Functor function)
            { interpreter.emptyLineCallback_ = function; }
    };

    //
    // PreDoCommandCallback
    //

    template <typename T>
    struct PreDoCommandCallback
    {
        typedef void (Type)(std::string&);
        static const char* name() { return "PreDoCommandCallback"; }
    };

    template <>
    struct SetCallbackImpl<PreDoCommandCallback>
    {
        template <typename T, typename Functor>
        static void setCallback(T& interpreter, Functor function)
            { interpreter.preDoCommandCallback_ = function; }
    };

    //
    // PostDoCommandCallback
    //

    template <typename T>
    struct PostDoCommandCallback
    {
        typedef bool (Type)(bool, const std::string&);
        static const char* name() { return "PostDoCommandCallback"; }
    };

    template <>
    struct SetCallbackImpl<PostDoCommandCallback>
    {
        template <typename T, typename Functor>
        static void setCallback(T& interpreter, Functor function)
            { interpreter.postDoCommandCallback_ = function; }
    };

    //
    // ParserErrorCallback
    //

    template <typename T>
    struct ParserErrorCallback
    {
            typedef bool (Type)(typename T::ParserErrorType const&,
                const std::string&);
            static const char* name() { return "ParserErrorCallback"; }
    };

    template <>
    struct SetCallbackImpl<ParserErrorCallback>
    {
            template <typename T, typename Functor>
            static void setCallback(T& interpreter, Functor function)
            { interpreter.parserErrorCallback_ = function; }
    };

    //
    // PreLoopCallback
    //

    template <typename T>
    struct PreLoopCallback
    {
        typedef void (Type)();
        static const char* name() { return "PreLoopCallback"; }
    };

    template <>
    struct SetCallbackImpl<PreLoopCallback>
    {
        template <typename T, typename Functor>
        static void setCallback(T& interpreter, Functor function)
            { interpreter.preLoopCallback_ = function; }
    };

    //
    // PostLoopCallback
    //

    template <typename T>
    struct PostLoopCallback
    {
        typedef void (Type)();
        static const char* name() { return "PostLoopCallback"; }
    };

    template <>
    struct SetCallbackImpl<PostLoopCallback>
    {
        template <typename T, typename Functor>
        static void setCallback(T& interpreter, Functor function)
            { interpreter.postLoopCallback_ = function; }
    };

    //
    // VariableLookupCallback
    //

    template <typename T>
    struct VariableLookupCallback
    {
        typedef std::string (Type)(const std::string&);
        static const char* name() { return "VariableLookupCallback"; }
    };

    template <>
    struct SetCallbackImpl<VariableLookupCallback>
    {
        template <typename T, typename Functor>
        static void setCallback(T& interpreter, Functor function)
            { interpreter.lineParser_->variableLookupCallback_ = function; }
    };

    //
    // PathnameExpansionCallback
    //

    template <typename T>
    struct PathnameExpansionCallback
    {
        typedef std::vector<std::string> (Type)(const std::string&);
        static const char* name() { return "PathnameExpansionCallback"; }
    };

    template <>
    struct SetCallbackImpl<PathnameExpansionCallback>
    {
        template <typename T, typename Functor>
        static void setCallback(T& interpreter, Functor function)
            { interpreter.lineParser_->PathnameExpansionCallback_ = function; }
    };
}}

#endif /* CALLBACKS_HPP_ */
