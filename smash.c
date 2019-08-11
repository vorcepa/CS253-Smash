#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "smash.h"
#include "history.h"

#define MAXLINE 4096

int main (){
    char bfr[MAXLINE];
    
    fputs("$ ",stderr);
    while (fgets(bfr, MAXLINE, stdin) != NULL) {
        bfr[strlen(bfr) - 1] = '\0';    //replace newline with NUL
        executeCommand(bfr);
        fputs("$ ", stderr);
    }

    char exitToken[] = "exit";
    executeCommand(exitToken);
    return 0;
}