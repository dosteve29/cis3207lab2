#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include "builtin.h"

void printPrompt();
char * readLine();
char ** parseLine(char * line);
int doStuff(char ** args);

extern char ** environ;

int numberOfArguments;

int main(int argc, char ** argv){
    char * line;
    char ** args;
    int status = 1;

    if (putenv("SHELL=/home/steve/cis3207lab2/myshell")){
        printf("putenv() error\n");
    }
    if (putenv("PARENT=/home/steve/cis3207lab2/")){
        printf("putenv() error\n");
    }

    if (argc > 1){
        printf("There is batchfile!\n");
        return 0;
    }

    while(status){
        printPrompt();
        line = readLine();
        args = parseLine(line);
        status = doStuff(args);

        free(line);
        free(args);
    }

    return 0;
}

void printPrompt(){
    char dirName[1024];
    getcwd(dirName, sizeof(dirName));
    printf("%s%c ", dirName, '>');
}

char * readLine(){
    char * line = NULL;
    size_t buf = 0;
    getline(&line, &buf, stdin);
    return line;
}

char ** parseLine(char * line){
    char **tokens = malloc(512 * sizeof(char*));
    char * token;
    char delimit[3];
    delimit[0] = ' ';
    delimit[1] = '\n';
    int position = 0;

    token = strtok(line, delimit);

    while ( token != NULL){
        tokens[position] = token;
        position++;

        token = strtok(NULL, delimit);
    }
    tokens[position] = NULL;
    numberOfArguments = position;
    return tokens;
}

int doStuff(char ** args){
    //if the user simply enters, then return nothing
    if (args[0] == NULL){
        return 1;
    }

    /* List redirection elements here */
    int left = hasLeftRedirection(args);
    int right = hasRightRedirection(args);
    int append = hasAppend(args);
    int bar = hasPipe(args);
    int background = hasAmpersand(args);


    if (bar){ //pipe is handled first
        printf("Pipe: %d\n", bar);
        char * leftArgs[bar + 1];
        char * rightArgs[numberOfArguments - bar];
        memset(leftArgs, '\0', sizeof(leftArgs));
        memset(rightArgs, '\0', sizeof(rightArgs));

        int i;
        for (i = 0; i < bar; i++){
            leftArgs[i] = args[i];
        }  
        for (i = bar + 1; i < numberOfArguments; i++){
            rightArgs[i - bar - 1] = args[i];
        }

        /* testing for correct left and right args */
        printf("Left: ");
        for (i = 0; i < bar; i++){
            printf("%s ", leftArgs[i]);
        }
        printf("\n");
        printf("Right: ");
        for (i = 0; i < numberOfArguments - bar - 1; i++){
            printf("%s ", rightArgs[i]);
        }
        printf("\n");
        
        int fd[2];
        pid_t left;
        pid_t right;

        if (pipe(fd) == -1){
            printf("Error creating pipe!\n");
            return 0;
        }
        
        if ((left = fork()) == -1){
            printf("Error in fork()1!\n");
            return 0;
        }
        else if (left == 0){ //left writes to the pipe
            close(STDOUT_FILENO);
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            int builtin = isBuiltin(leftArgs);
            if (builtin >= 0){
                builtin_cmd[builtin](leftArgs);
                exit(0);
            }
            else{
                execvp(leftArgs[0], leftArgs);
                printf("Left Program: %s does not exist!\n", leftArgs[0]);
                exit(0);
            } 
        }
        else{
            if ((right = fork()) == -1){
                printf("Error in fork()2!\n");
                return 0;
            }
            else if (right == 0){
                close(STDIN_FILENO);
                dup2(fd[0], STDIN_FILENO);
                close(fd[1]);
                execvp(rightArgs[0], rightArgs);
                printf("Right Program: %s not found!\n", rightArgs[0]);
                exit(0);
            }
            else{ //shell process
                waitpid(left, NULL, 0);
                close(fd[1]);
                waitpid(right, NULL, 0);
                close(fd[0]);
                return 1;
            }
        }
    }
    else if(left || right || append){
        printf("Left: %d\n", left);
        printf("Right: %d\n", right);
        printf("Append: %d\n", append);

        
    }
    else if (background){
        printf("Background: %d\n", background);
        int builtin = isBuiltin(args);
        if (builtin >= 0){
            return (*builtin_cmd[builtin])(args); 
        }
        else{
            pid_t pid;

            if ((pid = fork()) < 0){
                printf("Fork failed!\n");
                exit(0);
            }
            else if (pid == 0){
                execvp(args[0], args);
                printf("No program found!\n");
            }
            else{
                return 1;
            }
        }
    }
    else{
        int builtin = isBuiltin(args);
        if (builtin >= 0){
            return (*builtin_cmd[builtin])(args);
        }
        else{
            pid_t pid;

            if ((pid = fork()) < 0){
                printf("Fork failed!\n");
                exit(0);
            }
            else if (pid == 0){
                execvp(args[0], args);
                printf("No program found!\n");    
                exit(0);
            }
            else{
                waitpid(pid, NULL, 0);
                return 1;
            }
        }
    }
    return 1;
}
