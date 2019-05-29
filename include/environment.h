#pragma once

#include "common.h"

extern char** environ;

Command* processAliases(Command* cmd);

int removeAlias(char* alias);
int addAlias(char** args);