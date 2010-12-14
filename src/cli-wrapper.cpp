/*
 * cli-wrapper.cpp - C wrapper for command-line interpreter framework
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

#include <cassert>

#include <boost/function.hpp>

#include <cli/cli.hpp>
#include <cli/parser.hpp>
#include <cli/auxiliary.hpp>

#include "cli-wrapper.h"

typedef cli::CommandLineInterpreter<cli::parser::SimpleShellParser,
    std::vector<cli::parser::Command> >::ClassType InterpreterType;

//
// Memory management functions
//

void* cliMalloc(int size)
{
    try {
        return new char[size];
    }
    catch (...) {
        return NULL;
    }
}

void cliFree(void *p)
{
    delete reinterpret_cast<char*>(p);
}

//
// Constructor and destructor of command-line interpreter objects
//

bool cliCreate(CommandLineInterpreter* cli)
{
    InterpreterType* interpreter;
    try {
        interpreter = new InterpreterType();
    }
    catch (...) {
        return false;
    }

    *cli = interpreter;
    return true;
}

void cliDestroy(CommandLineInterpreter cli)
{
    InterpreterType* interpreter = reinterpret_cast<InterpreterType*>(cli);
    delete interpreter;
}

//
// Functions to interpret command-line input
//

bool cliLoop(CommandLineInterpreter cli)
{
    InterpreterType* interpreter = reinterpret_cast<InterpreterType*>(cli);
    try {
        interpreter->loop();
    }
    catch (...) {
        return false;
    }
    return true;
}

bool cliInterpretOneLine(CommandLineInterpreter cli, const char* line)
{
    InterpreterType* interpreter = reinterpret_cast<InterpreterType*>(cli);
    try {
        interpreter->interpretOneLine(line);
    }
    catch (...) {
        return false;
    }
    return true;
}

//
// Object getter and setters
//

const char *cliGetLastCommand(CommandLineInterpreter cli)
{
    InterpreterType* interpreter = reinterpret_cast<InterpreterType*>(cli);
    return interpreter->getLastCommand().c_str();
}

void cliSetIntroText(CommandLineInterpreter cli, const char* intro)
{
    InterpreterType* interpreter = reinterpret_cast<InterpreterType*>(cli);
    interpreter->setIntroText(intro);
}

void cliSetPromptText(CommandLineInterpreter cli, const char* prompt)
{
    InterpreterType* interpreter = reinterpret_cast<InterpreterType*>(cli);
    interpreter->setPromptText(prompt);
}

//
// Callback wrapper classes
//

struct BaseCallbackWrapper {};

struct RunCommandCallbackWrapper : public BaseCallbackWrapper
{
    RunCommandCallbackWrapper(cliRunCommandCallback callback)
        : callback_(callback) {}

    bool operator()(InterpreterType* object,
        const InterpreterType::CommandType& command)
    {
        int length = command.size();
        assert(length != 0);
        Command* c_command = new Command[length];
        for (int i = 0; i < length; i++) {
            copyCommand(c_command[i], command[i]);
        }

        bool result = callback_(object, length, c_command);

        for (int i = 0; i < length; i++) {
            destroyCommand(c_command[i]);
        }
        delete c_command;
        return result;
    }

    Command& copyCommand(Command& destination,
        const InterpreterType::CommandType::value_type& source)
    {
        int nArguments = source.arguments.size();
        destination.nArguments = nArguments;
        if (nArguments > 0) {
            destination.arguments =
                cli::auxiliary::stdVectorStringToArgV(source.arguments);
        }
        else {
            destination.arguments = NULL;
        }

        int nRedirections = source.redirections.size();
        destination.nRedirections = nRedirections;
        if (nRedirections > 0) {
            destination.redirections =
                new Command::StdioRedirection[nRedirections];
        }
        else {
            destination.redirections = NULL;
        }

        for (int i = 0; i < nRedirections; i++) {
            destination.redirections[i].argument =
                source.redirections[i].argument.c_str();

            switch (source.redirections[i].type) {
            case cli::parser::Command::StdioRedirection::TRUNCATED_INPUT:
                destination.redirections[i].type =
                    Command::StdioRedirection::TRUNCATED_INPUT;
                break;
            case cli::parser::Command::StdioRedirection::APPENDED_INPUT:
                destination.redirections[i].type =
                    Command::StdioRedirection::APPENDED_INPUT;
                break;
            case cli::parser::Command::StdioRedirection::TRUNCATED_OUTPUT:
                destination.redirections[i].type =
                    Command::StdioRedirection::TRUNCATED_OUTPUT;
                break;
            case cli::parser::Command::StdioRedirection::APPENDED_OUTPUT:
                destination.redirections[i].type =
                    Command::StdioRedirection::APPENDED_OUTPUT;
                break;
            }
        }

        switch (source.terminator) {
        case cli::parser::Command::NORMAL:
            destination.terminator = Command::NORMAL;
            break;
        case cli::parser::Command::BACKGROUNDED:
            destination.terminator = Command::BACKGROUNDED;
            break;
        case cli::parser::Command::PIPED:
            destination.terminator = Command::PIPED;
            break;
        }

        return destination;
    }

    void destroyCommand(Command& command)
    {
        if (command.arguments != NULL) {
            delete command.arguments;
        }
        if (command.redirections != NULL ) {
            delete command.redirections;
        }
    }

    private:
        cliRunCommandCallback callback_;
};

struct RunEmptyLineCallbackWrapper : public BaseCallbackWrapper
{
    RunEmptyLineCallbackWrapper(cliRunEmptyLineCallback callback)
        : callback_(callback) {}

    bool operator()(InterpreterType* object)
    {
        return callback_(object);
    }

    private:
        cliRunEmptyLineCallback callback_;
};

struct PreRunCommandCallbackWrapper : public BaseCallbackWrapper
{
    PreRunCommandCallbackWrapper(cliPreRunCommandCallback callback)
        : callback_(callback) {}

    void operator()(InterpreterType* object, std::string& line)
    {
        int length = line.length();
        char* c_line = new char[length + 1];
        line.copy(c_line, length, 0);
        c_line[length] = '\0';
        callback_(object, &c_line);
        line = c_line;
        delete c_line;
    }

    private:
        cliPreRunCommandCallback callback_;
};

struct PostRunCommandCallbackWrapper : public BaseCallbackWrapper
{
    PostRunCommandCallbackWrapper(cliPostRunCommandCallback callback)
        : callback_(callback) {}

    bool operator()(InterpreterType* object, bool isFinished,
        const std::string& line)
    {
        return callback_(object, isFinished, line.c_str());
    }

    private:
        cliPostRunCommandCallback callback_;
};

struct PrePostLoopCallbackWrapper : public BaseCallbackWrapper
{
    PrePostLoopCallbackWrapper(cliPreLoopCallback callback)
        : callback_(callback) {}

    void operator()(InterpreterType* object)
    {
        callback_(object);
    }

    private:
        cliPreLoopCallback callback_;
};

//
// Interpreter callback function setters
//

bool cliSetRunCommandCallback(CommandLineInterpreter cli,
    cliRunCommandCallback callback)
{
    InterpreterType* interpreter = reinterpret_cast<InterpreterType*>(cli);
    boost::function<bool (InterpreterType*,
        const InterpreterType::CommandType&)>
        function = RunCommandCallbackWrapper(callback);
    try {
        interpreter->setCallback("runCommand", function);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool cliSetRunEmptyLineCallback(CommandLineInterpreter cli,
    cliRunEmptyLineCallback callback)
{
    InterpreterType* interpreter = reinterpret_cast<InterpreterType*>(cli);
    boost::function<bool (InterpreterType*)>
        function = RunEmptyLineCallbackWrapper(callback);
    try {
        interpreter->setCallback("runEmptyLine", function);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool cliSetPreRunCommandCallback(CommandLineInterpreter cli,
    cliPreRunCommandCallback callback)
{
    InterpreterType* interpreter = reinterpret_cast<InterpreterType*>(cli);
    boost::function<void (InterpreterType*, std::string&)>
        function = PreRunCommandCallbackWrapper(callback);
    try {
        interpreter->setCallback("preRunCommand", function);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool cliSetPostRunCommandCallback(CommandLineInterpreter cli,
    cliPostRunCommandCallback callback)
{
    InterpreterType* interpreter = reinterpret_cast<InterpreterType*>(cli);
    boost::function<bool (InterpreterType*, bool, const std::string&)>
        function = PostRunCommandCallbackWrapper(callback);
    try {
        interpreter->setCallback("postRunCommand", function);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool cliSetPreLoopCallback(CommandLineInterpreter cli,
    cliPreLoopCallback callback)
{
    InterpreterType* interpreter = reinterpret_cast<InterpreterType*>(cli);
    boost::function<void (InterpreterType*)>
        function = PrePostLoopCallbackWrapper(callback);
    try {
        interpreter->setCallback("preLoop", function);
    }
    catch (...) {
        return false;
    }
    return true;
}

bool cliSetPostLoopCallback(CommandLineInterpreter cli,
    cliPostLoopCallback callback)
{
    InterpreterType* interpreter = reinterpret_cast<InterpreterType*>(cli);
    boost::function<void (InterpreterType*)>
        function = PrePostLoopCallbackWrapper(callback);
    try {
        interpreter->setCallback("postLoop", function);
    }
    catch (...) {
        return false;
    }
    return true;
}
