/*
 * shell_parser.hpp - Parser designed to emulate a very simple shell
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

#ifndef SHELL_PARSER_HPP_
#define SHELL_PARSER_HPP_

#include <iostream>
#include <string>
#include <vector>

//#define BOOST_SPIRIT_DEBUG

#include <boost/algorithm/string/join.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_container.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
#include <boost/spirit/include/qi.hpp>

#define translate(str) str  // TODO: Use Boost.Locale when available

#include <cli/auxiliary.hpp>
#include <cli/boost_parser_base.hpp>
#include <cli/callbacks.hpp>
#include <cli/glob.hpp>

namespace cli { namespace parser { namespace shellparser
{
    //
    // Class CommandDetails
    //
    // Stores the information provided by the parser ShellParser about the
    // command specified.
    //

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

    struct CommandDetails
    {
        enum TypeOfTerminator
        {
            NORMAL,             // command ;
            BACKGROUNDED,       // command &
            PIPED               // command1 | command2
        };

        VariableAssignment variable;
        std::vector<std::string> arguments;
        std::vector<StdioRedirection> redirections;
        TypeOfTerminator terminator;

        CommandDetails() : terminator(NORMAL) {}

        std::string getCommandName() const
            { return arguments.empty() ? std::string() : arguments[0]; }
    };

    //
    // Overload insertion operator (<<) for class CommandDetails.
    // It is required to debug the parser rules.
    //

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& out,
        const CommandDetails& details)
    {
        return out << "{variable: "     << details.variable
                  << ", arguments: "    << details.arguments
                  << ", redirections: " << details.redirections
                  << ", terminator: "   << details.terminator << '}';
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& out,
        const StdioRedirection& redirection)
    {
        return out << "{type: "     << redirection.type
                  << ", argument: " << redirection.argument << '}';
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& out,
        const VariableAssignment& variable)
    {
        return out << "{name: "  << variable.name
                  << ", value: " << variable.value << '}';
    }
}}}

//
// Adaptors from CommandDetails classes to Boost.Fusion sequences. They are
// required by the parser ShellParser. Must be defined at global scope.
//

BOOST_FUSION_ADAPT_STRUCT(
    cli::parser::shellparser::VariableAssignment,
    (std::string, name)
    (std::string, value)
)

BOOST_FUSION_ADAPT_STRUCT(
    cli::parser::shellparser::StdioRedirection,
    (cli::parser::shellparser::StdioRedirection::TypeOfRedirection, type)
    (std::string, argument)
)

BOOST_FUSION_ADAPT_STRUCT(
    cli::parser::shellparser::CommandDetails,
    (cli::parser::shellparser::VariableAssignment, variable)
    (std::vector<std::string>, arguments)
    (std::vector<cli::parser::shellparser::StdioRedirection>, redirections)
    (cli::parser::shellparser::CommandDetails::TypeOfTerminator, terminator)
)

namespace cli { namespace parser { namespace shellparser
{
    namespace qi = boost::spirit::qi;
    namespace iso8859_1 = boost::spirit::iso8859_1;
    namespace phoenix = boost::phoenix;

    //
    // Class ShellParser
    //
    // The parser uses ISO-8859 encoding to avoid problems with UTF-8 strings
    // because the Boost.Spirit support of ASCII encoding launches an
    // exceptions when finds 8-bit characters.
    //

    struct ShellParser
        : BoostParserBase<std::string, CommandDetails, iso8859_1::space_type>
    {
        ShellParser() : ShellParser::base_type(start)
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
            using qi::raw;
            using iso8859_1::char_;
            using iso8859_1::space;
            using phoenix::at;
            using phoenix::at_c;
            using phoenix::begin;
            using phoenix::bind;
            using phoenix::empty;
            using phoenix::end;
            using phoenix::insert;
            using phoenix::push_back;
            using phoenix::size;

            eol = eoi;
            neol = !eoi;
            character %= char_;
            dereference = '$';
            special %= dereference | redirectors | terminators | pipe;
            escape %= '\\' > character;

            name %= char_("a-zA-Z") >> *char_("a-zA-Z0-9");
            variable =
                eps[_a = false] >>
                dereference >> (
                    -lit('{')[_a = true] >
                    name[_val = bind(&ShellParser::variableLookup, *this, _1)]
                ) >> ((eps(_a) > '}') | eps(!_a));

            quotedString %= '\'' >> *(char_ - '\'') > '\'';
            doubleQuotedString = '"' >> *(
                variable                    [_val += _1] |
                (
                    char_('\'')             [push_back(_val, _1)]  >>
                    *((char_ - '\'' - '"')  [push_back(_val, _1)]) >>
                    char_('\'')             [push_back(_val, _1)]
                ) |
                (char_ - '"')               [push_back(_val, _1)]
            ) > '"';

            word = +(
                variable                    [_val += _1]          |
                quotedString
                    [_val += bind(&ShellParser::globEscape, _1)]  |
                doubleQuotedString
                    [_val += bind(&ShellParser::globEscape, _1)]  |
                escape                      [push_back(_val, _1)] |
                (char_ - space - special)   [push_back(_val, _1)]
            );

            expandedWord = word
                [_val = bind(&ShellParser::pathnameExpansion, *this, _1)];

            variableValue = expandedWord
                [_val = bind(&ShellParser::stringsJoin, _1)];

            unambiguousRedirection = eps(_r1);
            redirectionArgument = (
                (
                    &expandedWord[_a = size(_1)] >
                    unambiguousRedirection(_a == 1)

                ) >> expandedWord[_val = at(_1, 0)]
            ) | (eps > expandedWord);

            assignment %= name >> '=' >> -variableValue;
            redirection %= redirectors >> redirectionArgument;

            command = (
                assignment      [at_c<0>(_val) = _1] ||
                (
                    +expandedWord
                    [insert(at_c<1>(_val), end(at_c<1>(_val)),
                        begin(_1), end(_1))]
                ) ||
                (+redirection)  [at_c<2>(_val) = _1]
            ) >> (
                (terminators    [at_c<3>(_val) = _1] >> -eol) |
                (pipe           [at_c<3>(_val) = _1] >  neol) |
                (eps > eol)
            );
            start = command [
                 at_c<1>(_val) = _1,
                 at_c<0>(_val) = bind(&CommandDetails::getCommandName, _1)
            ];

            character.name(translate("character"));
            name.name(translate("name"));
            expandedWord.name(translate("word"));
            variableValue.name(translate("word"));
            unambiguousRedirection.name(translate("unambiguous redirection"));
            eol.name(translate("end-of-line"));
            neol.name(translate("more characters"));

            on_error<fail>(
                start, bind(&ShellParser::throwParserError, _1, _2, _3, _4)
            );

//            BOOST_SPIRIT_DEBUG_NODE(name);
//            BOOST_SPIRIT_DEBUG_NODE(variable);
//            BOOST_SPIRIT_DEBUG_NODE(quotedString);
//            BOOST_SPIRIT_DEBUG_NODE(doubleQuotedString);
//            BOOST_SPIRIT_DEBUG_NODE(expandedWord);
//            BOOST_SPIRIT_DEBUG_NODE(word);
//            BOOST_SPIRIT_DEBUG_NODE(assignment);
//            BOOST_SPIRIT_DEBUG_NODE(redirection);
//            BOOST_SPIRIT_DEBUG_NODE(ending);
//            BOOST_SPIRIT_DEBUG_NODE(command);
            BOOST_SPIRIT_DEBUG_NODE(start);
        }

        //
        // Parser rules
        //

        struct Redirections
            : qi::symbols<char, StdioRedirection::TypeOfRedirection>
        {
            Redirections()
            {
                add
                    ("<",  StdioRedirection::TRUNCATED_INPUT)
                    (">",  StdioRedirection::TRUNCATED_OUTPUT)
                    (">>", StdioRedirection::APPENDED_OUTPUT)
                ;
            }
        } redirectors;

        struct Terminators
            : qi::symbols<char, CommandDetails::TypeOfTerminator>
        {
            Terminators()
            {
                add
                    (";", CommandDetails::NORMAL)
                    ("&", CommandDetails::BACKGROUNDED)
                ;
            }
        } terminators;

        struct Pipe
            : qi::symbols<char, CommandDetails::TypeOfTerminator>
        {
            Pipe()
            {
                add
                    ("|", CommandDetails::PIPED)
                ;
            }
        } pipe;

        qi::rule<IteratorType> eol;
        qi::rule<IteratorType> neol;
        qi::rule<IteratorType, char()> character;
        qi::rule<IteratorType, char()> dereference;
        qi::rule<IteratorType, char()> special;
        qi::rule<IteratorType, char()> escape;
        qi::rule<IteratorType, std::string()> name;
        qi::rule<IteratorType, std::string(), qi::locals<bool> > variable;
        qi::rule<IteratorType, std::string()> quotedString;
        qi::rule<IteratorType, std::string()> doubleQuotedString;
        qi::rule<IteratorType, std::string()> word;
        qi::rule<IteratorType, std::vector<std::string>()> expandedWord;
        qi::rule<IteratorType, std::string()> variableValue;
        qi::rule<IteratorType, void(bool)> unambiguousRedirection;
        qi::rule<IteratorType, std::string(),
            qi::locals<int> > redirectionArgument;
        qi::rule<IteratorType, VariableAssignment()> assignment;
        qi::rule<IteratorType, StdioRedirection(),
            iso8859_1::space_type> redirection;
        qi::rule<IteratorType, CommandDetails(),
            iso8859_1::space_type> command;
        qi::rule<IteratorType, ShellParser::sig_type,
            iso8859_1::space_type> start;

        protected:

            //
            // Hook methods
            //

            std::string variableLookup(const std::string& name);
            std::vector<std::string> pathnameExpansion(
                const std::string& pattern);

        private:

            CLI_DECLARE_CALLBACKS(
                ShellParser,
                (VariableLookupCallback, variableLookupCallback_)
                (PathnameExpansionCallback, pathnameExpansionCallback_)
            )

            //
            // Auxiliary methods
            //

            static std::string globEscape(const std::string& pattern)
                { return glob::Glob::escape(pattern); }

            static std::string stringsJoin(const std::vector<std::string>& v)
                { return boost::algorithm::join(v, std::string(1, ' ')); }

            static void throwParserError(IteratorType const& first,
                IteratorType const& last, IteratorType const& error,
                const boost::spirit::info& info);
    };

    std::string ShellParser::variableLookup(const std::string& name)
    {
        return variableLookupCallback_.empty() ?
            std::string() : variableLookupCallback_(name);
    }

    std::vector<std::string> ShellParser::pathnameExpansion(
        const std::string& pattern)
    {
        if (! pathnameExpansionCallback_.empty()) {
            return pathnameExpansionCallback_(pattern);
        }

        using namespace glob;

        Glob glob(pattern, Glob::EXPAND_BRACE_EXPRESSIONS |
            Glob::NO_PATH_NAMES_CHECK | Glob::EXPAND_TILDE_WITH_CHECK);

        Glob::ErrorsType errors = glob.getErrors();
        for (Glob::ErrorsType::const_iterator iter = errors.begin();
            iter < errors.end(); ++iter)
        {
            std::cerr
                << ::program_invocation_short_name
                << ": "
                << translate("i/o error at")
                << " "
                << iter->first
                << ": "
                << iter->second.message();
        }

        return glob;
    }

    void ShellParser::throwParserError(IteratorType const& first,
        IteratorType const& last, IteratorType const& error,
        const boost::spirit::info& info)
    {
        std::string what;
        what += translate("syntax error, expecting");
        what += " " + info.tag + " " + translate("at") + ": ";
        what += (error == last) ? translate("<end-of-line>")
            : std::string(error, last);

        base_type::throwParserError(what, first, last, error, info);
    }
}}}

namespace cli { namespace parser
{
    using shellparser::ShellParser;
}}

#endif /* SHELL_PARSER_HPP_ */
