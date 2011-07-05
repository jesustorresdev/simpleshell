/*
 * main.cpp - Demo in C++ of a simple shell
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

#include <iostream>
#include <string>

#include <cli/callbacks.hpp>
#include <cli/cli.hpp>
#include <cli/parsers.hpp>

const char INTRO_TEXT[] = "\x1b[2J\x1b[H"
                          "Simple Shell - C++ Demo\n"
                          "Copyright 2010-2011 Jesús Torres <jmtorres@ull.es>\n";

const char PROMPT_TEXT[] = "$ ";

bool defaultCommandCallback(const std::string& command,
    const cli::parser::shellparser::CommandDetails& details)
{
    std::cout << command << ": ";
    std::cout << details << std::endl;
    return false;
}

bool exitCommandCallback(const std::string& command,
    const cli::parser::shellparser::CommandDetails& details)
{
    return true;
}

int main(int argc, char** argv)
{
    cli::CommandLineInterpreter<cli::parser::ShellParser> interpreter;
    interpreter.setIntroText(INTRO_TEXT);
    interpreter.setPromptText(PROMPT_TEXT);

    interpreter.setCallback<cli::callback::DoCommandCallback>(
        &defaultCommandCallback);
    interpreter.setCallback<cli::callback::DoCommandCallback>(
        &exitCommandCallback, "exit");

    interpreter.loop();

    return 0;
}
