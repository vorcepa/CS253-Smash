#ifndef __HISTORY_H
#define __HISTORY_H

#ifndef MAXLINE
        #define MAXLINE 4096
#endif

#ifndef MAXHISTORY
        #define MAXHISTORY 10
#endif

struct cmd {
        void* pid;
        char* cmd;
        int exitStatus;
        char** argv;
};

struct history {
        unsigned int capacity;
        unsigned int offset;
        struct cmd** entries;
};

struct history* init_history(int capacity);

void add_history(struct history* , char *cmd, int);

int print_history(struct history*);

void clear_history(struct history*);

#endif /* __HISTORY_H */