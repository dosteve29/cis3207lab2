#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void printPrompt();
char * readLine();

int main(int argc, char ** argv){
    char * line;

    while(1){
        printPrompt();
        line = readLine();
        printf("%s", line);
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
