#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>

#include "smash.h"
#include "history.h"

#define MAXLINE 4096

struct history* his = NULL;
pthread_mutex_t wait_children_mutex;

void *wait_children(void *none) {
    int status;
    pid_t pid;
    for (;;) {
        pthread_mutex_lock(&wait_children_mutex);
        for (;;) {
            pid = wait(&status);
            if (pid == -1) {
                if (errno == ECHILD) {
                    break;
                }
                fprintf(stderr, "wait() error\n");
                continue;
            }
            fprintf(stderr, "PID %d exited, status = %d\n", pid, status);
        }
        pthread_mutex_unlock(&wait_children_mutex);
    }
    return NULL;
}

void handle_interrupt(int signal){
    const char c[] = "^C\n";
    write(STDERR_FILENO, c, sizeof(c));

    return;
}

int main (){
    signal(SIGINT, handle_interrupt);
    char bfr[MAXLINE];
    his = init_history(10);

    pthread_mutex_init(&wait_children_mutex, NULL);
    pthread_mutex_lock(&wait_children_mutex);
    pthread_t wait_children_thread;
    pthread_create(&wait_children_thread, NULL, wait_children, NULL);
    
    fputs("$ ", stderr);
    while (fgets(bfr, MAXLINE, stdin) != NULL) {
        bfr[strlen(bfr) - 1] = '\0';    //replace newline with NUL
        processCommand(bfr);
        fputs("$ ", stderr);
    }

    clear_history(his);
    pthread_cancel(wait_children_thread);
    pthread_mutex_destroy(&wait_children_mutex);
    char exitToken[] = "exit";
    processCommand(exitToken);
    return 0;
}