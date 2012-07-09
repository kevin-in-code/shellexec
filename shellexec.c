
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

ExecErrorT simple_exec(ProcessInfoT* info, const char* file, char *const argv[], const char* input, const char* output, const char* error) {
    int inPipes[2] = {0, 0};
    int outPipes[2] = {0, 0};
    int errPipes[2] = {0, 0};
    int errToOut = 0;
    int outToErr = 0;
    int errToNull = 0;
    int outToNull = 0;
    
    info->pid = 0;
    info->exit_code = 0;
    info->input = 0;
    info->output = 0;
    info->error = 0;

    /* Initialise any pipes that may be needed. */
    if ((input) && (strcmp(input, "|") == 0))
    {
        if (pipe(inPipes)) return EE_PIPE;
    }
    if (output)
    {
        if (strcmp(output, "|") == 0)
        {
            if (pipe(outPipes)) return EE_PIPE;
        }
        else
        if (strcmp(output, "|err") == 0)
        {
            outToErr = 1;
        }
        else
        if (strcmp(output, "|null") == 0)
        {
            outToNull = 1;
        }
    }
    if (error)
    {
        if (strcmp(error, "|") == 0)
        {
            if (pipe(errPipes)) return EE_PIPE;
        }
        else
        if (strcmp(output, "|out") == 0)
        {
            errToOut = 1;
        }
        else
        if (strcmp(output, "|null") == 0)
        {
            errToNull = 1;
        }
    }

    pid_t childpid = fork();
    if (childpid) {
        if (childpid == -1) return EE_FORK;
        /* Parent has successfully forked and has a child process. */
        
        /* Close the pipes that the parent does not need. */
        if (inPipes[0]) close(inPipes[0]);
        if (outPipes[1]) close(outPipes[1]);
        if (errPipes[1]) close(errPipes[1]);

        info->input = inPipes[1]; /* Parent may output to this handle to provide input. */
        info->output = outPipes[0];
        info->error = errPipes[0];
        
        info->pid = childpid;
        
        return EE_OK;
    }
    else
    {
        /* Child process is running and can execute the intended command. */

        /* Close the pipes that the child does not need. */
        if (inPipes[1]) close(inPipes[1]);
        if (outPipes[0]) close(outPipes[0]);
        if (errPipes[0]) close(errPipes[0]);
        
        /* Open files for file redirection. */
        int fin = 0;
        int fout = 0;
        int ferr = 0;

        if ((input) && (input[0] != '|'))
        {
            int handle = open(input, O_RDONLY);
            if (handle <= 0) exit(EE_FILE);
            fin = handle;
            inPipes[0] = handle;
        }
        if ((output) && (output[0] != '|'))
        {
            int handle = open(output, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (handle <= 0) exit(EE_FILE);
            fout = handle;
            outPipes[1] = handle;
        }
        if ((error) && (error[0] != '|'))
        {
            int handle = open(error, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            if (handle <= 0) exit(EE_FILE);
            ferr = handle;
            errPipes[1] = handle;
        }

        /* Now we redirect the to files and piped streams. */
        if (outPipes[1])
        {
            if (dup2(outPipes[1], STDOUT_FILENO) == -1) exit(EE_PIPE);
        }
        if (errPipes[1])
        {
            if (dup2(errPipes[1], STDERR_FILENO) == -1) exit(EE_PIPE);
        }
        if (inPipes[0])
        {
            if (dup2(inPipes[0], STDIN_FILENO) == -1) exit(EE_PIPE);
        }
        
        /* Next we perform symbolic stream redirection. */
        if (errToOut)
        {
            if (dup2(STDOUT_FILENO, STDERR_FILENO) == -1) exit(EE_REDIRECT);
        }
        if (outToErr)
        {
            if (dup2(STDERR_FILENO, STDOUT_FILENO) == -1) exit(EE_REDIRECT);
        }
        
        /* Finally, we close open file handles. */
        /* Question: should open pipe handles also be closed? Investigate. */
        if (fin) close(fin);
        if (fout) close(fout);
        if (ferr) close(ferr);
		        
        int r = execvp(file, argv);
        if (r) exit(EE_EXEC);
        return 0;
    }
}

int simple_exec_wait(ProcessInfoT* info) {
    int status;
    waitpid(info->pid, &status, 0);
    if (WIFEXITED(status)) {
        info->exit_code = WEXITSTATUS(status);
    }
    return info->exit_code;
}
