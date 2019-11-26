#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void executeBuiltInCommand(short cmd_num);
short isBuiltInCommand(char info[]);
char* parseInput(char cmdLine[]);
char** argumentsParser(char input[]);
void executeCommand(char info[]);
int rowsAmount(char input[]);
int columnLength(char input[], int begin);
_Bool isBackgroundJob(char info[]);

int main (int argc, char **argv)
{
    int childPid;
    int backgroundPid;
    char* cmdLine;
    short cmd_num;
    cmdLine = (char *)malloc(sizeof(char)*20);
    char* info;
    info = (char *)malloc(sizeof(char)*20);
    while (1)                                                                                   // wait for user command
    {
        // put working directory using pwd
        gets(cmdLine);
        info = parseInput(cmdLine);                                                             // check if can put the returned array in info???
        cmd_num = isBuiltInCommand(info);
        if (cmd_num == 7)                                                                       // repeat command
        {
            //check history
        }
        else if(cmd_num)                                                                             // if a built-in command
        {
            executeBuiltInCommand(cmd_num);
        }
        else
        {
            childPid = fork();
            if (childPid == -1)
            {
                perror("fork error");                                                         // print error???
            }
            if (childPid == 0)                                                                  // child process
            {
                executeCommand(info);
            }
            else                                                                                // parent process
            {
                if (isBackgroundJob(info))
                {
                    backgroundPid = fork();
                    if (backgroundPid == -1)
                    {
                        perror("fork error");                                                         // print error???
                    }
                    if (backgroundPid == 0)                                                     // background process
                    {
                        //record pid in list of background jobs
                        executeCommand(info);
                    }
                }
                else
                {
                    //fix call!!!
                    waitpid(childPid);
                }
            }
        }
    }
}


//parse input to char array
char* parseInput(char cmdLine[])
{
  int i=0;
  char* tmp;
  while (cmdLine[i] != '\0')
  {
      i++;
  }
  tmp = (char *)malloc(sizeof(char)*(i+1));
  i=0;
  while (cmdLine[i] != '\0')
  {
      *(tmp+i) = cmdLine[i];
      i++;
  }
  *(tmp+i) = '\0';
    return  tmp;
}

// if a built-in command, return a non-zero short
short isBuiltInCommand(char info[])
{
    short cmd_num =0;
    if (strcmp(info,"jobs") == 0)
        cmd_num = 1;
    else if (strcmp(info,"cd") == 0)
        cmd_num = 2;
    else if (strcmp(info,"history") == 0)
        cmd_num = 3;
    else if (strcmp(info,"exit") == 0)
        cmd_num = 4;
    else if (strcmp(info,"kill") == 0)
        cmd_num = 5;
    else if (strcmp(info,"help") == 0)
        cmd_num = 6;
    else if (strcmp(info[0],"!") == 0)
        cmd_num = 7;
    return  cmd_num;
}

// execute built-in command by cmd_num
void executeBuiltInCommand(short cmd_num)
{
    switch (cmd_num)
    {
        case 1 :
            //executeJobs();
            break;
        case 2 :
            //executeCd();
            //chdir
            break;
        case 3 :
            //executeHistory();
            //executeHistorySizeSet()
            break;
        case 4 :
            //executeExit();
            //check the background is empty
            break;
        case 5 :
            //executeKill();
            //kill
            break;
        case 6 :
            //executeHelp();
            ;
    }
}

////cmd parser
//char* commandParser(char input[])
//{
//    char* cmd;
//    int i=0;
//    while (input[i] != ' ')
//    {
//        i++;
//    }
//    cmd = (char *)malloc(sizeof(char)*(i+1));
//    i=0;
//    while (input[i] != ' ')
//    {
//        *(cmd+i) = input[i];
//        i++;
//    }
//    *(cmd+i) = '\0';
//    return cmd;
//}

int rowsAmount(char input[])
{
    int i=0;                                                                                    // run over input     // run over argument
    int k=0;                                                                                    // run over returned array
    // find amount of arguments
    while (input[i] != '/0')
    {
        if (input[i] == ' ')
            k++;
        i++;
    }
    return k+1;
}

int columnLength(char input[], int begin)
{
    int i=0;
    while (input[begin+i] != '/0' || (input[begin+i] != ' '))
    {
        i++;
    }
    return i;
}


//arguments parser
char** argumentsParser(char input[])
{
    char** args;
    int i=0;                                                                                    // run over input
    int j=0;                                                                                    // run over argument
    int k=0;                                                                                    // run over returned array

    // find amount of arguments
    k = rowsAmount(input);
    args = (char **) malloc(sizeof(char) * (k));

    i=0;
    k=0;
    // find length of every argument
    while (input[i] != '/0')
    {
        j = columnLength(input, i);
        args[k] = (char *) malloc(sizeof(char) * (j + 1));
        k++;
        if (input[j+1] == '/0')                                                                  // if input is finished
            break;
        i=j+1;                                                                                   // next char that is not a space (2???)
    }

    i=0;                                                                                         // run over input
    j=0;                                                                                         // run over argument
    k=0;                                                                                         // run over returned array
    while (input[i] != '/0')
    {
        while (input[i+j] != ' ')
        {
            args[k][j] = input [i+j];
            j++;
        }
        args[k][j] = '/0';
        k++;
        if (input[j+1] == '/0')                                                                  // if input is finished
            break;
        i=j+2;                                                                                   // next char that is not a space
    }
    return args;
}

void executeCommand(char info[])
{
    char** args;

    // copied code !!!
    int i=0;                                                                                    // run over input
    int j=0;                                                                                    // run over argument
    int k=0;                                                                                    // run over returned array

    // find amount of arguments
    k = rowsAmount(info);
    args = (char **) malloc(sizeof(char) * (k));

    i=0;
    k=0;
    // find length of every argument
    while (info[i] != '/0')
    {
        j = columnLength(info, i);
        args[k] = (char *) malloc(sizeof(char) * (j + 1));
        k++;
        if (info[j+1] == '/0')                                                                  // if input is finished
            break;
        i=j+1;                                                                                   // next char that is not a space (2???)
    }
    //

    args = argumentsParser(info);
    execvp(args[0],args);
}

_Bool isBackgroundJob(char info[])
{
    int i=0;
    while (info[i] != '/0')
    {
        if (info[i] == '&')
            return true;
        i++;
    }
    return false;
}
