#include "environment.h"

#include <stdlib.h>
#include <string.h>


int maxAliasId = 0;

typedef struct
{
    char* alias;
    Command* target;
} AliasStruct;

AliasStruct* aliasArray = NULL;

// Argument alias ?
Command* processAliases(Command* cmd)
{
    for(int i = 0; i < maxAliasId; i++)
    {
        if(aliasArray[i].alias == NULL)
            continue;
        if(!strcmp(aliasArray[i].alias, cmd->argv[0]))
        {            
            Command* r = cmd_copy(aliasArray[i].target);
            for(int i = 0; i < cmd->redirAmt; i++)
                cmd_addRedirect(r, cmd->redirections[i]);
            return r;
        }
    }
    return cmd;
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