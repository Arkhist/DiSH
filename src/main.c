#include <stdio.h>
#include <stdlib.h>

#include "shell.h"


int main(int argc, char** argv)
{
    FILE* profile;
    profile = fopen("profile", "r");
    if(profile != NULL)
    {
        mainLoop(profile);
        fclose(profile);
    }

    FILE* input = stdin;
    if(argc > 1)
    {
        input = fopen(argv[1], "r");
    }
    mainLoop(input);

    if(input != stdin)
        fclose(input);

    return 0;
}