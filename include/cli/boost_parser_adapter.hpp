/*
 * boost_parser_adapter.hpp - Adapter for parsers based on Boost.Spirit
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

#ifndef BOOST_PARSER_ADAPTER_HPP_
#define BOOST_PARSER_ADAPTER_HPP_

#include <stdexcept>

#include <boost/spirit/include/qi.hpp>

#define translate(str) str  // TODO: Use Boost.Locale when available

namespace cli { namespace parser
{
    namespace qi = boost::spirit::qi;

    //
    // Class BoostParserError
    //
    // Type used to return parser errors to the interpreter.
    //

    template <typename Iterator>
    struct BoostParserError : public std::runtime_error
    {
        //
        // This attributes are only available if containsDetails() returns
        // true. For a description of them, see qi::on_error (error handling)
        // in Boost.Spirit documentation.
        //

        Iterator first;
        Iterator last;
        Iterator error;
        boost::spirit::info what_;

        //
        // Class constructors
        //

        BoostParserError(bool fail = false)
            : runtime_error(""), what_(""), parserFail_(fail),
              containsDetails_(false)
        {}

        BoostParserError(const char* what)
            : runtime_error(std::string(what)), what_(""), parserFail_(true),
              containsDetails_(false)
        {}

        BoostParserError(const std::string& what)
            : runtime_error(what), what_(""), parserFail_(true),
              containsDetails_(false)
        {}

        BoostParserError(const std::string& what, const Iterator& first,
            const Iterator& last, const Iterator& error,
            const boost::spirit::info& info)
            : runtime_error(what), first(first), last(last), error(error),
              what_(info), parserFail_(true), containsDetails_(true)
        {}

        virtual ~BoostParserError() throw() {}

        //
        // The interpreter requires the type can be convertible to bool
        //

        operator bool()
            { return parserFail_; }

        bool containsDetails()
            { return containsDetails_; }

        private:
            bool parserFail_;
            bool containsDetails_;
    };

    //
    // Class BoostParserAdapter
    //
    // Wrapper class for parsers based on Boost.Spirit.
    //

    template <typename Iterator, typename Arguments,
        template <typename> class Parser>
    struct BoostParserAdapter
    {
        typedef BoostParserAdapter<Iterator, Arguments, Parser> Type;
        typedef Parser<Iterator> ParserType;

        //
        // Define the members required to model STL function object concepts.
        //

        typedef BoostParserError<Iterator> result_type;
        typedef Iterator& arg1_type;
        typedef Iterator arg2_type;
        typedef std::string& arg3_type;
        typedef Arguments& arg4_type;

        BoostParserError<Iterator> operator()(Iterator& begin, Iterator end,
            std::string& command, Arguments& arguments)
            { return parse(begin, end, command, arguments); }

        protected:
            ParserType parser_;
            typename ParserType::skipper_type skipper_parser_;

	        virtual BoostParserError<Iterator> parse(Iterator& begin,
	            Iterator end, std::string& command, Arguments& arguments)
            {
	            // Passing the attributes 'command' and 'arguments' to the
    	        // parser forces that every valid grammar must return a
	            // two-references Sequence.
	            try {
	                bool success = qi::phrase_parse(begin, end, parser_,
	                    skipper_parser_, command, arguments);
	                if (success) {
	                    return BoostParserError<Iterator>();
	                }
	                else {
	                    return BoostParserError<Iterator>(
	                        translate("syntax error"));
	                }
	            }
	            catch (BoostParserError<Iterator> result) {
	                return result;
	            }
	        }
    };
}}

#endif /* BOOST_PARSER_ADATER_HPP_ */
