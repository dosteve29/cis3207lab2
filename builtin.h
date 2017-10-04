#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int cd(char **args);
int clr();
int dir(char **args);
int environ();
int echo(char **args);
int help();
int stopeverythinguntilenter();
int quit();
int hasLeftRedirection(char ** args);
int hasRightRedirection(char ** args);

char *builtin_cmds[] = {
    "cd",
    "clr",
    "dir",
    "environ",
    "echo",
    "help",
    "pause",
    "quit"
};

int (*builtin_cmd[])(char **) = {
    &cd,
    &clr,
    &dir,
    &environ,
    &echo,
    &help,
    &stopeverythinguntilenter,
    &quit    
};

int numberOfInternalCmds(){
    return sizeof(builtin_cmds) / sizeof(char *);
}

int cd(char **args){
    return 1;
}
int clr(){
    return 1;
}
int dir(char **args){
    return 1;
}
int environ(){
    return 1;
}
int echo(char **args){
    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);
    int in, out;
    int left, right;
    left = hasLeftRedirection(args);
    right = hasRightRedirection(args);
    if (left > 0){
        in = open("inputfile", O_RDONLY);
        if (in < 0)
            printf("Error!\n");
        dup2(in, STDIN_FILENO);
        close(in);
        dup2(stdin_copy, 0);
        close(stdin_copy);
    }
    if (right > 0){
        out = open("outputfile", O_WRONLY | O_TRUNC | O_CREAT);
        if (out < 0)
            printf("Error!\n");
        dup2(out, STDOUT_FILENO);
        close(out);
        dup2(stdout_copy, 1);
        close(stdout_copy);
    }
    int i = 1;
    while (args[i] != NULL){
        printf("%s ", args[i]);
        i++;
    }
    
    printf("\n");
    return 1;
}
int help(){
    return 1;
}
int stopeverythinguntilenter(){
    return 1;
}

int quit(){
    return 0;
}

int hasLeftRedirection(char ** args){
    int i = 1;
    while (args[i] != NULL){
        if (strcmp(args[i], "<") == 0)
            return i;
        i++;
    }

    return -1;
}

int hasRightRedirection(char ** args){
    int i = 1;
    while (args[i] != NULL){
        if (strcmp(args[i], ">") == 0)
            return i;
        i++;
    }

    return -1;

}
