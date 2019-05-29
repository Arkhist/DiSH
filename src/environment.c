#include "environment.h"

#include <stdlib.h>
#include <string.h>


int maxAliasId = 0;

typedef struct
{
    char* alias;
    char* target;
} AliasStruct;

AliasStruct* aliasArray = NULL;

// Argument alias ?
int processAliases(Command* cmd)
{
    for(int i = 0; i < maxAliasId; i++)
    {
        if(aliasArray[i].alias == NULL)
            continue;
        if(!strcmp(aliasArray[i].alias, cmd->argv[0]))
        {
            free(cmd->argv[0]);
            cmd->argv[0] = strdup(aliasArray[i].target);
        }
    }
    return 1;
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
            free(aliasArray[i].target);
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

    char* target = NULL;
    int tSize = 0;
    int i = 2;
    while(args[i] != NULL)
    {
        tSize += strlen(args[i])+1;
        i++;
    }
    target = (char*)calloc(tSize, sizeof(char));
    strcpy(target, args[2]);
    i = 3;
    while(args[i] != NULL)
    {
        strcat(target, " ");
        strcat(target, args[i]);
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