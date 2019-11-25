#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main (int argc, char **argv)
{
    int childPid;
    char * cmdLine;
    parseInfo *info;
    while (1)
    {
        cmdLine = readline(">");
        info = parase(cmdLine);
        if(isBuiltInCommand(info))
        {
            executeBuiltInCommand(info);
        }
        else
        {
            childPid = fork();
            if ( childPid == 0)
            {
                executeCommand(info);
            }
            else
            {
                if ( isBackgroundJob(info))
                {
                    record in list of background jobs
                }
                else
                {
                    waitpid(childPid);
                }
            }
        }
    }
}