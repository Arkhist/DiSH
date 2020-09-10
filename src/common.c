#include "common.h"

#include <stdlib.h>
#include <string.h>

extern int yyparse();

CommandPack* outPack = NULL;

char* strdup(char* s)
{
    if(s == NULL)
        return NULL;
    char* r;
    size_t len = strlen(s);

    r = (char*)calloc(len+2, sizeof(char));
    strcpy(r, s);

    return r;
}

Command* cmd_create()
{
    Command* r = (Command*)calloc(1, sizeof(Command));
    r->argc = 0;
    r->argv = NULL;
    r->redirections = NULL;
    r->redirAmt = 0;
    return r;
}
Command* cmd_copy(Command* cmd)
{
    Command* r = cmd_create();
    for(int i = 0; i < cmd->argc; i++)
        cmd_addArg(r, cmd->argv[i]);
    for(int i = 0; i < cmd->redirAmt; i++)
        cmd_addRedirect(r, cmd->redirections[i]);
    return r;
}

void cmd_addArg(Command* cmd, char* arg)
{
    cmd->argv = (char**)realloc(cmd->argv, (cmd->argc+1)*sizeof(char*));
    cmd->argv[cmd->argc++] = strdup(arg);
}
void cmd_addRedirect(Command* cmd, FileRedirection redir)
{
    for(int i = 0; i < cmd->redirAmt; i++)
    {
        if(cmd->redirections[i].descriptor == redir.descriptor)
        {
            cmd->redirections[i] = redir;
            return;
        }
    }
    cmd->redirections = (FileRedirection*)realloc(cmd->redirections, (cmd->redirAmt+1)*sizeof(FileRedirection));
    cmd->redirections[cmd->redirAmt++] = redir;
}
void cmd_destroy(Command* cmd)
{
    if(cmd->argc > 0)
    {
        for(int i = 0; i < cmd->argc-1; i++)
            free(cmd->argv[i]);
        free(cmd->argv);
    }
    if(cmd->redirAmt)
        free(cmd->redirections);
    free(cmd);
}

CommandLine* cmdl_create()
{
    CommandLine* r = (CommandLine*)calloc(1, sizeof(CommandLine));
    r->amt = 0;
    r->background = 0;
    r->pipeline = NULL;
    r->chain = NULL;
    return r;
}
void cmdl_addCmd(CommandLine* line, Command* cmd, enum ChainType chain)
{
    line->pipeline = (Command**)realloc(line->pipeline, (line->amt+1)*sizeof(Command*));
    line->pipeline[line->amt] = cmd;
    line->chain = (enum ChainType*)realloc(line->chain, (line->amt+1)*sizeof(enum ChainType));
    line->chain[line->amt++] = chain;
}
void cmdl_destroy(CommandLine* line)
{
    if(line->amt > 0)
    {
        for(int i = 0; i < line->amt; i++)
        {
            if(line->pipeline[i] != NULL)
                free(line->pipeline[i]);
        }
        free(line->pipeline);
        free(line->chain);
    }
    free(line);
}

CommandPack* cmdp_create()
{
    CommandPack* r = (CommandPack*)calloc(1, sizeof(CommandPack));
    r->amt = 0;
    r->lines = NULL;
    return r;
}
void cmdp_addLine(CommandPack* pack, CommandLine* line)
{
    pack->lines = (CommandLine**)realloc(pack->lines, (pack->amt+1)*sizeof(CommandLine*));
    pack->lines[pack->amt++] = line;
}
void cmdp_destroy(CommandPack* pack)
{
    if(pack->amt > 0)
    {
        for(int i = 0; i < pack->amt; i++)
            if(pack->lines[i] != NULL)
                free(pack->lines[i]);
        free(pack->lines);
    }
    free(pack);
}

CommandPack* parseCommands(FILE* inputFile)
{
    static int initialized = 0;
    if(!initialized) {
        yyrestart(inputFile);
        initialized = 1;
    }
    if(yyparse())
        return NULL;
    return outPack;
}