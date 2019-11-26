#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct HistoryNode {
    char** parsedCommand;
    struct HistoryNode* next;
    short cmd_numH;
    char* unparsedCmd;
};

struct BackgroundNode {
    char*  unparsedCmd;
    int pid;
    struct BackgroundNode* next;
    struct BackgroundNode* previous;
};
struct BackgroundNode* firstB = NULL;
int lengthH =0;
int setH = 10;
_Bool flagH = false;
struct HistoryNode* repeatedHNode;

void executeBuiltInCommand(short cmd_num);
short isBuiltInCommand(char* info);
char* parseInput(char* cmdLine);
char** argumentsParser(char input[]);
//void executeCommand(char info[]);
int rowsAmount(char input[]);
int columnLength(char input[], int begin);
_Bool isBackgroundJob(char* info);
int getHsteps(char info[]);
short changeCmd(int steps, HistoryNode* firstH);
void getIndexNode(int index, HistoryNode* firstH);
void insertH(char** cmdLine, short cmd_num ,  HistoryNode* firstH );
void insertB(int pid ,BackgroundNode* firstB ,char* cmdLine );
void removeBGLink (int pid );
void printB();
void handler_bgPrcss ()
{
    int sigPid = waitpid(-1 , &status, WNOHANG);

    if (sigPid != -1)
    {
        removeBGLink (sigPid);
    }

}

int main (int argc, char **argv)
{
    signal (SIGCHLD , handler_bgPrcss );
    int childPid;
    int backgroundPid;
    char* cmdLine;
    short cmd_num;
    cmdLine = (char *)malloc(sizeof(char)*20);
    char** info;
    //info = (char *)malloc(sizeof(char)*20);

    struct HistoryNode* firstH = NULL;
    int stepsH=0;

    while (1)                                                                                   // wait for user command
    {
        // put working directory using pwd
        gets(cmdLine);
        //info = parseInput(cmdLine);                                                             // check if can put the returned array in info???
        cmd_num = isBuiltInCommand(cmdLine);
        if (cmd_num == 7)                                                                       // repeat command
        {
            stepsH = getHsteps(cmdLine);
            cmd_num = changeCmd(stepsH, firstH);
            flagH = true;
        }

        if(cmd_num)                                                                             // if a built-in command
        {
            //inset built in command
            executeBuiltInCommand(cmd_num);
            flagH = false;
            if(cmd_num != 8)
            {
                insertH(NULL, cmd_num , firstH , cmdLine);
            }
            else
            {
                info = parseInput(cmdLine);
                insertH(info,cmd_num,firstH,cmdLine);
            }
        }
        else
        {
            //inset command
            info = parseInput(cmdLine);
            insertH(info,cmd_num,firstH,cmdLine);
            flagH = false;
            childPid = fork();
            if (childPid == -1)
            {
                perror("fork error");                                                         // print error???
            }
            if (childPid == 0)                                                                  // child process
            {
                execvp(info[0],info);
            }
            else                                                                                // parent process
            {
                if (isBackgroundJob(cmdLine))
                {
                    backgroundPid = fork();
                    if (backgroundPid == -1)
                    {
                        perror("fork error");                                                         // print error???
                    }
                    if (backgroundPid == 0)                                                     // background process
                    {
                        insertB(getpid() , firstB , cmdLine );
                        execvp(info[0],info);
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
// change to [][]!!!!!!!!!!!!!!!!!!!!
char** parseInput(char* cmdLine)
{
    char** args;
    //info = if not from history - parse cmdLine into [][]
    if (flagH)                                          // repeated command
    {
        args = repeatedHNode->parsedCommand;
        return args;
    }
    else
    {
        int i=0;                                                                                    // run over input
        int j=0;                                                                                    // run over argument
        int k=0;                                                                                    // run over returned array

        // find amount of arguments
        k = rowsAmount(cmdLine);
        args = (char **) malloc(sizeof(char) * (k));

        i=0;
        k=0;
        // find length of every argument
        while (cmdLine[i] != '/0')
        {
            j = columnLength(cmdLine, i);
            args[k] = (char *) malloc(sizeof(char) * (j + 1));
            k++;
            if (cmdLine[j+1] == '/0')                                                                  // if input is finished
                break;
            i=j+1;                                                                                   // next char that is not a space (2???)
        }
        args = argumentsParser(cmdLine);
        return args;
    }

}

// if a built-in command, return a non-zero short
short isBuiltInCommand(char* info)
{
    short cmd_num =0;
    char copy[10] = "";
    memcpy(copy, &info, 10);

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
    else if (strcmp(copy,"history -s") == 0)
        cmd_num = 8;
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
            break;
        case 4 :
            //executeExit();
            //check the background is empty
            break;
        case 5 :
            //executeKill();
            //kill
            break;
        case 8 :
            //if set > length we just change set, else we remove nodes
            //executeHistorySizeSet()
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

//void executeCommand(char** info)
//{
//    char** args;
//
//    // copied code !!!
//    int i=0;                                                                                    // run over input
//    int j=0;                                                                                    // run over argument
//    int k=0;                                                                                    // run over returned array
//
//    // find amount of arguments
//    k = rowsAmount(info);
//    args = (char **) malloc(sizeof(char) * (k));
//
//    i=0;
//    k=0;
//    // find length of every argument
//    while (info[i] != '/0')
//    {
//        j = columnLength(info, i);
//        args[k] = (char *) malloc(sizeof(char) * (j + 1));
//        k++;
//        if (info[j+1] == '/0')                                                                  // if input is finished
//            break;
//        i=j+1;                                                                                   // next char that is not a space (2???)
//    }
//    //
//
//    args = argumentsParser(info);
//
//}

_Bool isBackgroundJob(char* cmdLine)
{
    int i=0;
    while (cmdLine[i] != '/0')
    {
        if (cmdLine[i] == '&')
            return true;
        i++;
    }
    return false;
}

int getHsteps(char info[])
{
    int number =0;
    int length=0;
    int i=1;
    sign =1;
    if (info[1] == '-')
    {
        sign = -1;
        i++;
    }
    int j= i;
    while (info[i] != '/0')
    {
        length++;
        i++;
    }
    while (info[j] != '/0')
    {
        number = number + ( info[j] * pow(10,length) );
        j++;
        length--;
    }
    return number*sign;
}

short changeCmd(int steps, HistoryNode* firstH)
{
    int index;
    if (steps<0)
        index = lengthH + steps;
    else
        index = steps;

    getIndexNode(index,firstH);
    return isBuiltInCommand(repeatedHNode->parsedCommand[0]);                       // just the command
}
void getIndexNode(int index, HistoryNode* firstH)
{
    HistoryNode* temp = firstH;
    for (int i = 0; i <= index; ++i) {
        temp = temp->next;
    }
    repeatedHNode = temp;
}

// insert built ib command
void insertH(char** info, short cmd_num ,  HistoryNode* firstH , char* cmdLine )
{
    //if there is still place to hold history
    if (lengthH < setH)
    {
        lengthH++;
    }
    else //else we delete the first node and the next node becomes the first
    {
        firstH = firstH->next;
    }

    if (firstH == NULL)
    {
        firstH = (struct HistoryNode *) malloc(sizeof(struct HistoryNode));
        firstH-> { info, NULL, cmd_num , cmdLine };
    }
    else
    {
        HistoryNode *temp = firstH;
        while (temp->next != null)
        {
            temp = temp->next;
        }
        (temp->next) = (struct HistoryNode *) malloc(sizeof(struct HistoryNode));
        (temp->next)-> { info, NULL, cmd_num , cmdLine  };
    }
}

void removeHLink ( int num , HistoryNode* firstH)
{
    while (num > 0)
    {
        firstH = firstH->next;
        num --;
        //memory managment might need to free the pointers
    }
    lengthH = setH;
}
void printH (HistoryNode* firstH)
{
   //add index numvers to the printf
    HistoryNode *temp = firstH;
    while (temp->next != null)
    {
        printf("%s" , temp->unparsedCmd);
        temp = temp->next;
    }
}

void insertB(int pid ,BackgroundNode* firstB ,char* cmdLine )
{
    if (firstB == NULL)
    {
        firstB = (struct BackgroundNode *) malloc(sizeof(struct BackgroundNode));
        firstB-> { cmdLine,pid, NULL, NULL };
    }
    else
    {
        BackgroundNode *temp = firstB;
        BackgroundNode *last;
        while (temp->next != null)
        {
            temp = temp->next;
        }

        last = (struct BackgroundNode *) malloc(sizeof(struct BackgroundNode));
        last-> { cmdLine,pid, NULL, temp };
        temp = {.next = last};
    }
}

void removeBGLink (int pid )
{
    if ( firstB->pid == pid)
    {
        firstB = firstB->next;
        firstB->previous = NULL;
    }
    else
    {
        BackgroundNode *temp = firstB;
        BackgroundNode *prev;
        BackgroundNode *next;
        while ( temp->pid != pid )
        {
            temp = temp->next;
        }
        prev = temp->previous;
        next = temp->next;
        prev->next = next;
        next->previous = prev;
        //free(temp) we need to remove the link
    }
}

void printB()
{
    BackgroundNode *temp = firstB;
    //add a structure to the printf
    while (temp->next != null)
    {
        printf("%s" , temp->unparsedCmd);
        temp = temp->next;
    }
}