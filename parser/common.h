#pragma once

#define YYSTYPE TokType
#define MAX_PATH 4096

#include <stdlib.h>
#include <stdio.h>

char* strdup(char* s); 

enum RedirectionMode
{
    RED_WRITE = '>',
    RED_APPEND = '!',
    RED_READ = '<'
};

typedef struct
{
    int descriptor;
    char* target;
    enum RedirectionMode mode;
} FileRedirection;

typedef struct
{
    int argc;
    char** argv;

    int redirAmt;
    FileRedirection* redirections;
} Command;

Command* cmd_create();
Command* cmd_copy(Command* cmd);
void cmd_addArg(Command* cmd, char* arg);
void cmd_addRedirect(Command* cmd, FileRedirection redir);
void cmd_destroy(Command* cmd);

typedef struct
{
    int amt;
    Command** pipeline;
    int background;
    enum ChainType* chain;
} CommandLine;

CommandLine* cmdl_create();
void cmdl_addCmd(CommandLine* line, Command* cmd, enum ChainType chain);
void cmdl_destroy(CommandLine* line);

typedef struct
{
    int amt;
    CommandLine** lines;
} CommandPack;

CommandPack* cmdp_create();
void cmdp_addLine(CommandPack* pack, CommandLine* line);
void cmdp_destroy(CommandPack* pack);

typedef struct
{
    char* strVal;
    int iVal;

    Command* cmd;
    CommandLine* line;
    CommandPack* pack;

    FileRedirection red;
} TokType;

enum ChainType
{
    CHAIN_PIPE,
    CHAIN_AND,
    CHAIN_NONE
};

extern CommandPack* outPack;

CommandPack* parseCommands();