#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "builtin.h"

void printPrompt();
char * readLine();
char ** parseLine(char * line);
int doStuff(char ** args);

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
    char **tokens = malloc(1024 * sizeof(char*));
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
    int i;
    if (args[0] == NULL){
        return 1;
    }

    if (hasPipe(args)){
        printf("There is pipe!\n");
        return 1;
    }

    for (i = 0; i < numberOfInternalCmds(); i++){
        if (strcmp(args[0], builtin_cmds[i]) == 0){
            return (*builtin_cmd[i])(args);
        }
    }
    return 1;
}
