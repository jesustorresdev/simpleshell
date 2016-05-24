/*
 * shell.hpp - Interpreter designed to emulate a very simple shell
 *
 *   Copyright 2010-2016 Jesús Torres <jmtorres@ull.es>
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

#ifndef SHELL_HPP_
#define SHELL_HPP_

#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string/join.hpp>
#include <boost/spirit/include/qi.hpp>

#include <cli/basic_spirit.hpp>
#include <cli/callbacks.hpp>
#include <cli/glob.hpp>
#include <cli/utility.hpp>

namespace cli
{
    class ShellInterpreter;
}

namespace cli { namespace parser { namespace shellparser
{
    namespace qi = boost::spirit::qi;
    namespace iso8859_1 = boost::spirit::iso8859_1;
    namespace fusion = boost::fusion;

    //
    // Class Arguments
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

    struct Arguments
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

        Arguments() : terminator(NORMAL) {}

        std::string getCommandName() const
            { return arguments.empty() ? std::string() : arguments[0]; }
    };

    //
    // Overload insertion operator (<<) for class Arguments.
    // It is required to debug the parser rules.
    //

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os,
        const Arguments& arguments)
    {
        using namespace cli::prettyprint;

        if (isPrettyprintEnabled(os)) {
            os << CHART_LITERAL(CharT, "{") << endlAndIndent;
            os << CHART_LITERAL(CharT, "variables: ")
                << arguments.variables << CHART_LITERAL(CharT, ",") << endl;
            os << CHART_LITERAL(CharT, "arguments: ")
                << arguments.arguments << CHART_LITERAL(CharT, ",") << endl;
            os << CHART_LITERAL(CharT, "redirections: ")
                << arguments.redirections << CHART_LITERAL(CharT, ",") << endl;
            os << CHART_LITERAL(CharT, "terminator: ")
                << arguments.terminator << endlAndDeindent;
        }
        else {
            os << CHART_LITERAL(CharT, "{");
            os << CHART_LITERAL(CharT, "variables: ")
                << arguments.variables << CHART_LITERAL(CharT, ", ");
            os << CHART_LITERAL(CharT, "arguments: ")
                << arguments.arguments << CHART_LITERAL(CharT, ", ");
            os << CHART_LITERAL(CharT, "redirections: ")
                << arguments.redirections << CHART_LITERAL(CharT, ", ");
            os << CHART_LITERAL(CharT, "terminator: ")
                << arguments.terminator;
        }

        return os << CHART_LITERAL(CharT, '}');
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os,
        Arguments::TypeOfTerminator type)
    {
        using namespace cli::prettyprint;

        if (isPrettyprintEnabled(os)) {
            switch (type) {
            case Arguments::NORMAL:
                os << "Arguments::NORMAL";
                break;
            case Arguments::BACKGROUNDED:
                os << "Arguments::BACKGROUNDED";
                break;
            case Arguments::PIPED:
                os << "Arguments::PIPED";
                break;
            default:
                os << "UNKNOWN" << '(' << static_cast<int>(type) << ')';
                break;
            }
        }
        else {
            os << static_cast<int>(type);
        }

        return os;
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os,
        const StdioRedirection& redirection)
    {
        using namespace cli::prettyprint;

        if (isPrettyprintEnabled(os)) {
            os << CHART_LITERAL(CharT, "(struct){")
                << prettyprint::endlAndIndent;
            os << CHART_LITERAL(CharT, "type: ") << redirection.type
                << CHART_LITERAL(CharT, ",") << endl;
            os << CHART_LITERAL(CharT, "argument: ") << redirection.argument
                << endlAndDeindent;
        }
        else {
            os << CHART_LITERAL(CharT, "{");
            os << CHART_LITERAL(CharT, "type: ") << redirection.type
                << CHART_LITERAL(CharT, ", ");
            os << CHART_LITERAL(CharT, "argument: ") << redirection.argument;
        }

        return os << CHART_LITERAL(CharT, '}');
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os,
        StdioRedirection::TypeOfRedirection type)
    {
        using namespace cli::prettyprint;

        if (isPrettyprintEnabled(os)) {
            switch (type) {
            case StdioRedirection::INPUT:
                os << "StdioRedirection::INPUT";
                break;
            case StdioRedirection::TRUNCATED_OUTPUT:
                os << "StdioRedirection::TRUNCATED_OUTPUT";
                break;
            case StdioRedirection::APPENDED_OUTPUT:
                os << "StdioRedirection::APPENDED_OUTPUT";
                break;
            default:
                os << "UNKNOWN" << '(' << static_cast<int>(type) << ')';
                break;
            }
        }
        else {
            os << static_cast<int>(type);
        }

        return os;
    }

    template <typename CharT, typename Traits>
    std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits>& os,
        const VariableAssignment& variable)
    {
        using namespace cli::prettyprint;

        if (isPrettyprintEnabled(os)) {
            os << CHART_LITERAL(CharT, "(struct){")
                << prettyprint::endlAndIndent;
            os << CHART_LITERAL(CharT, "name: ") << variable.name
                << CHART_LITERAL(CharT, ",") << endl;
            os << CHART_LITERAL(CharT, "value: ") << variable.value
                << endlAndDeindent;
        }
        else {
            os << CHART_LITERAL(CharT, "{");
            os << CHART_LITERAL(CharT, "name: ") << variable.name
                << CHART_LITERAL(CharT, ", ");
            os << CHART_LITERAL(CharT, "value: ") << variable.value;
        }

        return os << CHART_LITERAL(CharT, '}');
    }

    //
    // Class ShellParser
    //
    // Because it is used together with BasicSpiritParser, this parser must
    // return a two-references Sequence. They have to refer to the name and
    // the arguments of parsed command respectively.
    //
    // The parser uses ISO-8859 encoding to avoid problems with UTF-8 strings
    // because the Boost.Spirit support of ASCII encoding launches an
    // exceptions when finds 8-bit characters.
    //

    template <typename Iterator>
    struct ShellParser
        : qi::grammar<Iterator, fusion::vector<std::string&, Arguments&>(),
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
            : qi::symbols<char, Arguments::TypeOfTerminator>
        {
            Terminators()
            {
                add
                    (";", Arguments::NORMAL)
                    ("&", Arguments::BACKGROUNDED)
                ;
            }
        } terminators;

        struct Pipe
            : qi::symbols<char, Arguments::TypeOfTerminator>
        {
            Pipe()
            {
                add
                    ("|", Arguments::PIPED)
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
        qi::rule<Iterator, Arguments(), iso8859_1::space_type> command;
        qi::rule<Iterator, fusion::vector<std::string&, Arguments&>(),
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
}}}

namespace cli
{
    using namespace cli::parser;

    typedef shellparser::Arguments ShellArguments;
    typedef shellparser::VariableAssignment VariableAssignment;
    typedef shellparser::StdioRedirection StdioRedirection;

    namespace traits
    {
        template <>
        struct ParserTraits<spiritparser::BasicSpiritParser<ShellArguments,
            shellparser::ShellParser> >
        {
            typedef ShellArguments ArgumentsType;
            typedef spiritparser::SpiritParseError ErrorType;
        };
    }

    //
    // Class ShellInterpreter
    //
    // Interpreter which uses ShellParser to parse the command line, emulating
    // a very simple shell.
    //

    class ShellInterpreter
        : public cli::BasicSpiritInterpreter<ShellArguments,
              shellparser::ShellParser>
    {
        public:
            typedef cli::BasicSpiritInterpreter<ShellArguments,
                shellparser::ShellParser> BaseType;

            ShellInterpreter(bool useReadline = true);
            ShellInterpreter(std::istream& in, std::ostream& out,
                std::ostream& err = std::cerr, bool useReadline = true);

            //
            // Accessors of callback functions
            //

            cli::callback::VariableLookupCallback onVariableLookup;
            cli::callback::PathnameExpansionCallback onPathnameExpansion;

        private:

            //
            // Hook methods invoked during parsing
            //

            std::string variableLookup(const std::string& name);
            std::vector<std::string> pathnameExpansion(
                const std::string& pattern);
    };
}

#endif /* SHELL_HPP_ */
