// libraries used
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

//function prototypes
int cd(char **args);
int clr();
int dir(char **args);
int print_environ();
int echo(char **args);
int help(char **args);
int stopeverythinguntilenter();
int quit();
int hasLeftRedirection(char ** args);
int hasRightRedirection(char ** args);

//array of strings
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

//array of pointers to functions with args as parameter
int (*builtin_cmd[])(char ** args) = {
    &cd,
    &clr,
    &dir,
    &print_environ,
    &echo,
    &help,
    &stopeverythinguntilenter,
    &quit    
};

//return number of internal commands
int numberOfInternalCmds(){
    return sizeof(builtin_cmds) / sizeof(char *);
}

//change directory
int cd(char **args){
    char dirName[1024]; //name of directory
    if (args[1] == NULL){ //no argument. 
        if (getcwd(dirName, sizeof(dirName)) == NULL){ //error checking
            printf("Error in getcwd()!\n");
            return 1;
        }
        //notify the user of no directory change 
        printf("No directory change\n");
        printf("Current working directory: %s\n", dirName);
    }
    else{ //there is argument
        strcpy(dirName, args[1]); //get the directory name from arg
        if (chdir(dirName) < 0){ //change directory and error checking for existence of directory
            printf("Directory does not exist!\n");
            return 1;
        }
        if (getcwd(dirName, sizeof(dirName)) == NULL){ //error checking in getcwd
            printf("Erro in getcwd()!\n");
            return 1;
        }
        if (setenv("PWD", dirName, 1) < 0){ //setenv error checking
            printf("setenv() error!\n");
        }
    }
    return 1;
}

//clear the screen
int clr(){
    system("clear");
    return 1;
}

//list contents of directory
int dir(char **args){
    DIR * dir;
    struct dirent * dent;
    char dirName[1024]; //directory name
    if (args[1] == NULL){ //no argument. print current directory
        if (getcwd(dirName, sizeof(dirName)) == NULL){
            printf("Error in getcwd()!\n"); 
            return 1;
        }
        dir = opendir(dirName); //open the directory
        while ( (dent=readdir(dir)) != NULL){ //while there is no more to read
            if ( !strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")){ //skip . or ..
            }
            else{ //print the contents
                printf(dent->d_name);
                printf("\n");
            }
        } 
        closedir(dir); //close the directory 
    }
    else{ //there is argument. different directory. do the same as above.
        strcpy(dirName, args[1]);
        if ( (dir = opendir(dirName)) == NULL){
            printf("No directory exists!\n");
            closedir(dir);
            return 1;
        }
        while ( (dent=readdir(dir)) != NULL){
            if ( !strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")){
            }
            else{
                printf(dent->d_name);
                printf("\n");
            }
        }
        closedir(dir);
    }
    return 1;
}

//print the global, external variable environ
int print_environ(){
    int i;
    for (i = 0; __environ[i] != NULL; i++){
        printf("%s\n", __environ[i]);
    }

    return 1;
}

//literally echo the arguments
int echo(char **args){
    int i = 1;
    while (args[i] != NULL){
        printf("%s ", args[i]);
        i++;
    } 
    printf("\n");
    return 1;
}

//print the readme file 
int help(char ** args){
    //get the file location to helpDir
    char helpDir[1024];
    char * directory = getenv("PARENT");
    char * helpfile = "/readme";
    memset(helpDir, '\0', sizeof(helpDir));
    strcpy(helpDir, directory);
    strcat(helpDir, helpfile);

    //open the file
    FILE * fp = fopen(helpDir, "r");
    if (fp == NULL){
        printf("Error opening the file!\n");
        return 1;
    }

    //print the readme. the professor has instructed the class to just print the file
    char c;
    while ((c = fgetc(fp)) != EOF){
        printf("%c", c);
    }
    
    //close the file pointer
    fclose(fp);
    return 1;
}

//until enter is entered, do nothing in the shell
int stopeverythinguntilenter(){
    char c;
    do{
       c = getchar(); 
    }while(c != '\n');
    return 1;
}

//exit the shell
int quit(){
    return 0;
}

/*
 * The next shell of helper functions are used
 * to determine whether the argument has redirection elements
 * such as <, >, >>, &, |
 * also, to determine if the command is built-in or not
 */
int hasLeftRedirection(char ** args){
    int i = 1;
    while (args[i] != NULL){
        if (strcmp(args[i], "<") == 0)
            return i;
        i++;
    }

    return 0;
}

int hasRightRedirection(char ** args){
    int i = 1;
    while (args[i] != NULL){
        if (strcmp(args[i], ">") == 0)
            return i;
        i++;
    }

    return 0;

}

int hasAppend(char ** args){
    int i = 1;
    while (args[i] != NULL){
        if (strcmp(args[i], ">>") == 0)
            return i;
        i++;
    }

    return 0;

}

int hasPipe(char ** args){
    int i = 1;
    while (args[i] != NULL){
        if (strcmp(args[i], "|") == 0)
            return i;
        i++;
    }

    return 0;
}

int hasAmpersand(char ** args){
    int i = 1;
    while (args[i] != NULL){
        if (strcmp(args[i], "&") == 0)
            return i;
        i++;
    }

    return 0;
}

int isBuiltin(char **args){
    int i = 0;
    for (i = 0; i < numberOfInternalCmds(); i++){
        if (strcmp(args[0], builtin_cmds[i]) == 0){
            return i;
        }
    }

    return -1;
}
