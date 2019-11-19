// A header file for helpers.c
// Declare any additional functions in this file
#pragma once

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include "icssh.h"
#include "linkedList.h"


void farewells(List_t *bglist);

void killthemall(List_t *bglist);