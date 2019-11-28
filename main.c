#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

struct HistoryNode {
    char** parsedCommand;
    struct HistoryNode* next;
    short cmd_numH;
    char* unparsedCmd;
    struct pipedCmd* pipedCmd;
};

struct BackgroundNode {
    char*  unparsedCmd;
    int pid;
    struct BackgroundNode* next;
    struct BackgroundNode* previous;
};
struct pipedCmd
{
    char** parsedCmd;
    struct pipedCmd* next;
    char* cmdSS;
};

struct BackgroundNode* firstB = NULL;
int lengthH =0;
int setH = 10;
_Bool flagB = false;
_Bool flagH = false;
struct HistoryNode* repeatedHNode;
_Bool flagP = false;
struct pipedCmd* firstP;
void executeBuiltInCommand(short cmd_num);
short isBuiltInCommand(char* info);
//char** parseInput(char* cmdLine);
char** argumentsParser(char* input);
int rowsAmount(char input[]);
int columnLength(char* input, int begin);
_Bool isBackgroundJob(char* info);
int getHsteps(char info[]);
short changeCmd(int steps, struct HistoryNode* firstH);
void getIndexNode(int index,struct HistoryNode* firstH);
void insertH(char** info, short cmd_num ,  struct HistoryNode* firstH , char* cmdLine , struct pipedCmd* pipedP);
void insertB(int pid ,struct BackgroundNode* firstB ,char* cmdLine );
void removeBGLink (int pid );
void printB();
int isHPiped ();
int isPiped(char* cmdLine);
void pipedParsedCmd(char* cmdLine);
void runPipedCommands(int numPipes , struct BackgroundNode* firstB);
char* substring (int start , int fin , char* arr);
char* trim(char* cmdLine, char c);
void handler_bgPrcss ()
{
    int sigPid = waitpid(-1 , NULL, WNOHANG);

    if (sigPid != -1)
    {
        printf("child finished \n");
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
    cmdLine = (char *)malloc(sizeof(char)*100);
    char** info;
    int numOfPipesH = 0;
    int numbOfPipes = 0;
    struct HistoryNode* firstH = NULL;
    int stepsH=0;
    int pipesCtr = 0;
    while (1)                                                                                   // wait for user command
    {
        printf("write command: ");
        // put working directory using pwd
        fgets(cmdLine, 30, stdin);
        //getline(cmdLine, 30, stdin);
        //scanf("%s", cmdLine);
        cmdLine = trim(cmdLine, '\n');
        //printf ("input is: %s \n", cmdLine);
        cmd_num = isBuiltInCommand(cmdLine);
        printf ("cmd num is: %d \n", cmd_num);
        if (cmd_num == 7)                                                                       // repeat command
        {
            printf("repeated command \n");
            stepsH = getHsteps(cmdLine);
            cmd_num = changeCmd(stepsH, firstH);
            flagH = true;
            numOfPipesH = isHPiped();
            if (numOfPipesH)
            {
                flagP = true;
            }
        }
        if(cmd_num != 0)                                                                             // if a built-in command
        {
            printf("built in command \n");
            executeBuiltInCommand(cmd_num);
            flagH = false;
            if(cmd_num != 8)
            {
                insertH(NULL, cmd_num , firstH , cmdLine,NULL);
            }
            else
            {
                info = argumentsParser(cmdLine);
                insertH(info,cmd_num,firstH,cmdLine,NULL);
            }
        }
        else
        {
            printf("shell command \n");
            numbOfPipes = isPiped(cmdLine);
            if(numbOfPipes || flagP)                                                                   //pipe
            {
                printf("piped command \n");
                if(flagH) //repeated pipe
                {
                    firstP = repeatedHNode->pipedCmd;
                }
                else        //new pipe
                {
                    pipedParsedCmd(cmdLine);
                }
                insertH(NULL, cmd_num, firstH, cmdLine , firstP);
                if(flagH)
                {
                    pipesCtr = numOfPipesH;
                }
                else
                {
                    pipesCtr = numbOfPipes;
                }

                runPipedCommands(pipesCtr,firstB);   //execute general + insert to background linked list if flagB = 1
                firstP = NULL;
                flagP = false;
                flagH = false;
            }
            else                                                                                    //not repeated cmd
            {
                printf("regular command \n");
                //info = argumentsParser(cmdLine);
                //insertH(info, cmd_num, firstH, cmdLine, NULL);
                flagH = false;
                childPid = fork();
                if (childPid == -1) {
                    perror("fork error");                                                         // print error???
                }
                if (childPid == 0)                                                                  // child process
                {
                    printf("exec: %s \n",info[0]);
                    execvp(info[0], info);
                }
                else                                                                                // parent process
                {
                    if (isBackgroundJob(cmdLine))
                    {
                        printf("background command \n");
                        cmdLine = trim(cmdLine, '&');
                        info = argumentsParser(cmdLine);
                        insertH(info, cmd_num, firstH, cmdLine, NULL);
                        backgroundPid = fork();
                        if (backgroundPid == -1)
                        {
                            perror("fork error");                                                         // print error???
                        }
                        if (backgroundPid == 0)                                                     // background process
                        {
                            insertB(getpid(), firstB, cmdLine);
                            execvp(info[0], info);
                        }
                    }
                    else
                    {
                        printf("wait command \n");
                        info = argumentsParser(cmdLine);
                        insertH(info, cmd_num, firstH, cmdLine, NULL);
                        //waitpid(childPid,NULL,0);
                    }
                }
            }
        }
    }
}

char* trim(char* cmdLine, char c)
{
    int i=strlen(cmdLine);
    while (cmdLine[i] != c)
    {
        i--;
    }
    //printf("trimmed: %s .",substring(0, i-1, cmdLine));
    return substring(0, i-1, cmdLine);
}

////parse input to char array
//char** parseInput(char* cmdLine)
//{
//    char** args;
//    if (flagH)                                          // repeated command
//    {
//        args = repeatedHNode->parsedCommand;
//        return args;
//    }
//    else
//    {
//        int i=0;                                                                                    // run over input
//        int j=0;                                                                                    // run over argument
//        int k=0;                                                                                    // run over returned array
//
//        // find amount of arguments
//        k = rowsAmount(cmdLine);
//        printf("k: %d \n", k);
//        args = (char **) malloc(sizeof(char) * (k));
//
//        i=0;
//        int rows=0;
//        // find length of every argument
//        while (cmdLine[i] != '\0' && rows<k)
//        {
//            printf("inside parser \n");
//            j = columnLength(cmdLine, i);
//            printf("j: %d \n", j);
//            args[rows] = (char *) malloc(sizeof(char) * (j + 1));
//            rows++;
//            if (cmdLine[j+1] == '\0')                                                                  // if input is finished
//                break;
//            i=j+1;                                                                                   // next char that is not a space (2???)
//        }
//        args = argumentsParser(cmdLine);
//        return args;
//    }
//
//}

// if a built-in command, return a non-zero short
short isBuiltInCommand(char* info)
{
    //printf ("inside isBuiltInCommand function \n");
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
    else if (info[0] == '!')
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
            printf ("execute jobs \n");
            //executeJobs();
            break;
        case 2 :
            printf ("execute cd \n");
            //executeCd();
            //chdir
            break;
        case 3 :
            printf ("execute history \n");
            //executeHistory();
            break;
        case 4 :
            printf ("execute exit \n");
            //executeExit();
            //check the background is empty
            break;
        case 5 :
            printf ("execute kill \n");
            //executeKill();
            //kill
            break;
        case 8 :
            printf ("execute history -s \n");
            //if set > length we just change set, else we remove nodes
            //executeHistorySizeSet()
            break;
        case 6 :
            printf ("execute help \n");
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
    int i=0;                                                                                    // run over input
    int k=0;                                                                                    // run over returned array
    // find amount of arguments
    while (input[i] != '\0')
    {
        if (input[i] == ' ' && input[i+1] != ' ')
            k++;
        i++;
    }
    return k+1;
}

int columnLength(char* input, int begin)
{
    //printf("inside columnLength, input: %s, begin: %d \n", input, begin);
    int i=0;
    //printf("input in place %d: %c \n",begin+i,input[begin+i]);
    while (input[begin+i] != '\0' && (input[begin+i] != ' '))
    {
        i++;
    }
    //printf("return i: %d \n", i);
    return i;
}


//arguments parser
char** argumentsParser(char* input)
{
    //printf("input: %s \n",input);
    char** args;
    int i=0;                                                                                    // run over input
    int j=0;                                                                                    // run over argument
    int k=0;                                                                                    // run over returned array

    // find amount of arguments
    k = rowsAmount(input);
    //printf("k: %d \n", k);
    args = (char **) malloc(sizeof(char) * (k));

    i=0;
    int rows=0;
    // find length of every argument
    while (input[i] != '\0' && rows<k)
    {
        //printf("inside argument parser \n");
        j = columnLength(input, i);
        //printf("j: %d \n",j);
        args[rows] = (char *) malloc(sizeof(char) * (j + 1));
        rows++;
        if (input[i+j+1] == '\0')                                                                  // if input is finished
            break;
        i=i+j+1;                                                                                   // next char that is not a space (2???)
    }

    i=0;                                                                                    // run over input
    j=0;                                                                                    // run over argument
    k=0;                                                                                    // run over returned array
    while (input[i] != '\0')
    {
        int length = columnLength(input, i);
        //printf("length: %d \n",length);
        while (input[i+j] != ' ' && input[i+j] != '\0' &&j<length+1)
        {
            printf("args[%d][%d]: %c \n",k,j, input[i+j]);
            args[k][j] = input [i+j];
            j++;
        }
        //printf("finish: %c \n",'\0');
        args[k][length] = '\0';
        printf("args[%d][%d]: %c \n",k,length,args[k][length]);
        k++;
        j=0;
        if (input[i+j+1] == '\0')                                                                  // if input is finished
            break;
        i=i+length+1;                                                                                   // next char that is not a space
    }



//    i=0;                                                                                         // run over input
//    int column = 0;                                                                                        // run over argument
//    int rows2=0;                                                                                         // run over returned array
//    while (input[i] != '\0' && rows2<k)
//    {
//        j = columnLength(input, i);
//        //printf("j: %d \n",j);
//        while (input[i+column] != ' ' && column<j+1)
//        {
//            //j = columnLength(input, i+column);
//            printf("args[%d][%d]: %c \n",rows2,column, input[i+column]);
//            args[rows2][column] = input [i+column];
//            column++;
//        }
//        column=0;
//        //printf("out of while \n");
//        args[rows2][j] = '\0';
//        rows2++;
//        if (input[i+j+1] == '\0')                                                                  // if input is finished
//            break;
//        i=i+j+1;                                                                                   // next char that is not a space
//    }
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
//    while (info[i] != '\0')
//    {
//        j = columnLength(info, i);
//        args[k] = (char *) malloc(sizeof(char) * (j + 1));
//        k++;
//        if (info[j+1] == '\0')                                                                  // if input is finished
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
    while (cmdLine[i] != '\0')
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
    int sign =1;
    if (info[1] == '-')
    {
        sign = -1;
        i++;
    }
    int j= i;
    while (info[i] != '\0')
    {
        length++;
        i++;
    }
    while (info[j] != '\0')
    {
        int pow = length;
        int strength = 1;
        while (pow > 0)
        {
            strength = strength*10;
            pow--;
        }
        //number = number + ( info[j] * pow(10,length) );
        number = number + ( info[j] * strength );
        j++;
        length--;
    }
    return number*sign;
}

short changeCmd(int steps, struct HistoryNode* firstH)
{
    int index;
    if (steps<0)
        index = lengthH + steps;
    else
        index = steps;

    getIndexNode(index,firstH);
    return isBuiltInCommand(repeatedHNode->parsedCommand[0]);                       // just the command
}
void getIndexNode(int index,struct HistoryNode* firstH)
{
    struct HistoryNode* temp = firstH;
    for (int i = 0; i <= index; ++i) {
        temp = temp->next;
    }
    repeatedHNode = temp;
}

// insert built ib command
void insertH(char** info, short cmd_num ,  struct HistoryNode* firstH , char* cmdLine , struct pipedCmd* pipedP)
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
        *firstH = (struct HistoryNode){info, NULL, cmd_num , cmdLine , pipedP};
    }
    else
    {
        struct HistoryNode *temp = firstH;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }
        (temp->next) = (struct HistoryNode *) malloc(sizeof(struct HistoryNode));
        *(temp->next) = (struct HistoryNode) {info, NULL, cmd_num , cmdLine ,pipedP };
    }
}

void removeHLink ( int num , struct HistoryNode* firstH)
{
    while (num > 0)
    {
        firstH = firstH->next;
        num --;
        //memory managment might need to free the pointers
    }
    lengthH = setH;
}
void printH (struct HistoryNode* firstH)
{
    //add index numvers to the printf
    struct HistoryNode *temp = firstH;
    while (temp->next != NULL)
    {
        printf("%s" , temp->unparsedCmd);
        temp = temp->next;
    }
}

void insertB(int pid ,struct BackgroundNode* firstB ,char* cmdLine )
{
    if (firstB == NULL)
    {
        firstB = (struct BackgroundNode *) malloc(sizeof(struct BackgroundNode));
        *firstB = (struct BackgroundNode){ cmdLine,pid, NULL, NULL };
    }
    else
    {
        struct BackgroundNode *temp = firstB;
        struct BackgroundNode *last;
        while (temp->next != NULL)
        {
            temp = temp->next;
        }

        last = (struct BackgroundNode *) malloc(sizeof(struct BackgroundNode));
        *last = (struct BackgroundNode){ cmdLine,pid, NULL, temp };
        *temp = (struct BackgroundNode){.next = last};
    }
}

void removeBGLink (int pid )
{
    _Bool flag = false;
    if(firstB != NULL)
    {
        if ( firstB->pid == pid)
        {
            firstB = firstB->next;
            firstB->previous = NULL;
        }
        else
        {
            struct BackgroundNode *temp = firstB;
            struct BackgroundNode *prev;
            struct BackgroundNode *next;
            while ( temp->pid != pid )
            {
                temp = temp->next;
                if (temp->pid == pid)
                {
                    flag = true;
                }
            }
            if(flag)
            {
                prev = temp->previous;
                next = temp->next;
                prev->next = next;
                next->previous = prev;
                //free(temp) we need to remove the link
            }
        }
    }
}

void printB()
{
    struct BackgroundNode *temp = firstB;
    //add a structure to the printf
    while (temp->next != NULL)
    {
        printf("%s" , temp->unparsedCmd);
        temp = temp->next;
    }
}

//returns the number of pipes in a cmd
int isPiped(char* cmdLine)
{
    int counter = 0;
    int i = 0;
    while(cmdLine[i] != '\0')
    {
        if (cmdLine[i] == '|')
        {
            counter++;
        }
        i++;
    }
    return counter;
}


//returns the number of pipes in a cmd
int isHPiped ()
{
    int counter = 0;
    struct pipedCmd * tmp = repeatedHNode->pipedCmd;
    if (repeatedHNode->pipedCmd != NULL)
    {
        while(tmp != NULL)
        {
            counter++;
            tmp = tmp->next;
        }
    }
    return counter;
}

void pipedParsedCmd(char* cmdLine)
{
    int i = 0;
    int j = 0;
    char** tmp;
    while(cmdLine[i] != '\0')
    {
        while(cmdLine[i] != '|')
        {
            i++;
        }
        printf("sunstring: %s \n",substring(j , i-1 , cmdLine));
        tmp = argumentsParser(substring(j , i-1 , cmdLine));
        if (firstP == NULL)
        {
            firstP = (struct pipedCmd*) malloc(sizeof(struct pipedCmd));
            *firstP = (struct pipedCmd) {tmp, NULL , substring(j , i , cmdLine)};
        }
        else
        {
            struct pipedCmd *tmpP = firstP;
            while (tmpP->next != NULL)
            {
                tmpP = tmpP->next;
            }
            (tmpP->next) = (struct pipedCmd*) malloc(sizeof(struct pipedCmd));
            *(tmpP->next) = (struct pipedCmd){tmp, NULL,substring(j , i , cmdLine)};
        }
        i += 2;
        j = i;
    }
}

char* substring (int start , int fin , char* arr)
{
    int i = fin - start ;
    char* tmp = (char *)malloc(sizeof(char)*(i+1));
    while(i >= 0)
    {
        tmp[i] = arr[start + i];
        i--;
    }
    return tmp;
}

void runPipedCommands(int numPipes, struct BackgroundNode* firstB)
{
    struct pipedCmd* command = firstP;
    int status;
    int i = 0;
    int pid;
    int pipefds[2*numPipes];

    for(i = 0; i < (numPipes); i++)
    {
        if(pipe(pipefds + i*2) < 0)
        {
            perror("couldn't pipe");
            exit(EXIT_FAILURE);
        }
    }
    int j = 0;
    while(command)
    {
        pid = fork();
        if(pid == 0)
        {
            //if not last command
            if(command->next)
            {
                if(dup2(pipefds[j + 1], 1) < 0)
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            //if not first command&& j!= 2*numPipes
            if(j != 0 )
            {
                if(dup2(pipefds[j-2], 0) < 0)
                {
                    perror(" dup2");///j-2 0 j+1 1
                    exit(EXIT_FAILURE);

                }
            }
            for(i = 0; i < 2*numPipes; i++)
            {
                close(pipefds[i]);
            }

            if( execvp(command->parsedCmd[0], command->parsedCmd) < 0 )
            {
                perror(command->cmdSS);
                exit(EXIT_FAILURE);
            }
            if(flagB)
            {
                insertB(pid,firstB , command->cmdSS );
            }
        }
        else if(pid < 0)
        {
            perror("error");
            exit(EXIT_FAILURE);
        }
        else if (pid >0)
        {
            command = command->next;
            j+=2;
        }
    }
    /**Parent closes the pipes and wait for children*/
    for(i = 0; i < 2 * numPipes; i++)
    {
        close(pipefds[i]);
    }
    if(flagB)
    {
        for(i = 0; i < numPipes + 1; i++)
            wait(&status);
    }

}