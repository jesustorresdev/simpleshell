/*
 * main.c - Demo in C of a simple shell
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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "cli-wrapper.h"

const char INTRO_TEXT[] = "\x1b[2J\x1b[H"
                          "Simple Shell - C Demo\n"
                          "Copyright 2010 Jesús Torres <jmtorres@ull.es>\n";
const char PROMPT_TEXT[] = "$ ";

bool runCommandCallback(CommandLineInterpreter cli, int nCommand,
    const struct Command* commands)
{
    if (strcmp(commands[0].arguments[0], "exit") == 0) {
        return true;
    }
    else {
        puts(commands[0].arguments[0]);
    }
    return false;
}

int main(int argc, char** argv)
{
    CommandLineInterpreter interpreter;
    bool isOk = cliCreate(&interpreter);
    if (! isOk) {
        return 1;
    }

    cliSetIntroText(interpreter, INTRO_TEXT);
    cliSetPromptText(interpreter, PROMPT_TEXT);

    isOk = cliSetRunCommandCallback(interpreter, runCommandCallback);
    if (! isOk) {
        return 2;
    }

    isOk = cliLoop(interpreter);
    if (! isOk) {
        return 3;
    }

    cliDestroy(interpreter);

    return 0;
}
