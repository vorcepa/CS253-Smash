int processCommand(char *str);
int doChangeDirectory(char**);

struct processList{
    pid_t* processID;
    int numProcesses;
    int capacity;
};
