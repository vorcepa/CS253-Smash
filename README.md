NAME
    smash - a simplified re-implementation of the bash Unix shell.

SYNOPSIS
    smash

DESCRIPTION
    Runs commands similar to how one would expect bash to execute them.

    Built-in commands:
        cd [DIR] - Change the shell working directory.  No flags are implemented.
        history - List the 10 most recent commands executed and their exit statuses while in this session of smash.
        exit - exits the smash shell (returns you to the bash shell)

    Smash can also execute external files and commands.  Common Linux commands such as ls and pwd
    will work in the same way as bash would.  If the command to be executed is an external one,
    smash will look in the directory provided.  If none is provided, it will check the current
    working directory, as well as /bin for the command.  An error will be reported if smash cannot
    find the command to execute.
    
    Exit Status:
        Returns 0 on exit.

AUTHOR
    Phillip Vorce
    Boise State University
    Summer, 2019
