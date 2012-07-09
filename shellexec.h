
#ifndef shellexec_h
#define shellexec_h
#pragma once

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef enum {
    EE_OK,
    EE_FORK = 0x1001,
    EE_FILE,
    EE_PIPE,
    EE_REDIRECT,
    EE_EXEC
} ExecErrorT;

typedef struct {
    pid_t pid;
    int exit_code;
    
    /* IO handles from the perspective of the child process */
    int input;
    int output;
    int error;
    
} ProcessInfoT;

ExecErrorT simple_exec(ProcessInfoT* info,
                        const char* file, char *const argv[],
                        const char* input, const char* output, const char* error);

int simple_exec_wait(ProcessInfoT* info);

#endif
