/*
 * callbacks.hpp - Framework support of callback functions
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

#ifndef CALLBACKS_HPP_
#define CALLBACKS_HPP_

#include <string>
#include <vector>

#include <boost/function.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/type_traits.hpp>

#include <cli/exceptions.hpp>

//
// Macro CLI_DECLARE_CALLBACKS & CLI_DECLARE_CALLBACKS_TPL
//
// Helps to add the support for callbacks to a class declaration.
//

#define CLI_DECLARE_CALLBACKS_FILLER_0(X, Y)    \
    ((X, Y)) CLI_DECLARE_CALLBACKS_FILLER_1
#define CLI_DECLARE_CALLBACKS_FILLER_1(X, Y)    \
    ((X, Y)) CLI_DECLARE_CALLBACKS_FILLER_0
#define CLI_DECLARE_CALLBACKS_FILLER_0_END
#define CLI_DECLARE_CALLBACKS_FILLER_1_END

#define CLI_DECLARE_CALLBACK_MEMBER(r, TYPE, DECLARATION_TUPLE)             \
    typedef cli::callback::                                                 \
        BOOST_PP_TUPLE_ELEM(2, 0, DECLARATION_TUPLE)<TYPE>::Type            \
        BOOST_PP_TUPLE_ELEM(2, 0, DECLARATION_TUPLE);                       \
    boost::function<BOOST_PP_TUPLE_ELEM(2, 0, DECLARATION_TUPLE)>           \
        BOOST_PP_TUPLE_ELEM(2, 1, DECLARATION_TUPLE);                       \
    template <int N, int M>                                                 \
    struct SetCallbackImpl<                                                 \
        cli::callback::BOOST_PP_TUPLE_ELEM(2, 0, DECLARATION_TUPLE), N, M>  \
    {                                                                       \
        template <typename T, typename Functor>                             \
        static void setCallback(T& interpreter, Functor function)           \
            { interpreter.BOOST_PP_TUPLE_ELEM(2, 1, DECLARATION_TUPLE) =    \
                function; }                                                 \
    };

#define CLI_DECLARE_CALLBACK_MEMBER_TPL(r, TYPE, DECLARATION_TUPLE)         \
    typedef typename cli::callback::                                        \
        BOOST_PP_TUPLE_ELEM(2, 0, DECLARATION_TUPLE)<TYPE>::Type            \
        BOOST_PP_TUPLE_ELEM(2, 0, DECLARATION_TUPLE);                       \
    boost::function<BOOST_PP_TUPLE_ELEM(2, 0, DECLARATION_TUPLE)>           \
        BOOST_PP_TUPLE_ELEM(2, 1, DECLARATION_TUPLE);                       \
    template <int N, int M>                                                 \
    struct SetCallbackImpl<                                                 \
        cli::callback::BOOST_PP_TUPLE_ELEM(2, 0, DECLARATION_TUPLE), N, M>  \
    {                                                                       \
        template <typename T, typename Functor>                             \
        static void setCallback(T& interpreter, Functor function)           \
            { interpreter.BOOST_PP_TUPLE_ELEM(2, 1, DECLARATION_TUPLE) =    \
                function; }                                                 \
    };

#define CLI_DECLARE_CALLBACKS(TYPE, DECLARATIONS_SEQ)               \
    template <template <typename> class Callback, int N, int M>     \
    struct SetCallbackImpl                                          \
    {                                                               \
        template <typename T, typename Functor>                     \
        static void setCallback(T& interpreter, Functor function)   \
        {                                                           \
            throw cli::exception::UnknownCallbackException(         \
                Callback<T>::name());                               \
        }                                                           \
    };                                                              \
    BOOST_PP_SEQ_FOR_EACH(                                          \
        CLI_DECLARE_CALLBACK_MEMBER,                                \
        TYPE,                                                       \
        BOOST_PP_CAT(                                               \
            CLI_DECLARE_CALLBACKS_FILLER_0 DECLARATIONS_SEQ, _END))

#define CLI_DECLARE_CALLBACKS_TPL(TYPE, DECLARATIONS_SEQ)           \
    template <template <typename> class Callback, int N, int M>     \
    struct SetCallbackImpl                                          \
    {                                                               \
        template <typename T, typename Functor>                     \
        static void setCallback(T& interpreter, Functor function)   \
        {                                                           \
            throw cli::exception::UnknownCallbackException(         \
                Callback<T>::name());                               \
        }                                                           \
    };                                                              \
    BOOST_PP_SEQ_FOR_EACH(                                          \
        CLI_DECLARE_CALLBACK_MEMBER_TPL,                            \
        TYPE,                                                       \
        BOOST_PP_CAT(                                               \
            CLI_DECLARE_CALLBACKS_FILLER_0 DECLARATIONS_SEQ, _END))

//
// Macro CLI_CALLBACK_SIGNATURE_ASSERT & CLI_CALLBACK_SIGNATURE_ASSERT_TPL
//
// Tests if the function signature of FUNCTOR match the expected by CALLBACK.
//

#define CLI_CALLBACK_SIGNATURE_ASSERT(CALLBACK, FUNCTOR)                \
    BOOST_MPL_ASSERT((boost::is_convertible<FUNCTOR,                    \
        boost::function<CALLBACK<Type>::Type> >))

#define CLI_CALLBACK_SIGNATURE_ASSERT_TPL(CALLBACK, FUNCTOR)            \
    BOOST_MPL_ASSERT((boost::is_convertible<FUNCTOR,                    \
        boost::function<typename CALLBACK<Type>::Type> >))

namespace cli { namespace callback
{
    //
    // DoCommandCallback
    //

    template <typename T>
    struct DoCommandCallback
    {
        typedef bool (Type)(const std::string&,
            typename T::ArgumentsType const&);
        static const char* name() { return "DoCommandCallback"; }
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

    //
    // PreDoCommandCallback
    //

    template <typename T>
    struct PreDoCommandCallback
    {
        typedef void (Type)(std::string&);
        static const char* name() { return "PreDoCommandCallback"; }
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

    //
    // ParseErrorCallback
    //

    template <typename T>
    struct ParseErrorCallback
    {
            typedef bool (Type)(typename T::ParseErrorType const&,
                const std::string&);
            static const char* name() { return "ParseErrorCallback"; }
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

    //
    // PostLoopCallback
    //

    template <typename T>
    struct PostLoopCallback
    {
        typedef void (Type)();
        static const char* name() { return "PostLoopCallback"; }
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

    //
    // PathnameExpansionCallback
    //

    template <typename T>
    struct PathnameExpansionCallback
    {
        typedef std::vector<std::string> (Type)(const std::string&);
        static const char* name() { return "PathnameExpansionCallback"; }
    };
}}

#endif /* CALLBACKS_HPP_ */
