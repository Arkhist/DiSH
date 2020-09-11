#include "environment.h"

#include <stdlib.h>
#include <string.h>

extern char** environ;

int maxAliasId = 0;

typedef struct
{
    char* alias;
    Command* target;
} AliasStruct;

AliasStruct* aliasArray = NULL;

Command* processAliases(Command* cmd)
{
    for(int i = 0; i < maxAliasId; i++)
    {
        if(aliasArray[i].alias == NULL)
            continue;
        if(!strcmp(aliasArray[i].alias, cmd->argv[0]))
        {            
            Command* r = cmd_copy(aliasArray[i].target);
            for(int i = 1; i < cmd->argc; i++) {
                cmd_addArg(r, cmd->argv[i]);
            }
            for(int i = 0; i < cmd->redirAmt; i++)
                cmd_addRedirect(r, cmd->redirections[i]);
            return r;
        }
    }
    return cmd;
}

void processEnv(Command* cmd)
{
    char* buf;
    size_t buflen = 0;
    int parsingDollar = 0;
    for(int i = 0; i < cmd->argc; i++)
    {
        if(!cmd->argv[i])
            continue;
        int startIndex = 0, endIndex = 0, length = strlen(cmd->argv[i]);
        while(endIndex < length)
        {
            if(cmd->argv[i][endIndex] == '$')
            {
                if(!parsingDollar)
                {
                    parsingDollar = 1;
                    buflen = endIndex;
                    buf = malloc(buflen*sizeof(char)+1);
                    buf[buflen] = 0;
                    memcpy(buf, cmd->argv[i], endIndex);
                }
                else
                {
                    char* tmpbuf = malloc((endIndex-startIndex) * sizeof(char) + 1);
                    memcpy(tmpbuf, (cmd->argv[i])+startIndex, endIndex-startIndex);
                    char* envValue = getenv(tmpbuf);
                    free(tmpbuf);
                    if(envValue) {
                        buflen += sizeof(envValue);
                        buf = realloc(buf, (buflen+strlen(envValue))*sizeof(char)+1);
                        strcat(buf, envValue);
                    }
                    startIndex = endIndex;
                }
                startIndex++;
                endIndex++;
            }
            else
            {
                if(!parsingDollar)
                {
                    startIndex++;
                }
                endIndex++;
            }
        }
        if(parsingDollar && startIndex != endIndex)
        {
            char* tmpbuf = malloc((endIndex-startIndex) * sizeof(char) + 1);
            tmpbuf[endIndex-startIndex] = 0;
            memcpy(tmpbuf, (cmd->argv[i])+startIndex, endIndex-startIndex);
            char* envValue = getenv(tmpbuf);
            free(tmpbuf);
            if(envValue) {
                buflen += sizeof(envValue);
                buf = realloc(buf, (buflen+strlen(envValue))*sizeof(char)+1);
                strcat(buf, envValue);
            }
            free(cmd->argv[i]);
            cmd->argv[i] = buf;
        }
    }
}

int removeAlias(char* alias)
{
    for(int i = 0; i < maxAliasId; i++)
    {
        if(aliasArray[i].alias == NULL)
            continue;
        if(!strcmp(aliasArray[i].alias, alias))
        {
            free(aliasArray[i].alias);
            cmd_destroy(aliasArray[i].target);
            aliasArray[i].alias = NULL;
            aliasArray[i].target = NULL;
            return 1;
        }
    }
    return 0;
}


int addAlias(char* args[])
{
    // We suppose args contains at least 4 elements, and is terminated by NULL ptr.
    char* alias = args[1];
    int i = 0;

    Command* target = NULL;
    target = cmd_create();

    i = 2;
    while(args[i] != NULL)
    {
        cmd_addArg(target, strdup(args[i]));
        i++;
    }
    int targetId = -1;
    for(i = 0; i < maxAliasId; i++)
    {
        if(aliasArray[i].alias == NULL)
        {
            targetId = i;
            break;
        }
    }
    if(i == maxAliasId)
    {
        aliasArray = (AliasStruct*)realloc(aliasArray, maxAliasId+1); // TODO : error checking ?
        targetId = maxAliasId;
        maxAliasId++;
    }
    aliasArray[targetId].alias = strdup(alias);
    aliasArray[targetId].target = target;

    return 1;
}