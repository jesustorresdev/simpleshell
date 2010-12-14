/*
 * cli-wrapper.h - C wrapper for command-line interpreter framework
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

#ifndef CLI_WRAPPER_H_
#define CLI_WRAPPER_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Struct where store the information required for command execution */
struct Command
{
    int nArguments;
    const char** arguments;
    int nRedirections;

    struct StdioRedirection
    {
        enum TypeOfRedirection
        {
            TRUNCATED_INPUT,    // command <filename
            APPENDED_INPUT,     // command <<filename
            TRUNCATED_OUTPUT,   // command >filename
            APPENDED_OUTPUT     // command >>filename

        } type;

        const char* argument;
    } * redirections;

    enum TypeOfTerminator
    {
        NORMAL,                 // command ;
        BACKGROUNDED,           // command &
        PIPED                   // command1 | command2

    } terminator;
};

/* Command-line interpreter object type */
typedef void* CommandLineInterpreter;

/* Callback function types */
typedef bool (*cliRunCommandCallback)
    (CommandLineInterpreter cli, int nCommands,
        const struct Command* commands);

typedef bool (*cliRunEmptyLineCallback)
    (CommandLineInterpreter cli);

typedef void (*cliPreRunCommandCallback)
    (CommandLineInterpreter cli, char** line);

typedef bool (*cliPostRunCommandCallback)
    (CommandLineInterpreter cli, bool isFinished, const char* line);

typedef void (*cliPreLoopCallback)
    (CommandLineInterpreter cli);

typedef void (*cliPostLoopCallback)
    (CommandLineInterpreter cli);

/* Memory management functions */
void* cliMalloc(int size);
void cliFree(void *p);

/* Constructor and destructor of command-line interpreter objects */
bool cliCreate(CommandLineInterpreter* cli);
void cliDestroy(CommandLineInterpreter cli);

/* Functions to interpret command-line input */
bool cliLoop(CommandLineInterpreter cli);
bool cliInterpretOneLine(CommandLineInterpreter cli, const char* line);

/* Get last inputted line */
const char *cliGetLastCommand(CommandLineInterpreter cli);

/* Set intro and prompt interpreter strings */
void cliSetIntroText(CommandLineInterpreter cli, const char* intro);
void cliSetPromptText(CommandLineInterpreter cli, const char* prompt);

/* Interpreter callback function setters */
bool cliSetRunCommandCallback(CommandLineInterpreter cli,
    cliRunCommandCallback callback);

bool cliSetRunEmptyLineCallback(CommandLineInterpreter cli,
    cliRunEmptyLineCallback callback);

bool cliSetPreRunCommandCallback(CommandLineInterpreter cli,
    cliPreRunCommandCallback callback);

bool cliSetPostRunCommandCallback(CommandLineInterpreter cli,
    cliPostRunCommandCallback callback);

bool cliSetPreLoopCallback(CommandLineInterpreter cli,
    cliPreLoopCallback callback);

bool cliSetPostLoopCallback(CommandLineInterpreter cli,
    cliPostLoopCallback callback);

#ifdef __cplusplus
}
#endif

#endif /* CLI_WRAPPER_H_ */
