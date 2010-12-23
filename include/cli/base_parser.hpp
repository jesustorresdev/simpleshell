/*
 * base_parser.hpp - Base class for command-line parsers. It is required
 *                   by the setCallback() dispatching.
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

#ifndef BASE_PARSER_HPP_
#define BASE_PARSER_HPP_

#include <cli/exception.hpp>

namespace cli { namespace parser
{
    //
    // Class BaseParser
    //

    struct BaseParser
    {
        template <typename Signature>
        void setCallback(const std::string& callbackName,
            const boost::function<Signature>& function);
    };

    template <typename Signature>
    void BaseParser::setCallback(const std::string& callbackName,
        const boost::function<Signature>& function)
    {
        throw cli::exception::UnknownCallbackException(callbackName);
    }
}}

#endif /* BASE_PARSER_HPP_ */
