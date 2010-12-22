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

#include <vector>

#include <cli/cli.hpp>
#include <cli/parser.hpp>

#include <boost/function.hpp>

const char INTRO_TEXT[] = "\x1b[2J\x1b[H"
                          "Simple Shell - C++ Demo\n"
                          "Copyright 2010 Jesús Torres <jmtorres@ull.es>\n";
const char PROMPT_TEXT[] = "$ ";

typedef cli::CommandLineInterpreter<cli::parser::SimpleShellParser,
    std::vector<cli::parser::Command> > InterpreterType;

bool runCommandCallback(InterpreterType* interpreter,
    const InterpreterType::CommandType& commands)
{
    if (commands.size() != 0) {
        interpreter->getOutStream() << commands << std::endl;
        if (commands[0].arguments[0] == "exit") {
            return true;
        }
    }
    return false;
}

int main(int argc, char** argv)
{
    using namespace cli;

    InterpreterType interpreter;
    interpreter.setIntroText(INTRO_TEXT);
    interpreter.setPromptText(PROMPT_TEXT);

    boost::function<bool (InterpreterType*,
        const InterpreterType::CommandType&)> callback = &runCommandCallback;
    interpreter.setCallback("runCommand", callback);

    interpreter.loop();

    return 0;
}
