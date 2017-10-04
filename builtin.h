#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

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

int numberOfInternalCmds(){
    return sizeof(builtin_cmds) / sizeof(char *);
}

int cd(char **args){
    return 1;
}
int clr(){
    system("clear");
    return 1;
}
int dir(char **args){
    return 1;
}
int print_environ(){
    int i;
    for (i = 0; __environ[i] != NULL; i++){
        printf("%s\n", __environ[i]);
    }

    return 1;
}
int echo(char **args){
    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);
    int in, out;
    int left, right;
    left = hasLeftRedirection(args);
    right = hasRightRedirection(args);
    
    if (left && right){
        if ((in = open(args[left + 1], O_RDONLY)) < 0){
            printf("Error in opening input file!\n");
            return 1;
        }
        dup2(in, STDIN_FILENO);
        if ((out = open(args[right + 1], O_WRONLY | O_TRUNC | O_CREAT)) < 0){
            printf("Error in opening output file!\n");
            return 1;
        }
        dup2(out, STDOUT_FILENO);

        char c;
        while ((c = getchar()) != EOF){
            putchar(c);
        }
        close(in);
        dup2(stdin_copy, STDIN_FILENO);
        close(stdin_copy);

        close(out);
        dup2(stdout_copy, STDOUT_FILENO);
        close(stdout_copy);
    }
    else if (left && !right){
        if ((in = open(args[left + 1], O_RDONLY)) < 0){
            printf("Error in opening input file!\n");
            return 1;
        }
        dup2(in, STDIN_FILENO);
        
        char c;
        while ((c = getchar()) != EOF){
            putchar(c);
        }

        close(in);
        dup2(stdin_copy, STDIN_FILENO);
        close(stdin_copy);
    }
    else if (!left && right){
        if ((out = open(args[right + 1], O_WRONLY | O_TRUNC | O_CREAT)) < 0){
            printf("Error in opening output file!\n");
            return 1;
        }
        dup2(out, STDOUT_FILENO);

        int i = 1;
        while (i < right){
            printf("%s ", args[i]);
            i++;
        }
        printf("\n");

        close(out);
        dup2(stdout_copy, STDOUT_FILENO);
        close(stdout_copy);
    }
    else{
        int i = 1;
        while (args[i] != NULL){
            printf("%s ", args[i]);
            i++;
        } 
        printf("\n");
    }
    return 1;
}

int help(char ** args){
    int stdout_copy = dup(STDOUT_FILENO);
    int right, out;
    int c;
    if ((right = hasRightRedirection(args))){
        if ((out = open(args[right + 1], O_WRONLY | O_TRUNC | O_CREAT, 0666)) < 0){
            printf("Error in opening output file!\n");
            return 1;
        } 
        dup2(out, STDOUT_FILENO);
        FILE * fp = fopen("readme", "r");
        while(1){
            c = fgetc(fp);
            if (feof(fp))
                break;
            printf("%c", c);
        }
        fclose(fp); 
        close(out);
        dup2(stdout_copy, STDOUT_FILENO);
        close(stdout_copy);
    } 
    else{
        FILE * fp = fopen("readme", "r");
        while(1){
            c = fgetc(fp);
            if (feof(fp))
                break;
            printf("%c", c);
        }
        fclose(fp); 
    }
    return 1;
}

int stopeverythinguntilenter(){
    char c;
    do{
       c = getchar(); 
    }while(c != '\n');
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


