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
    printf("%s: ", "Prompt"); 
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
    if (args[0] == NULL){
        return 1;
    }
    /* List redirection elements here */
    int left = hasLeftRedirection(args);
    int right = hasRightRedirection(args);
    int append = hasAppend(args);
    int pipe = hasPipe(args);
    int background = hasAmpersand(args);
    int builtin = isBuiltin(args);

    /* printf("Left: %d\n", left); */
    /* printf("right: %d\n", right); */
    /* printf("append: %d\n", append); */
    /* printf("pipe: %d\n", pipe); */
    /* printf("background: %d\n", background); */
    /* printf("builtin: %d\n", builtin); */

    //if the user simply enters, then return nothing
    

    if (pipe){ //pipe is handled first
        printf("pipe!\n");
        return 1;
    }
    else if (builtin >= 0){ //builtin command
        printf("Built in !\n");
        return (*builtin_cmd[builtin])(args);
    }
    else{ //program invocation
        pid_t cpid;
        if ( (cpid = fork()) == -1){
            return 1;
        }
        else if (cpid == 0){ //child process
            if ((execvp(args[0], args)) < 0){
                printf("program not found!\n");
                return 0;
            }
        }
        else{
            waitpid(cpid, NULL, 0);
            return 1;
        }
        return 1;
    }
}
