
#include "shellexec.h"
#include <stdio.h>

typedef int (*TestFunction)(char *const argv[]);

int ReportEE(ExecErrorT ee) {
    switch (ee) {
        case EE_OK:         printf("ok");
        case EE_FORK:       printf("fork problem");
        case EE_FILE:       printf("file problem");
        case EE_PIPE:       printf("pipe problem");
        case EE_REDIRECT:   printf("redirect problem");
        case EE_EXEC:       printf("exit code non-zero");
    }
    return 0;
}

int ReportExitCode(ProcessInfoT* info) {
    printf("exit code was %d", info->exit_code);
    return 0;
}



int TestOutRedirect(char *const argv[]) {
    ProcessInfoT info;
    ExecErrorT ee;
    ee = simple_exec(&info, "ls", argv, NULL, "out.txt", NULL);
    if (ee != EE_OK) return ReportEE(ee);
    simple_exec_wait(&info);
    if (info.exit_code != 0) return ReportExitCode(&info);
    return 1;
}



int RunTests(int count, const TestFunction tests[], char *const argv[]) {
    int c;
    int total = 0;
    for (c = 0; c < count; c++) {
        printf("Test %d: ", c);
        if (tests[c](argv)) {
            printf("ok");
            total++;
        }
        printf("\n");
    }
    return total;
}


const int NumTests = 1;
const TestFunction Tests[] = {
    TestOutRedirect
};

int main(int argc, char *const argv[]) {
    int total = RunTests(NumTests, Tests, argv);
    printf("%d of %d tests passed\n", total, NumTests);
}
