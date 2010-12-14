{*
 * cliwrapper.pas - Pascal wrapper for command-line interpreter framework
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
 *}
 
unit cliwrapper;

interface

{$IFDEF FPC}
{$LINKLIB cli-c}
{$PACKRECORDS C}
{$ENDIF}

    { Struct where store the information required for command execution }
    type
        StdioRedirection = record
            argument : Pchar;           (* declared as const char* in C *)
            typeOfRedirection : longint;
        end;
        
        PStdioRedirection = ^StdioRedirection;

        Command = record
            nArguments : longint;
            arguments : PPchar;         (* declared as const char** in C *)
            nRedirections : longint;
            redirections : PStdioRedirection;
            typeOfTerminator : longint;
        end;
        
        PCommand  = ^Command;
    
    { Valid values for StdioRedirection.typeOfRedirection }
    const
        TRUNCATED_INPUT_REDIRECTION = 0;
        APPENDED_INPUT_REDIRECTION = 1;
        TRUNCATED_OUTPUT_REDIRECTION = 2;
        APPENDED_OUTPUT_REDIRECTION = 3;
    
    { Valid values for Command.typeOfTerminator }
    const
        NORMAL_TERMINATOR = 0;
        BACKGROUNDED_TERMINATOR = 1;
        PIPED_TERMINATOR = 2;

    { Command-line interpreter object }
    type
        CommandLineInterpreter = pointer;
        PCommandLineInterpreter  = ^CommandLineInterpreter;

    { Callback functions }
    type
        cliRunCommandCallback = function (cli:CommandLineInterpreter;
            nCommands:longint; commands:PCommand):boolean;cdecl;
    
        cliRunEmptyLineCallback = function
    	    (cli:CommandLineInterpreter):boolean;cdecl;
    	
        cliPreRunCommandCallback = procedure (cli:CommandLineInterpreter;
            line:PPchar);cdecl;
    
        cliPostRunCommandCallback = function (cli:CommandLineInterpreter;
            isFinished:boolean; line:Pchar):boolean;cdecl;
    
        cliPreLoopCallback = procedure (cli:CommandLineInterpreter);cdecl;
        cliPostLoopCallback = procedure (cli:CommandLineInterpreter);cdecl;
  
    { Memory management functions }
    function cliMalloc(size:longint):pointer;cdecl;external;
    procedure cliFree(p:pointer);cdecl;external;

    { Constructor and destructor of command-line interpreter objects }
      function cliCreate(cli:pCommandLineInterpreter):boolean;cdecl;external;
    procedure cliDestroy(cli:CommandLineInterpreter);cdecl;external;

    { Functions to interpret command-line input }
    function cliLoop(cli:CommandLineInterpreter):boolean;cdecl;external;
    function cliInterpretOneLine(cli:CommandLineInterpreter;
    	line:pchar):boolean;cdecl;external;

    { Get last inputted line }
    function
    	cliGetLastCommand(cli:CommandLineInterpreter):Pchar;cdecl;external;

    { Set intro and prompt interpreter strings }
    procedure cliSetIntroText(cli:CommandLineInterpreter;
    	intro:Pchar);cdecl;external;
    procedure cliSetPromptText(cli:CommandLineInterpreter;
    	prompt:Pchar);cdecl;external;

    { Interpreter callback function setters }
    function cliSetRunCommandCallback(cli:CommandLineInterpreter;
    	callback:cliRunCommandCallback):boolean;cdecl;external;
   	
    function cliSetRunEmptyLineCallback(cli:CommandLineInterpreter;
    	callback:cliRunEmptyLineCallback):boolean;cdecl;external;
    
    function cliSetPreRunCommandCallback(cli:CommandLineInterpreter;
    	callback:cliPreRunCommandCallback):boolean;cdecl;external;
    
    function cliSetPostRunCommandCallback(cli:CommandLineInterpreter;
    	callback:cliPostRunCommandCallback):boolean;cdecl;external;
    
    function cliSetPreLoopCallback(cli:CommandLineInterpreter;
    	callback:cliPreLoopCallback):boolean;cdecl;external;
    
    function cliSetPostLoopCallback(cli:CommandLineInterpreter;
    	callback:cliPostLoopCallback):boolean;cdecl;external;

implementation

end.
