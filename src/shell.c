#include "shell.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <dirent.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "environment.h"
#include "common.h"
#include "internal.h"


void displayPrompt(int err)
{
    printf("dish [%d]> ", err);
}

int redirect(int desc, int target)
{
    int save = dup(target);
    close(target);
    if(dup2(desc, target) == -1)
    {
        dup2(save, target);
        close(save);
        return -1;
    }
    return save;
}

int executeFromPath(Command* cmd, /*@out@*/ int* ret)
{
    pid_t forkVal;
    forkVal = fork();
    if(forkVal == 0)
    {
        int r = execvp(cmd->argv[0], cmd->argv);
        exit(r);
    }
    else
    {
        int status = 0;
        wait(&status);
        int code = (char)WEXITSTATUS(status);
        *ret = code;
        if(code == -1)
            printf("Command not found: %s\n", cmd->argv[0]);
        return 1;
    }
}

int executeCommand(Command* cmd)
{
    if(cmd->argc == 0)
        return 0;
    int ret = 0;
    if(executeInternal(cmd, &ret))
        return ret;
    if(executeFromPath(cmd, &ret))
        return ret;
    return 0;
}

int executeLine(CommandLine* line)
{
    int ret = 0;
    int pipefd[2] = {-1, -1};
    for(int i = 0; i < line->amt; i++)
        if(line->chain[i] == CHAIN_PIPE)
        {
            pipe(pipefd);
        }
    for(int i = 0; i < line->amt; i++)
    {
        if(exiting)
            break;
        // Pipe behavior
        if(i != line->amt - 1)
        {
            if(line->chain[i+1] == CHAIN_PIPE)
            {
                redirect(1, pipefd[1]);
            }
        }
        if(line->chain[i] == CHAIN_PIPE)
        {
            redirect(0, pipefd[0]);
        }


        if(i == 0)
        {
            ret = executeCommand(line->pipeline[0]);
        }
        else
        {
            if(line->chain[i] == CHAIN_AND && ret != 0)
                return ret;
            
            ret = executeCommand(line->pipeline[i]);
        }
    }
    return ret;
}

int executePack(CommandPack* pack)
{
    int ret = 0;
    for(int i = 0; i < pack->amt; i++)
    {
        if(exiting)
            break;
        CommandLine* line = pack->lines[i];
        if(line->background)
        {
            pid_t f = fork();
            if(f == 0)
            {
                ret = executeLine(line);
                exit(ret);
            }
        }
        else
            ret = executeLine(line);
    }
    return ret;
}

int mainLoop()
{    
    int lastErr = 0;

    while(1)
    {
        displayPrompt(lastErr);
        CommandPack* pack = parseCommands();
        if(pack == NULL) // TODO : Newline doesn't work ?!
        {
            printf("\n");
            break;
        }

        lastErr = executePack(pack);
        if(exiting)
            break;
    }

    return 0;
}