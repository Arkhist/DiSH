#include "internal.h"

#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include "common.h"

#define INTERN_COMMAND_AMT 5

int exiting = 0;

typedef int (*CommandFunction)(Command*);

typedef struct {
    char* identifier;
    CommandFunction funct;
} CommandFunctID;


static int changeDirectory(Command* cmd);
static int printWorkingDirectory(Command* cmd);
static int assign(Command* cmd);
static int alias(Command* cmd);
static int exitSh(Command* cmd);


CommandFunctID internalCommands[INTERN_COMMAND_AMT] = {
    {"cd", changeDirectory},
    {"pwd", printWorkingDirectory},
    {"_assign", assign},
    {"alias", alias},
    {"exit", exitSh}
};

int executeInternal(Command* cmd, /*@out@*/ int* ret)
{
    for(int i = 0; i < INTERN_COMMAND_AMT; i++)
    {
        if(!strcmp(cmd->argv[0], internalCommands[i].identifier))
        {
            *ret = internalCommands[i].funct(cmd);
            return 1;
        }
    }
    return 0;
}

static int changeDirectory(Command* cmd)
{
    if(cmd->argc < 2)
    {
        printf("Usage: cd PATH\n");
        return 1;
    }

    // TODO : Simplify paths

    int pathLen = strlen(cmd->argv[1]);

    if(cmd->argv[1][0] == '/') // Absolute Path
    {
        DIR* curDir = NULL;
        // Maybe use access() ?
        curDir = opendir(cmd->argv[1]);
        if(curDir == NULL)
        {
            printf("Failed to open %s\n", cmd->argv[1]);
            return 1;
        }
        closedir(curDir);
        char newEnv[3+1+pathLen+1];
        strcpy(newEnv, "PWD=");
        strcat(newEnv, cmd->argv[1]);
        putenv(strdup(newEnv));
    }
    else // Relative Path
    {
        char* target = NULL;
        target = (char*)calloc(strlen(getenv("PWD"))+1, sizeof(char));
        strcpy(target, getenv("PWD"));
        DIR* curDir = NULL;

        int from = 0;
        int to = 0;

        while(to < pathLen)
        {
            to = from;
            while(to < pathLen && cmd->argv[1][to] != '/')
                to++;
            printf("%d\n", to);
            
            target = (char*)realloc(target, (strlen(target)+to-from+1)*sizeof(char));
            
            strcat(target, "/");
            for(int i = from; i < to; i++)
                strncat(target, &cmd->argv[1][i], 1);
            printf("%s\n", target);

            curDir = opendir(target);
            if(curDir == NULL)
            {
                printf("Failed to open %s\n", target);
                return 1;
            }
            closedir(curDir);
            from = to + 1;
        }

        char newEnv[3+1+strlen(target)+1];
        strcpy(newEnv, "PWD=");
        strcat(newEnv, target);
        putenv(strdup(newEnv));
    }

    return 0;
}

static int printWorkingDirectory(Command* cmd)
{
    printf("%s\n", getenv("PWD"));
    return 0;
}

static int assign(Command* cmd)
{
    if(cmd->argc != 3)
    {
        printf("Internal assignment error : args\n");
        return 1;
    }

    char newEnv[strlen(cmd->argv[1])+1+strlen(cmd->argv[2])+1];
    strcpy(newEnv, cmd->argv[1]);
    strcat(newEnv, "=");
    strcat(newEnv, cmd->argv[2]);
    putenv(strdup(newEnv));
    return 0;
}

static int alias(Command* cmd)
{
    if(cmd->argc < 2)
    {
        printf("Usage: alias ALIAS [ALIASED]\n");
        return 1;
    }
    

    return 0;
}

static int exitSh(Command* cmd)
{
    exiting = 1;
    return 0;
}