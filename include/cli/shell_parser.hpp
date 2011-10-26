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

#include <boost/algorithm/string/join.hpp>
#include <boost/spirit/include/qi.hpp>

#include <cli/auxiliary.hpp>
#include <cli/boost_parser_base.hpp>
#include <cli/callbacks.hpp>
#include <cli/glob.hpp>

namespace cli { namespace parser { namespace shellparser
{
    namespace qi = boost::spirit::qi;
    namespace iso8859_1 = boost::spirit::iso8859_1;

    //
    // Class CommandArguments
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

    struct CommandArguments
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

        CommandArguments() : terminator(NORMAL) {}

        std::string getCommandName() const
            { return arguments.empty() ? std::string() : arguments[0]; }
    };

    //
    // Overload insertion operator (<<) for class CommandArguments.
    // It is required to debug the parser rules.
    //

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& out,
        const CommandArguments& arguments)
    {
        return out << "{variable: "     << arguments.variable
                  << ", arguments: "    << arguments.arguments
                  << ", redirections: " << arguments.redirections
                  << ", terminator: "   << arguments.terminator << '}';
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

    //
    // Class ShellParser
    //
    // The parser uses ISO-8859 encoding to avoid problems with UTF-8 strings
    // because the Boost.Spirit support of ASCII encoding launches an
    // exceptions when finds 8-bit characters.
    //

    struct ShellParser
        : BoostParserBase<std::string, CommandArguments, iso8859_1::space_type>
    {
        ShellParser();

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
            : qi::symbols<char, CommandArguments::TypeOfTerminator>
        {
            Terminators()
            {
                add
                    (";", CommandArguments::NORMAL)
                    ("&", CommandArguments::BACKGROUNDED)
                ;
            }
        } terminators;

        struct Pipe
            : qi::symbols<char, CommandArguments::TypeOfTerminator>
        {
            Pipe()
            {
                add
                    ("|", CommandArguments::PIPED)
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
        qi::rule<IteratorType, CommandArguments(),
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
}}}

namespace cli { namespace parser
{
    using shellparser::ShellParser;
}}

#endif /* SHELL_PARSER_HPP_ */
