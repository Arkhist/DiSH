#pragma once

#include "common.h"

extern int exiting;

int executeInternal(Command* cmd, /*@out@*/ int* ret);