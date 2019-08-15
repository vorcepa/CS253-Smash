#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "smash.h"
#include "history.h"

#define MAXLINE 4096

void handle_interrupt(int signal){
    const char c[] = "^C\n";
    write(STDERR_FILENO, c, sizeof(c));

    return;
}

int main (){
    signal(SIGINT, handle_interrupt);
    char bfr[MAXLINE];
    
    fputs("$ ", stderr);
    while (fgets(bfr, MAXLINE, stdin) != NULL) {
        bfr[strlen(bfr) - 1] = '\0';    //replace newline with NUL
        processCommand(bfr);
        fputs("$ ", stderr);
    }

    char exitToken[] = "exit";
    processCommand(exitToken);
    return 0;
}