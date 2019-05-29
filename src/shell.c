#include "shell.h"

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <dirent.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
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
    dup2(desc, target);
    return 1;
}

int executeFromPath(int pipes[2], Command* cmd, /*@out@*/ int* ret)
{
    pid_t forkVal;
    forkVal = fork();
    if(forkVal == 0)
    {
        for(int i = 0; i < cmd->redirAmt; i++)
        {
            FileRedirection r = cmd->redirections[i]; // TODO : Add modes to redirections
            int fd = open(r.target, O_RDWR | O_CREAT | ((r.mode != RED_WRITE) ? O_APPEND : O_TRUNC ), 0666);
            redirect(fd, r.descriptor); // TODO : check errors
        }
        for(int i = 0; i < 2; i++)
            if(pipes[i] != -1)
                redirect(pipes[i], i); // TODO : check errors
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

int executeCommand(int pipes[2], Command* cmd)
{
    processAliases(cmd);
    if(cmd->argc == 0)
        return 0;
    int ret = 0;
    if(executeInternal(cmd, &ret))
        return ret;
    if(executeFromPath(pipes, cmd, &ret))
        return ret;
    return 0;
}

int executeLine(CommandLine* line)
{
    int ret = 0;
    int pipeFirst[2] = {-1, -1};
    int pipeSecond[2] = {-1, -1};

    for(int i = 0; i < line->amt; i++)
    {
        pipe(pipeSecond);
        if(exiting)
            break;

        if(line->chain[i] == CHAIN_AND && ret != 0)
            return ret;
        int cPipe[2] = {-1, -1};
        if(i != line->amt - 1)
            if(line->chain[i+1] == CHAIN_PIPE)
                cPipe[1] = pipeSecond[1];
        if(line->chain[i] == CHAIN_PIPE)
        {
            cPipe[0] = pipeFirst[0];
        }
        ret = executeCommand(cPipe, line->pipeline[i]);

        if(pipeFirst[0] != -1)
            close(pipeFirst[0]);
        close(pipeSecond[1]);
        pipeSecond[1] = -1;
        for(int j = 0; j < 2; j++)
        {
            
            pipeFirst[j] = pipeSecond[j];
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