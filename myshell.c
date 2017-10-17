//libraries used
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include "builtin.h"

//function prototypes
void printPrompt();
char * readLine();
char ** parseLine(char * line);
int doStuff(char ** args);

//get the environ from the calling process
extern char ** environ;

//global variable to indicate number of arguments from batchfile or user
int numberOfArguments;

//main
int main(int argc, char ** argv){
    //required variables for shell processing
    char * line = NULL; //line is the sentence by user or file until \n
    char ** args; //args, which are tokenized from line
    int status = 1; //status of the shell

    //setting environment variables for portability
    char shellName[1024];
    char parent[1024];
    char pathedit[sizeof(size_t)];

    //set all the array elements to '\0'
    memset(parent, '\0', sizeof(parent));
    memset(shellName, '\0', sizeof(shellName));
    memset(pathedit, '\0', sizeof(pathedit));

    //set the parent variable to the current working directory (where myshell executable is)
    getcwd(parent, sizeof(parent));
    setenv("PARENT", parent, 1);

    //myshell is appended after the current working directory
    strcpy(shellName, parent);
    strcat(shellName, "/myshell");

    //set the SHELL environment variable
    setenv("SHELL", shellName, 1);

    //get the PATH and append the current working directory so that executables within the directory will execute
    strcpy(pathedit, getenv("PATH"));
    strcat(pathedit, ":");
    strcat(pathedit, parent);

    //set the PATH variable with the new pathedit variable
    setenv("PATH", pathedit, 1);

    //if there is batchfile: ./myshell batchfile
    if (argc > 1){
        FILE * fp = fopen(argv[1], "r"); //open batchfile next to ./myshell
        char cmdsFromFile[256] = ""; //this is similar to line variable in main

        while(fgets(cmdsFromFile, sizeof(cmdsFromFile), fp)){ //read a line
            printf("%s", cmdsFromFile);  //print the line
            args = parseLine(cmdsFromFile); //get the arguments
            doStuff(args); //doStuff

            free(args); //free the arguments
            /* memset(cmdsFromFile, '\0', sizeof(cmdsFromFile)); */
        }
        fclose(fp); //close the file pointer
        exit(0); //exit
    }

    //this is without batchfile
    while(status){
        printPrompt(); //print the prompt
        line = readLine(); //read a line
        args = parseLine(line); //get the arguments
        status = doStuff(args); //do stuff and get status

        free(line); //free line
        free(args); //free args
    }

    return 0;
}

//print the current working directory
void printPrompt(){
    char dirName[1024];
    getcwd(dirName, sizeof(dirName));
    printf("%s%c ", dirName, '>');
}

//read line from standard input until \n
char * readLine(){
    char * line = NULL;
    size_t buf = 0;
    getline(&line, &buf, stdin);
    return line;
}

//parse the line into tokens
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
    numberOfArguments = position; //get the number of arguments into global variable
    return tokens;
}

//process the arguments
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
        char * leftArgs[bar + 1]; //arguments on the left side of the pipe
        char * rightArgs[numberOfArguments - bar]; //arguments on the right side of the pipe
        memset(leftArgs, '\0', sizeof(leftArgs)); //set all of array to '\0'
        memset(rightArgs, '\0', sizeof(rightArgs));

        //get appropriate arguments from args
        int i;
        for (i = 0; i < bar; i++){
            leftArgs[i] = args[i];
        }  
        for (i = bar + 1; i < numberOfArguments; i++){
            rightArgs[i - bar - 1] = args[i];
        }

        //this is the pipe
        int fd[2];
        //this is the child processes
        pid_t left;
        pid_t right;

        //check for pipe error
        if (pipe(fd) == -1){
            printf("Error creating pipe!\n");
            return 0;
        }
        
        //check for fork error in first fork
        if ((left = fork()) == -1){
            printf("Error in fork()1!\n");
            return 0;
        }
        else if (left == 0){ //left writes to the pipe
            close(STDOUT_FILENO); //close the standard output
            dup2(fd[1], STDOUT_FILENO); //set write-end of pipe as standard output
            close(fd[0]); //close the read-end of pipe
            int builtin = isBuiltin(leftArgs); //check if command is built-in
            if (builtin >= 0){ //if it is
                builtin_cmd[builtin](leftArgs); //use built-in commands to exec leftArgs
                exit(0); //then exit the child process
            }
            else{ //if not built-in cmd
                execvp(leftArgs[0], leftArgs); //exec the program in leftArgs
                printf("Left Program: %s does not exist!\n", leftArgs[0]); //checking for non-existent programs
                exit(0); //if the program does not exist, then exit the child process
            } 
        }
        else{ //this goes to the right side of the pipe
            if ((right = fork()) == -1){ //check for fork error
                printf("Error in fork()2!\n");
                return 0;
            }
            else if (right == 0){ //this is the second child process
                close(STDIN_FILENO); //close the standard input
                dup2(fd[0], STDIN_FILENO); //set read-end of pipe as standard input
                close(fd[1]); //close the write-end of pipe
                //since built-in commands do not support stdin redirection,
                //all commands on the right side of the pipe is exec or external programs
                execvp(rightArgs[0], rightArgs);
                printf("Right Program: %s not found!\n", rightArgs[0]); //error checking
                exit(0);
            }
            else{ //shell process
                waitpid(left, NULL, 0); //wait for the process to end
                close(fd[1]); //close the write-end of pipe
                waitpid(right, NULL, 0); //wait for the process to end
                close(fd[0]); //close the read-end of pipe
                return 1; //return success status
            }
        }
    }
    //this is for <, >, and >> redirection
    else if(left || right || append){
        int inputfile;  //inputfile from <
        int outputfile; //outputfile for > or >>
        int stdin_copy = dup(STDIN_FILENO); //backup stdin and stdout
        int stdout_copy = dup(STDOUT_FILENO);
        char * newArgs[numberOfArguments]; //prepare space for appropriate arguments
        memset(newArgs, '\0', sizeof(newArgs)); //set array spaces as null

        if (left){ //if there is <
            if((inputfile = open(args[left + 1], O_RDONLY, 0777)) < 0){ //open the file
                printf("Error in opening input file!\n"); //error checking
                return 1;
            }

            //get the newArgs from beginning of args to <
            int i;
            for (i = 0; i < left; i++){
                newArgs[i] = args[i];
            }

            dup2(inputfile, STDIN_FILENO); //change standard input to the inputfile
        }
        if (right){ //if there is >
            if ((outputfile = open(args[right + 1], O_TRUNC | O_CREAT | O_WRONLY, 0777)) < 0){ //open the outputfile
                printf("Error in opening output file for truncating!\n"); //error checking
                return 1;
            }                

            dup2(outputfile, STDOUT_FILENO); //set outputfile as standard output

            //get the newArgs with conditions
            if (left){ //there is < also. do nothing
            }
            else{ //there is no <. get newArgs from beginning of args to >
                int i;
                for (i = 0; i < right; i++){
                    newArgs[i] = args[i];
                }
            }

        }
        else if(append){ //or there is >>
            if ((outputfile = open(args[append + 1], O_APPEND | O_CREAT | O_WRONLY, 0777)) < 0){ //open the outputfile for append
                printf("Error in opening output file for appending!\n"); //error checking
                return 1;
            }

            dup2(outputfile, STDOUT_FILENO); //set outputfile as standard output

            //get the newArgs with conditions
            if (left){ //there is < also. do nothing
            }
            else{ //there is no <. get newArgs from beginning of args to >>
                int i;
                for (i = 0; i < append; i++){
                    newArgs[i] = args[i];
                }
            }

        }

        //check if built-in command from newArgs
        int builtin = isBuiltin(newArgs);
        if (builtin >= 0){ //if the prog is builtin
            //launch the built-in command
            builtin_cmd[builtin](newArgs);
            close(inputfile); //close the inputfile
            dup2(stdin_copy, STDIN_FILENO); //restore the backup stdin
            close(stdin_copy);

            close(outputfile); //cose the outputfile
            dup2(stdout_copy, STDOUT_FILENO); //restore the backup stdout
            close(stdout_copy);
            return 1;
        }
        else{ //else, use fork and exec
            pid_t pid;

            if ((pid = fork()) < 0){
                printf("Fork failed!\n");
                exit(0);
            }
            else if (pid == 0){ //child process
                execvp(newArgs[0], newArgs); //exec with newArgs here
                printf("Program not found!\n"); //error checking
                exit(0);  
            }
            else{ //parent process
                close(inputfile); //close inputfile
                dup2(stdin_copy, STDIN_FILENO); //restore the backup stdin
                close(stdin_copy);

                close(outputfile); //close outputfile
                dup2(stdout_copy, STDOUT_FILENO); //restore the backup stdout
                close(stdout_copy);
       
                waitpid(pid, NULL, 0); //wait for the child process to end
            }
        }
    }
    //background & 
    else if (background){
        char * newArgs[numberOfArguments]; //prepare space for appropriate arguments
        memset(newArgs, '\0', sizeof(newArgs)); //set array spaces as null
        int i;
        for (i = 0; i < background; i++){
            newArgs[i] = args[i];
        }

        //if built-in command
        int builtin = isBuiltin(newArgs);
        //launch the built-in command
        if (builtin >= 0){
            int stdout_copy = dup(STDOUT_FILENO); //save stdout
            close(STDOUT_FILENO); //close stdout
            builtin_cmd[builtin](newArgs); //
            dup2(stdout_copy, STDOUT_FILENO); //restore the stdout
            close(stdout_copy); //close stdout
            return 1;
        }
        else{ //if not built-in, use fork and exec
            pid_t pid;
            int tempFile; //the background process will write to this file

            if ((pid = fork()) < 0){
                printf("Fork failed!\n");
                exit(0);
            }
            else if (pid == 0){ //child process
                tempFile = open("tempfile", O_CREAT | O_WRONLY, 0777); //create and write to the file with correct permissions
                close(STDOUT_FILENO); //close stdout so nothing prints to shell
                dup2(tempFile, STDOUT_FILENO); //make the stdout to this file
                execvp(args[0], newArgs); //launch the program
                printf("No program found!\n");
            }
            else{
                //don't wait for the process to end
                return 1;
            }
        }
    }
    //no redirection
    else{
        //check if built-in command
        int builtin = isBuiltin(args);
        if (builtin >= 0){ //if it is, launch it
            return (*builtin_cmd[builtin])(args);
        }
        else{ //if not, fork and exec to launch the program
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
                waitpid(pid, NULL, 0); //wait for the process to end
                return 1;
            }
        }
    }
    return 1;
}
