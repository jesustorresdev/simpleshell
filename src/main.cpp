/*
 * main.cpp - Demo in C++ of a simple shell
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

#include <ostream>
#include <vector>

#include <cli/auxiliary.hpp>
#include <cli/callbacks.hpp>
#include <cli/cli.hpp>
#include <cli/shell_parser.hpp>

const char INTRO_TEXT[] = "\x1b[2J\x1b[H"
                          "Simple Shell - C++ Demo\n"
                          "Copyright 2010 Jesús Torres <jmtorres@ull.es>\n";

const char PROMPT_TEXT[] = "$ ";

typedef cli::CommandLineInterpreter<cli::parser::ShellParser> InterpreterType;

bool doCommandCallback(const InterpreterType::CommandType& commands)
{
    if (! commands.empty()) {
        std::cout << commands << std::endl;
        for (InterpreterType::CommandType::const_iterator iter =
            commands.begin(); iter < commands.end(); ++iter)
        {
            if (! iter->arguments.empty()) {
                if (iter->arguments[0] == "exit") {
                    return true;
                }
            }
        }
    }
    return false;
}

int main(int argc, char** argv)
{
    InterpreterType interpreter;
    interpreter.setIntroText(INTRO_TEXT);
    interpreter.setPromptText(PROMPT_TEXT);

    interpreter.setCallback<cli::callbacks::DoCommandCallback>(
        &doCommandCallback);

    interpreter.loop();

    return 0;
}
