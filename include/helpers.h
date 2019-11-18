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

#define INT_MODE 0
#define STR_MODE 1

void sigchld_handler();

int compare(void* a, void* b)   {
    return ((bgentry_t*)a)->seconds - ((bgentry_t*)b)->seconds;
}

void farewells(List_t *bglist);