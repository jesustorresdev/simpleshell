/*
 * parser.hpp - Example parsers for the command line interpreter framework
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

#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <string>
#include <vector>

//#define BOOST_SPIRIT_DEBUG

#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/qi.hpp>

#include <cli/exception.hpp>
#include <cli/standard.hpp>

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

namespace cli { namespace parsers
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    //
    // Class SimpleParser
    //

    template<typename Iterator>
    struct SimpleParser
        : qi::grammar<Iterator, std::vector<std::string>(), ascii::space_type>
    {
        SimpleParser() : SimpleParser::base_type(start)
        {
            using qi::lexeme;
            using ascii::char_;
            using ascii::space;

            // TODO: Non-ascii characters support
            escape %= lexeme['\\' >> char_];
            word %= lexeme[+(escape | (char_ - space))];
            quoted_string %= lexeme['\'' >> *(char_ - '\'') >> '\''];
            double_quoted_string %= lexeme['"' >> *(char_ - '"') >> '"'];
            argument %= quoted_string | double_quoted_string | word;
            start = +argument;

//            BOOST_SPIRIT_DEBUG_NODE(word);
//            BOOST_SPIRIT_DEBUG_NODE(quoted_string);
//            BOOST_SPIRIT_DEBUG_NODE(double_quoted_string);
//            BOOST_SPIRIT_DEBUG_NODE(argument);
            BOOST_SPIRIT_DEBUG_NODE(start);
        }

        qi::rule<Iterator, char()> escape;
        qi::rule<Iterator, std::string(), ascii::space_type> word;
        qi::rule<Iterator, std::string(), ascii::space_type> quoted_string;
        qi::rule<Iterator, std::string(), ascii::space_type> double_quoted_string;
        qi::rule<Iterator, std::string(), ascii::space_type> argument;
        qi::rule<Iterator, std::vector<std::string>(), ascii::space_type> start;
    };
}}

namespace cli { namespace parser
{
    //
    // Struct Command
    //
    // Stores the information required for command execution that is provided
    // by the parser SimpleShellParser.
    //

    struct Command
    {
        Command() : terminator(NORMAL){}

        struct StdioRedirection
        {
            enum TypeOfRedirection
            {
                TRUNCATED_INPUT,    // command <filename
                TRUNCATED_OUTPUT,   // command >filename
                APPENDED_OUTPUT     // command >>filename
            };

            TypeOfRedirection type;
            std::string argument;
        };

        enum TypeOfTerminator
        {
            NORMAL,                 // command ;
            BACKGROUNDED,           // command &
            PIPED                   // command1 | command2
        };

        std::vector<std::string> arguments;
        std::vector<StdioRedirection> redirections;
        TypeOfTerminator terminator;
    };

    //
    // Overload of insertion operator (<<) for struct Command
    // It is required to debug the parser rules
    //

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& out, const Command& command)
    {
        return out << "{arguments: "    << command.arguments
                  << ", redirections: " << command.redirections
                  << ", terminator: "   << command.terminator << '}';
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& out,
        const Command::StdioRedirection& redirection)
    {
        return out << "{type: "     << redirection.type
                  << ", argument: " << redirection.argument << '}';
    }
}}

//
// Adaptors from Command structs to the Boost.Fusion sequences required by
// the parser SimpleShellParser. Must be defined at global scope.
//

BOOST_FUSION_ADAPT_STRUCT(
    cli::parser::Command::StdioRedirection,
    (cli::parser::Command::StdioRedirection::TypeOfRedirection, type)
    (std::string, argument)
)

BOOST_FUSION_ADAPT_STRUCT(
    cli::parser::Command,
    (std::vector<std::string>, arguments)
    (std::vector<cli::parser::Command::StdioRedirection>, redirections)
    (cli::parser::Command::TypeOfTerminator, terminator)
)

namespace cli { namespace parser
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;
    namespace phoenix = boost::phoenix;

    //
    // Class SimpleShellParser
    //

    template<typename Iterator>
    struct SimpleShellParser
        : qi::grammar<Iterator, std::vector<Command>(), ascii::space_type>
    {
        SimpleShellParser() : SimpleShellParser::base_type(start)
        {
            using qi::_1;
            using qi::_val;
            using qi::lexeme;
            using ascii::char_;
            using ascii::space;
            using phoenix::push_back;

            // TODO: Environment variables substitution
            // TODO: Non-ascii characters support
            escape %= lexeme['\\' >> char_];
            word %= lexeme[+(escape
                | (char_ - space - redirectors - terminators))];
            quoted_string %= lexeme['\'' >> *(char_ - '\'') >> '\''];
            double_quoted_string %= lexeme['"' >> *(char_ - '"') >> '"'];
            argument %= quoted_string | double_quoted_string | word;
            redirection %= redirectors >> argument;
            end %= terminators;
            command %= +(argument - redirection) >> *redirection >> end;
            last_command %= +(argument - redirection) >> *redirection;
            start = +command    [push_back(_val, _1)]
                || last_command [push_back(_val, _1)];

//            BOOST_SPIRIT_DEBUG_NODE(word);
//            BOOST_SPIRIT_DEBUG_NODE(quoted_string);
//            BOOST_SPIRIT_DEBUG_NODE(double_quoted_string);
//            BOOST_SPIRIT_DEBUG_NODE(argument);
//            BOOST_SPIRIT_DEBUG_NODE(redirection);
//            BOOST_SPIRIT_DEBUG_NODE(end);
//            BOOST_SPIRIT_DEBUG_NODE(command);
//            BOOST_SPIRIT_DEBUG_NODE(last_command);
            BOOST_SPIRIT_DEBUG_NODE(start);
        }

        struct Redirections
            : qi::symbols<char, Command::StdioRedirection::TypeOfRedirection>
        {
            Redirections()
            {
                add
                    ("<",  Command::StdioRedirection::TRUNCATED_INPUT)
                    (">",  Command::StdioRedirection::TRUNCATED_OUTPUT)
                    (">>", Command::StdioRedirection::APPENDED_OUTPUT)
                ;
            }
        } redirectors;

        struct Terminators
            : qi::symbols<char, Command::TypeOfTerminator>
        {
            Terminators()
            {
                add
                    (";", Command::NORMAL)
                    ("&", Command::BACKGROUNDED)
                    ("|", Command::PIPED)
                ;
            }
        } terminators;

        qi::rule<Iterator, char()> escape;
        qi::rule<Iterator, std::string(), ascii::space_type> word;
        qi::rule<Iterator, std::string(), ascii::space_type> quoted_string;
        qi::rule<Iterator, std::string(), ascii::space_type> double_quoted_string;
        qi::rule<Iterator, std::string(), ascii::space_type> argument;
        qi::rule<Iterator, Command::StdioRedirection(), ascii::space_type> redirection;
        qi::rule<Iterator, Command::TypeOfTerminator(), ascii::space_type> end;
        qi::rule<Iterator, Command(), ascii::space_type> command;
        qi::rule<Iterator, Command(), ascii::space_type> last_command;
        qi::rule<Iterator, std::vector<Command>(), ascii::space_type> start;
    };
}}

#endif /* PARSER_HPP_ */
