%{
#include "common.h"


int yylex();
void yyerror(const char *s);

%}

%define parse.error verbose

%token NEWLINE SEPARATOR

%token WORD

%token IO_NUMBER

%token END

%token AMP PIPE LESS GREAT DGREAT AND EQUALS

%start CommandPack

%%

CommandPack     : CommandPack CommandLine background_opt {
                    if($2.line != NULL)
                    {
                        $2.line->background = $3.iVal;
                        cmdp_addLine($1.pack, $2.line);
                        outPack = $1.pack;
                    }
                    $$ = $1;
                }
                | CommandLine background_opt {
                    CommandPack* pack = cmdp_create();
                    if($1.line != NULL)
                    {
                        $1.line->background = $2.iVal;
                        cmdp_addLine(pack, $1.line);
                    }
                    TokType tok;
                    tok.pack = pack;
                    outPack = pack;
                    $$ = tok;
                    YYACCEPT;
                }
                | END   {
                    outPack = NULL;
                    YYACCEPT;
                }
                ;

CommandLine     : CommandLine Chain_op Command {
                    enum ChainType c = CHAIN_NONE;
                    switch($2.iVal)
                    {
                        case '&':
                            c = CHAIN_AND;
                            break;
                        case '|':
                            c = CHAIN_PIPE;
                            break;
                    }
                    cmdl_addCmd($1.line, $3.cmd, c);
                    $$ = $1;
                }
                | Command   {
                    CommandLine* line = cmdl_create();
                    cmdl_addCmd(line, $1.cmd, CHAIN_NONE);
                    TokType tok;
                    tok.line = line;
                    $$ = tok;
                }
                | Linebreak {
                    TokType tok;
                    tok.line = NULL;
                    $$ = tok;
                }
                ;

Chain_op        : PIPE  {
                    TokType tok;
                    tok.iVal = '|';
                    $$ = tok;
                }
                | AND   {
                    TokType tok;
                    tok.iVal = '&';
                    $$ = tok;
                }
                ;

background_opt  : AMP   {
                    TokType tok;
                    tok.iVal = 1;
                    $$ = tok;
                }
                |   {
                    TokType tok;
                    tok.iVal = 0;
                    $$ = tok;
                }
                ;

Command         : Modifiers WORD ArgList Modifiers  {
                    Command* cmd = cmd_create();
                    cmd_addArg(cmd, $2.strVal);
                    free($2.strVal);
                    for(int i = 0; i < $3.cmd->argc; i++)
                        cmd_addArg(cmd, $3.cmd->argv[i]);
                    cmd_destroy($3.cmd);
                    for(int i = 0; i < $1.cmd->redirAmt; i++)
                        cmd_addRedirect(cmd, $1.cmd->redirections[i]);
                    cmd_destroy($1.cmd);
                    for(int i = 0; i < $4.cmd->redirAmt; i++)
                        cmd_addRedirect(cmd, $4.cmd->redirections[i]);
                    cmd_destroy($4.cmd);
                    cmd_addArg(cmd, NULL);
                    TokType tok;
                    tok.cmd = cmd;
                    $$ = tok;
                }
                | Modifiers WORD EQUALS WORD {
                    Command* cmd = cmd_create();
                    cmd_addArg(cmd, "_assign");
                    cmd_addArg(cmd, $2.strVal);
                    free($2.strVal);
                    cmd_addArg(cmd, $4.strVal);
                    free($4.strVal);
                    cmd_addArg(cmd, NULL);
                    TokType tok;
                    tok.cmd = cmd;
                    $$ = tok;
                }
                ;

ArgList         : ArgList WORD  {
                    cmd_addArg($1.cmd, $2.strVal);
                    free($2.strVal);
                    $$ = $1;
                }
                |   {
                    Command* cmd = cmd_create();
                    TokType tok;
                    tok.cmd = cmd;
                    $$ = tok;
                }
                ;

Modifiers       : Modifiers Redirection {
                    cmd_addRedirect($1.cmd, $2.red);
                    $$ = $1;
                }
                |   {
                    Command* cmd = cmd_create();
                    TokType tok;
                    tok.cmd = cmd;
                    $$ = tok;
                }
                ;

Redirection     : IO_NUMBER Redirect_op WORD    {
                    FileRedirection red;
                    red.mode = $2.iVal;
                    red.descriptor = $1.iVal;
                    red.target = strdup($3.strVal);
                    free($3.strVal);
                    TokType tok;
                    tok.red = red;
                    $$ = tok;
                }
                | Redirect_op WORD {
                    FileRedirection red;
                    switch($1.iVal)
                    {
                        case '<':
                            red.descriptor = 0;
                            red.mode = RED_READ;
                            break;
                        case '>':
                            red.descriptor = 1;
                            red.mode = RED_WRITE;
                            break;
                        case '!':
                            red.descriptor = 1;
                            red.mode = RED_APPEND;
                            break;
                    }
                    red.target = strdup($2.strVal);
                    free($2.strVal);
                    TokType tok;
                    tok.red = red;
                    $$ = tok;
                }
                ;

Redirect_op     : LESS  {
                    TokType tok;
                    tok.iVal = '<';
                    $$ = tok;
                }
                | GREAT {
                    TokType tok;
                    tok.iVal = '>';
                    $$ = tok;
                }
                | DGREAT {
                    TokType tok;
                    tok.iVal = '!';
                    $$ = tok;
                }
                ;

Linebreak       : NEWLINE
                | SEPARATOR
                | Linebreak Linebreak
                ;
