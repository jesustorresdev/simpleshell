/*
 * shell_parser.hpp - Parser designed to emulate a very simple shell
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

#ifndef SHELL_PARSER_HPP_
#define SHELL_PARSER_HPP_

#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/spirit/include/qi.hpp>

#include <cli/auxiliary.hpp>
#include <cli/basic_spirit.hpp>
#include <cli/callbacks.hpp>
#include <cli/glob.hpp>

namespace cli { namespace parser { namespace shellparser
{
    namespace qi = boost::spirit::qi;
    namespace iso8859_1 = boost::spirit::iso8859_1;
    namespace fusion = boost::fusion;

    class ShellInterpreter;

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
            INPUT,              // command < filename
            TRUNCATED_OUTPUT,   // command > filename
            APPENDED_OUTPUT     // command >> filename
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

        std::vector<VariableAssignment> variables;
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
        return out << "{variables: "    << arguments.variables
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
    // The parser must return a two-references Sequence. They have to refer to
    // the name and the arguments of parsed command respectively.
    //
    // The parser uses ISO-8859 encoding to avoid problems with UTF-8 strings
    // because the Boost.Spirit support of ASCII encoding launches an
    // exceptions when finds 8-bit characters.
    //

    template <typename Iterator>
    struct ShellParser
        : qi::grammar<Iterator,
              fusion::vector<std::string&, CommandArguments&>(),
              iso8859_1::space_type>
    {
        ShellParser(ShellInterpreter& interpreter);

        //
        // Parser rules
        //

        struct Redirections
            : qi::symbols<char, StdioRedirection::TypeOfRedirection>
        {
            Redirections()
            {
                add
                    ("<",  StdioRedirection::INPUT)
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

        qi::rule<Iterator> eol;
        qi::rule<Iterator> neol;
        qi::rule<Iterator, char()> character;
        qi::rule<Iterator, char()> dereference;
        qi::rule<Iterator, char()> special;
        qi::rule<Iterator, char()> escape;
        qi::rule<Iterator, std::string()> name;
        qi::rule<Iterator, std::string(), qi::locals<bool> > variable;
        qi::rule<Iterator, std::string()> quotedString;
        qi::rule<Iterator, std::string()> doubleQuotedString;
        qi::rule<Iterator, std::string()> word;
        qi::rule<Iterator, std::vector<std::string>()> expandedWord;
        qi::rule<Iterator, std::string()> variableValue;
        qi::rule<Iterator, void(bool)> unambiguousRedirection;
        qi::rule<Iterator, std::string(),
            qi::locals<int> > redirectionArgument;
        qi::rule<Iterator, VariableAssignment()> assignment;
        qi::rule<Iterator, StdioRedirection(),
            iso8859_1::space_type> redirection;
        qi::rule<Iterator, CommandArguments(), iso8859_1::space_type> command;
        qi::rule<Iterator, fusion::vector<std::string&, CommandArguments&>(),
            iso8859_1::space_type> start;

        private:
            ShellInterpreter& interpreter_;

            //
            // Auxiliary methods
            //

            static std::string globEscape(const std::string& pattern)
                { return glob::Glob::escape(pattern); }

            static std::string stringsJoin(const std::vector<std::string>& v)
                { return boost::algorithm::join(v, std::string(1, ' ')); }
    };

    //
    // Class ShellInterpreter
    //
    // Interpreter which uses ShellParser to parse the command line.
    //

    class ShellInterpreter
        : public BasicSpiritInterpreter<CommandArguments, ShellParser>
    {
        public:
            typedef BasicSpiritInterpreter<CommandArguments, ShellParser>
                BaseType;

            ShellInterpreter(bool useReadline = true);
            ShellInterpreter(std::istream& in, std::ostream& out,
                std::ostream& err, bool useReadline = true);

        protected:

            //
            // Hook methods
            //

            std::string variableLookup(const std::string& name);
            std::vector<std::string> pathnameExpansion(
                const std::string& pattern);

        private:

            CLI_DECLARE_CALLBACKS(
                ShellInterpreter,
                (VariableLookupCallback, variableLookupCallback_)
                (PathnameExpansionCallback, pathnameExpansionCallback_)
            )
    };
}}}

namespace cli
{
    typedef parser::shellparser::CommandArguments ShellInterpreterArguments;
    typedef parser::shellparser::ShellInterpreter ShellInterpreter;
}

#endif /* SHELL_PARSER_HPP_ */
