/*
 * simple_shell_parser.hpp - Parser designed to make simple shells
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

#ifndef SIMPLE_SHELL_PARSER_HPP_
#define SIMPLE_SHELL_PARSER_HPP_

#include <string>
#include <vector>

//#define BOOST_SPIRIT_DEBUG

#include <boost/function.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/phoenix/bind/bind_member_function.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/qi.hpp>

#include <cli/callbacks.hpp>

namespace cli { namespace parser
{
    //
    // Class Command
    //
    // Stores the information provided by the parser SimpleShellParser
    // that is required for command execution.
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
// Adaptors from Command classes to Boost.Fusion sequences. They are required
// by the parser SimpleShellParser. Must be defined at global scope.
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

    template <typename Iterator>
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
            using phoenix::begin;
            using phoenix::bind;
            using phoenix::end;
            using phoenix::insert;
            using phoenix::push_back;

            escape %= '\\' >> char_;
            name %= char_("a-zA-Z") >> *char_("a-zA-Z0-9");
            variableA %= '$' >>
                name [bind(&ClassType::variableLookup, *this, _val, _1)];
            variableB %= "${" >>
                name [bind(&ClassType::variableLookup, *this, _val, _1)] >>
                '}';
            variable %= variableA | variableB;

            quotedString %= lexeme['\'' >> *(char_ - '\'') >> '\''];
            doubleQuotedString = lexeme['"' >> *(
                variable    [insert(_val, end(_val), begin(_1), end(_1))] |
                (
                    char_('\'')             [push_back(_val, _1)] >>
                    *((char_ - '\'' - '"')  [push_back(_val, _1)]) >>
                    char_('\'')             [push_back(_val, _1)]
                ) |
                (char_ - '"')               [push_back(_val, _1)]
            ) >> '"'];
            word = lexeme[+(
                variable            [insert(_val, end(_val), begin(_1), end(_1))] |
                quotedString        [insert(_val, end(_val), begin(_1), end(_1))] |
                doubleQuotedString  [insert(_val, end(_val), begin(_1), end(_1))] |
                escape              [push_back(_val, _1)] |
                (char_ - space)     [push_back(_val, _1)]
            )];

            redirection %= redirectors >> word;
            ending %= terminators;
            command %= +(word - redirection) >> *redirection >> ending;
            lastCommand %= +(word - redirection) >> *redirection;
            start = +command    [push_back(_val, _1)] ||
                    lastCommand [push_back(_val, _1)];

//            BOOST_SPIRIT_DEBUG_NODE(variable);
//            BOOST_SPIRIT_DEBUG_NODE(quotedString);
//            BOOST_SPIRIT_DEBUG_NODE(doubleQuotedString);
//            BOOST_SPIRIT_DEBUG_NODE(word);
//            BOOST_SPIRIT_DEBUG_NODE(redirection);
//            BOOST_SPIRIT_DEBUG_NODE(end);
//            BOOST_SPIRIT_DEBUG_NODE(command);
//            BOOST_SPIRIT_DEBUG_NODE(lastCommand);
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
        qi::rule<Iterator, std::string()> name;
        qi::rule<Iterator, std::string()> variableA;
        qi::rule<Iterator, std::string()> variableB;
        qi::rule<Iterator, std::string()> variable;
        qi::rule<Iterator, std::string()> quotedString;
        qi::rule<Iterator, std::string()> doubleQuotedString;
        qi::rule<Iterator, std::string(), ascii::space_type> word;
        qi::rule<Iterator, std::string(), ascii::space_type> argument;
        qi::rule<Iterator, Command::StdioRedirection(), ascii::space_type> redirection;
        qi::rule<Iterator, Command::TypeOfTerminator(), ascii::space_type> ending;
        qi::rule<Iterator, Command(), ascii::space_type> command;
        qi::rule<Iterator, Command(), ascii::space_type> lastCommand;
        qi::rule<Iterator, std::vector<Command>(), ascii::space_type> start;

        typedef SimpleShellParser<Iterator> ClassType;
        typedef typename callbacks::VariableLookupCallback<ClassType>::Type
            VariableLookupCallback;

        protected:
            void variableLookup(std::string &value, const std::string& name);

        private:
            boost::function<VariableLookupCallback> variableLookupCallback_;
    };

    template <typename Iterator>
    void SimpleShellParser<Iterator>::variableLookup(std::string& value,
        const std::string& name)
    {
        variableLookupCallback_.empty() ?
            value.clear() : variableLookupCallback_(value, name);
    }
}}

#endif /* SIMPLE_SHELL_PARSER_HPP_ */
