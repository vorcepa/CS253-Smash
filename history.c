#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "history.h"

struct history* init_history(int capacity){
    struct history* retVal = malloc(sizeof(struct history));
    retVal->capacity = capacity;
    retVal->offset = 0;
    retVal->entries = calloc((capacity), sizeof(*retVal->entries)); /*
                                                                          * assign the entries member to the value of the pointer returned by malloc.
                                                                          * *retVal->entries dereferences retVal (via ->) and then dereferences
                                                                          * entries (via *).  *entries == cmd*
                                                                          */
    return retVal;
}

int print_history(struct history* toPrint){
    int i;

    for (i = 0; i <= (toPrint->offset) - 1; i++){
        printf("%d [%d] %s\n", toPrint->offset - i, toPrint->entries[i]->exitStatus, toPrint->entries[i]->cmd);
        fflush(stdout);
    }

    return 0;
}

void add_history(struct history* history, char* command, int exitStatus){
    struct cmd* cmdToAdd = malloc(sizeof(*cmdToAdd));
    cmdToAdd->pid = 0;
    cmdToAdd->exitStatus = exitStatus;
    cmdToAdd->cmd = malloc(MAXLINE);
    strncpy(cmdToAdd->cmd, command, MAXLINE);

    if (history->offset == MAXHISTORY){
        free(history->entries[0]->cmd);
        free(history->entries[0]);
        memmove(&history->entries[0], &history->entries[1], (MAXHISTORY - 1) * sizeof (*history->entries));
        history->offset--;
    }

    history->entries[history->offset++] = cmdToAdd;
}

void clear_history(struct history* history){
    int i;
    for (i = 0; i < history->offset; i++){
        free(history->entries[i]->cmd);
        history->entries[i]->cmd = NULL;
        free(history->entries[i]);
        history->entries[i] = NULL;
    }

    free(history->entries);
    free(history);
}