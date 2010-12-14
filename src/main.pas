{*
 * main.pas - Demo in Pascal of a simple shell
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

program SimpleShell;
uses cliwrapper, strings;

const
    IntroText = #27'[2J'#27'[H' +
       			'Simple Shell - Pascal Demo'#10 +
    			'Copyright 2010 Jesús Torres <jmtorres@ull.es>'#10#0;
    PromptText = '$ '#0;

var
    interpreter : cliwrapper.CommandLineInterpreter;
    isOk : boolean;

function runCommandCallback(cli:CommandLineInterpreter; nCommands:longint;
    commands:PCommand):boolean;cdecl;
begin
    if strings.strcomp(commands[0].arguments[0], 'exit'#0) = 0 then
        runCommandCallback := true
    else
    begin
        writeln(strings.strpas(commands[0].arguments[0]));
        runCommandCallback := false;
    end; 
end;

begin
    isOk := cliwrapper.cliCreate(@interpreter);
    if not isOk then
        Halt(1);
    
    cliwrapper.cliSetIntroText(interpreter, @IntroText[1]);
    cliwrapper.cliSetPromptText(interpreter, @PromptText[1]);
    
    isOk := cliwrapper.cliSetRunCommandCallback(interpreter,
        @runCommandCallback);
    if not isOk then
        Halt(2);
    
    isOk := cliwrapper.cliLoop(interpreter);
    if not isOk then
        Halt(3);
    
    cliwrapper.cliDestroy(interpreter);
end.
