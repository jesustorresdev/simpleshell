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

#include <libintl.h>    // TODO: To use Boost.Locale when available
#define translate(str) ::gettext(str)

//#define BOOST_SPIRIT_DEBUG

#include <boost/function.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/phoenix/bind/bind_member_function.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/phoenix/operator/comparison.hpp>
#include <boost/spirit/home/phoenix/operator/if_else.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
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

        struct VariableAssignment
        {
            std::string name;
            std::string value;
        };

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

        VariableAssignment variable;
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
        return out << "{variable: "     << command.variable
                  << ", arguments: "    << command.arguments
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

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& out,
        const Command::VariableAssignment& variable)
    {
        return out << "{name: "  << variable.name
                  << ", value: " << variable.value << '}';
    }
}}

//
// Adaptors from Command classes to Boost.Fusion sequences. They are required
// by the parser SimpleShellParser. Must be defined at global scope.
//

BOOST_FUSION_ADAPT_STRUCT(
    cli::parser::Command::VariableAssignment,
    (std::string, name)
    (std::string, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    cli::parser::Command::StdioRedirection,
    (cli::parser::Command::StdioRedirection::TypeOfRedirection, type)
    (std::string, argument)
)

BOOST_FUSION_ADAPT_STRUCT(
    cli::parser::Command,
    (cli::parser::Command::VariableAssignment, variable)
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
            using qi::_2;
            using qi::_3;
            using qi::_4;
            using qi::_a;
            using qi::_val;
            using qi::_r1;
            using qi::eps;
            using qi::eoi;
            using qi::fail;
            using qi::lexeme;
            using qi::lit;
            using qi::on_error;
            using ascii::char_;
            using ascii::space;
            using phoenix::at_c;
            using phoenix::begin;
            using phoenix::bind;
            using phoenix::construct;
            using phoenix::end;
            using phoenix::if_else;
            using phoenix::insert;
            using phoenix::push_back;
            using phoenix::val;

            eol = eoi;
            character %= char_;
            dereference = '$';
            special %= dereference | redirectors | terminators;
            escape %= '\\' > character;

            name %= char_("a-zA-Z") >> *char_("a-zA-Z0-9");
            variable =
                eps[_a = false] >>
                dereference >> (
                    -lit('{')[_a = true] >
                    name[bind(&Type::variableLookup, *this, _val, _1)]
                ) >> ((eps(_a) > '}') | eps(!_a));

            quotedString %= '\'' >> *(char_ - '\'') > '\'';
            doubleQuotedString = '"' >> *(
                variable    [insert(_val, end(_val), begin(_1), end(_1))] |
                (
                    char_('\'')             [push_back(_val, _1)] >>
                    *((char_ - '\'' - '"')  [push_back(_val, _1)]) >>
                    char_('\'')             [push_back(_val, _1)]
                ) |
                (char_ - '"')               [push_back(_val, _1)]
            ) > '"';

            word = +(
                variable            [insert(_val, end(_val), begin(_1), end(_1))] |
                quotedString        [insert(_val, end(_val), begin(_1), end(_1))] |
                doubleQuotedString  [insert(_val, end(_val), begin(_1), end(_1))] |
                escape                      [push_back(_val, _1)] |
                (char_ - space - special)   [push_back(_val, _1)]
            );

            assignment %= name >> '=' >> -word;
            redirection %= redirectors > word;
            ending %= terminators;

            // This statement doesn't work as expected in boost 1.45.0
            // command %= assignment || +word || +redirection;
            command = (
                assignment      [at_c<0>(_val) = _1] ||
                (+word)         [at_c<1>(_val) = _1] ||
                (+redirection)  [at_c<2>(_val) = _1]
            ) >> (
                ending          [at_c<3>(_val) = _1, _r1 = true ] |
                eps                                 [_r1 = false]
            );
            expressions = +(
                command(_a)[push_back(_val, _1)] >>
                ((eps(!_a) > eol) | eps(_a))
            ) > eol;

            // start rule with locals doesn't compile in boost 1.44.0
            start %= expressions;

            character.name(translate("character"));
            name.name(translate("name"));
            word.name(translate("word"));
            eol.name(translate("end-of-line"));

            on_error<fail>(
                start,
                std::cerr
                    << val(translate("parse error, expecting"))
                    << val(" ")
                    << _4
                    << val(" ")
                    << val(translate("at"))
                    << val(": ")
                    << if_else(_3 == _2, val(translate("<end-of-line>")),
                        construct<std::string>(_3, _2))
                    << std::endl
            );

//            BOOST_SPIRIT_DEBUG_NODE(name);
//            BOOST_SPIRIT_DEBUG_NODE(variable);
//            BOOST_SPIRIT_DEBUG_NODE(quotedString);
//            BOOST_SPIRIT_DEBUG_NODE(doubleQuotedString);
//            BOOST_SPIRIT_DEBUG_NODE(word);
//            BOOST_SPIRIT_DEBUG_NODE(assignment);
//            BOOST_SPIRIT_DEBUG_NODE(redirection);
//            BOOST_SPIRIT_DEBUG_NODE(ending);
//            BOOST_SPIRIT_DEBUG_NODE(command);
            BOOST_SPIRIT_DEBUG_NODE(expressions);
        }

        //
        // Parser rules
        //

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

        qi::rule<Iterator> eol;
        qi::rule<Iterator, char()> character;
        qi::rule<Iterator, char()> dereference;
        qi::rule<Iterator, char()> special;
        qi::rule<Iterator, char()> escape;
        qi::rule<Iterator, std::string()> name;
        qi::rule<Iterator, std::string(), qi::locals<bool> > variable;
        qi::rule<Iterator, std::string()> quotedString;
        qi::rule<Iterator, std::string()> doubleQuotedString;
        qi::rule<Iterator, std::string()> word;
        qi::rule<Iterator, Command::VariableAssignment()> assignment;
        qi::rule<Iterator, Command::StdioRedirection(), ascii::space_type> redirection;
        qi::rule<Iterator, Command::TypeOfTerminator(), ascii::space_type> ending;
        qi::rule<Iterator, Command(bool&), ascii::space_type> command;
        qi::rule<Iterator, std::vector<Command>(), qi::locals<bool>, ascii::space_type> expressions;
        qi::rule<Iterator, std::vector<Command>(), ascii::space_type> start;

        //
        // Callback functions
        //

        typedef SimpleShellParser<Iterator> Type;
        typedef typename callbacks::VariableLookupCallback<Type>::Type
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
