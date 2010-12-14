/*
 * exception.hpp - Framework exceptions
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

#ifndef EXCEPTION_HPP_
#define EXCEPTION_HPP_

#include <exception>

namespace cli { namespace exception
{
    //
    // Class UnknownCallbackException
    //

    struct UnknownCallbackException : public std::exception
    {

        UnknownCallbackException(const std::string& callbackName)
            : exception()
        {
            callbackName_ += "Unknown callback function name: ";
            callbackName_ += callbackName;
        }

        ~UnknownCallbackException() throw() {}

        virtual const char* what() const throw()
        {
            return callbackName_.c_str();
        }

        private:
            std::string callbackName_;
    };

    //
    // Class IncompatibleSignatureException
    //

    struct IncompatibleSignatureException : public std::exception
    {
        IncompatibleSignatureException() : exception() {}
        ~IncompatibleSignatureException() throw() {}

        virtual const char* what() const throw()
        {
            return "Incompatible callback function signature";
        }
    };
}}

#endif /* EXCEPTION_HPP_ */
