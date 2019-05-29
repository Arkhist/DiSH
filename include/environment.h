#pragma once

#include "common.h"

extern char** environ;

int processAliases(Command* cmd);

int removeAlias(char* alias);
int addAlias(char** args);