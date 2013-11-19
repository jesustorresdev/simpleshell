/*
 * main.cpp - Demo in C++ of a simple shell
 *
 *   Copyright 2010-2013 Jesús Torres <jmtorres@ull.es>
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

#include <cstdlib>
#include <iostream>
#include <string>

#include <cli/callbacks.hpp>
#include <cli/prettyprint.hpp>
#include <cli/shell.hpp>
#include <cli/utility.hpp>

const char INTRO_TEXT[] = "\x1b[2J\x1b[H"
                          "Simple Shell - C++ Demo\n"
                          "Copyright 2010-2013 Jesús Torres <jmtorres@ull.es>\n";

const char PROMPT_TEXT[] = "$ ";

//
// Function to be invoked by the interpreter to substitute variable names
// in command-line by its value.
//
// It lookups then name in process environment variables and returns its
// value if the variable exist. Or returns an empty string in other case.
//

std::string variableLookupCallback(const std::string& name)
{
    return std::string(getenv(name.c_str()));
}

//
// Function to be invoked by the interpreter when the user inputs the
// 'exit' command.
//
// If this function returns true, the interpreter ends.
//
// cli::ShellArguments is an alias of cli::parser::shellparser::Arguments.
// See include/cli/shell.hpp for its definition.
//

bool onExit(const std::string& command, cli::ShellArguments const& arguments)
{
    std::cout << "command:   " << command << std::endl;
    std::cout << "arguments: " << arguments << std::endl;
    std::cout << std::endl;
    return true;
}

//
// Function to be invoked by the interpreter when the user inputs any
// other command.
//
// If this function returns true, the interpreter ends.
//
// cli::ShellArguments is an alias of cli::parser::shellparser::Arguments.
// See include/cli/shell.hpp for its definition.
//
//    struct ShellArguments
//    {
//        enum TypeOfTerminator
//        {
//            NORMAL,             // command ;
//            BACKGROUNDED,       // command &
//            PIPED               // command1 | command2
//        };
//
//        std::vector<VariableAssignment> variables;
//        std::vector<std::string> arguments;
//        std::vector<StdioRedirection> redirections;
//        TypeOfTerminator terminator;
//        ...
//    };
//
//    struct VariableAssignment
//    {
//        std::string name;
//        std::string value;
//    };
//
//    struct StdioRedirection
//    {
//        enum TypeOfRedirection
//        {
//            INPUT,              // command < filename
//            TRUNCATED_OUTPUT,   // command > filename
//            APPENDED_OUTPUT     // command >> filename
//        };
//
//        TypeOfRedirection type;
//        std::string argument;
//    };
//

bool onOtherCommand(const std::string& command,
    cli::ShellArguments const& arguments)
{
    using namespace cli::prettyprint;

    std::cout << prettyprint;
    std::cout << "command:   " << command << std::endl;
    std::cout << "arguments: " << arguments << std::endl;
    std::cout << "------------------------" << std::endl;
    std::cout << noprettyprint << std::endl;
    return false;
}

//
// Main function
//

int main(int argc, char** argv)
{
    // Create the shell-like interpreter
    cli::ShellInterpreter interpreter;

    // Set the intro and prompt texts
    interpreter.introText(INTRO_TEXT);
    interpreter.promptText(PROMPT_TEXT);

    // Set the callback function that will be invoked for variable substitution
    interpreter.setCallback<cli::callback::VariableLookupCallback>(
       &variableLookupCallback);

    // Set the callback function that will be invoked when the user inputs
    // the 'exit' command
    interpreter.onRunCommand("exit", &onExit);

    // Set the callback function that will be invoked when the user inputs
    // any other command
    interpreter.onRunCommand(&onOtherCommand);

    // Run the interpreter
    interpreter.loop();

    return 0;
}
