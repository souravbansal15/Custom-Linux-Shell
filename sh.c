#include <stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h>
#include<limits.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include<sys/wait.h> 
#include <fcntl.h>

#define clear() printf("\033[H\033[J");
typedef enum {true, false} bool;
void parseExe(char command[]);

void start_shell(){
    //clear the screen;
    clear();
    printf("\n--------------------------------------------------------------\n");
    printf("|                       Linux Shell in C                     |\n");
    printf("|             --By Sourav Bansal (2018CS50421)               |\n");
    printf("--------------------------------------------------------------\n");    
}
void prompt(){
    //current directory
    char current[1000];
    getcwd(current, sizeof(current));
    printf("%s", current);
    //prompt
    printf(": shell> ");
}
int get_input(char* input){
    scanf("%[^\n]",input);;
    getchar();//the next character
    if(strlen(input)==0){
        return 0;
    }else{
        return 1;
    }
}
void call_pwd(){
    // pwd command
    printf("Current Diectory Adress:\n");
    char current[1000];
    getcwd(current, sizeof(current));
    printf("%s\n", current);
}
void call_cd(char **word){
    // cd command
    if(chdir(word[1])==-1){
        printf("No such directory exists\n");
    }else{
        printf("Directory Changes\n");
    }
}
void call_mkdir(char **word, int input_length){
    // mkdir command
    for(int i=1;i<input_length;i++){
        if(mkdir(word[i], 0777)==-1){
            printf("'%s' directory already exists or can not be created\n", word[i]);
        }else{
            printf("Directory '%s' created successfully\n", word[i]);
        }
    }
}
void call_rmdir(char **word, int input_length){
    // rmdir command
    for(int i=1;i<input_length;i++){
        if(rmdir(word[i])==-1){
            printf("'%s' directory does not exist or can not be deleted\n", word[i]);
        }else{
            printf("Directory '%s' deleted successfully\n", word[i]);
        }
    }
}
void call_exit(){
    // exit command
    printf("Exiting Shell\n");
    exit(0);
    exit(1);
}

void runcmd(char **word, int modeI, int modeO, char *infile, char *outfile){
    int fd1, fd2;
    int dummy;
    char *newargv[2];
    pid_t child_pid;
    int child_status;
    child_pid = fork();
    if(child_pid < 0){
        printf("---forking child process failed---\n");
        exit(1);
    }else if(child_pid == 0) {
        //child process
        if(modeI==1){
            fd1 = open(infile, O_RDONLY);
            if (fd1 < 0) {
                perror("error opening infile");
                exit(1);
            }
            if (dup2(fd1, 0) != 0) {
                perror("error in dup2(infile, 0)");
                exit(1);
            }
            close(fd1);
        }
        if(modeO==1){
            fd2 = open(outfile, O_WRONLY | O_TRUNC | O_CREAT, 0644);
            if (fd2 < 0) {
                perror("error opening infile");
                exit(2);
            }
            if (dup2(fd2, 1) != 1) {
                perror("error in dup2(outfile, 1)");
                exit(1);
            }
            close(fd2);
        }
        execvp(word[0], word);
        printf("Unknown command\n");
        //exiting
        exit(1);
        
    }else{
        wait(NULL);
  }
}
int pipe_present(char **word, char **c1, char **c2,int len){
    int temp=0,check=0,i=0;
    for(i=0;i<len;i++){
        if(strcmp(word[i], "|") == 0){
            check=1;
            temp=0;
            c1[i]=NULL;
            continue;
        }
        if(check==0){
            c1[temp]=word[i];
            temp++;
        }
        if(check==1){
            c2[temp]=word[i];
            temp++;
        }
    }
    c2[i]=NULL;
    //printf("%s %s %s %s\n",c1[0],c2[0],c1[1],c2[1]);
    return check;
}
void exePiped(char **c1, char **c2){
    int pfd[2];
    pid_t p1,p2;
    //make pipe
    if (pipe(pfd) < 0) { 
		printf("\nPipe not made\n"); 
		return; 
	}
    p1=fork();
    if(p1<0){
        printf("fork was not possible\n");
        return;
    }
    if(p1==0){
        close(pfd[0]);
        dup2(pfd[1], STDOUT_FILENO);
        close(pfd[1]);
        char* temp=c1[0];
        parseExe(temp);
        // if(execvp(c1[0], c1)<0){ 
		// 	printf("could not execute '%s'\n",c1[0]); 
		// 	exit(0);
		// }
        exit(0);
    }else{
        wait(NULL);
        p2 = fork();
		if (p2 < 0) { 
			printf("fork was not possible\n");
			return; 
		}
        if (p2 == 0) { 
            close(pfd[1]);
            dup2(pfd[0], STDIN_FILENO);
            close(pfd[0]);
            char* temp=c2[0];
            parseExe(temp);
            // if(execvp(c2[0], c2)<0){ 
            //     printf("could not execute '%s'",c2[0]); 
            //     exit(0);
            // }
            exit(0);
        }else{
            wait(NULL);
            wait(NULL);
        }
    }
}
int parselvl1(char *inp,char **commands){
    char *ptr = strtok(inp, "|");
    int l=0;
    while(ptr != NULL){
        commands[l]=ptr;
        ptr = strtok(NULL, "|");
        l++;
    }
    return l;
}
void parseExe(char command[]){
    char *cmd_args[100];
    char cmd[1000];

    int stdoutCopy = dup(0);
    int stdinCopy = dup(1);

    //printf("%ld",strlen(command));
    int j=0,i=0;
    for(i=0;i<strlen(command)-1;i++){
        if(command[i+1]=='<' && command[i]!=' '){
            cmd[j++]=command[i];cmd[j++]=' ';
        }else if(command[i+1]!=' ' && command[i]=='<'){
            cmd[j++]=command[i];cmd[j++]=' ';
        }else if(command[i+1]=='>' && command[i]!=' '){
            cmd[j++]=command[i];cmd[j++]=' ';
        }else if(command[i+1]!=' ' && command[i]=='>'){
            cmd[j++]=command[i];cmd[j++]=' ';
        }else{
            cmd[j++]=command[i];
        }
    }
    if(command[i]!=' '){
        cmd[j++]=command[i];
    }
    cmd[j]='\0';
    //printf("%s\n",cmd);
    int k=0;
    char *ptr = strtok(cmd, " ");
    while(ptr != NULL){
        cmd_args[k]=ptr;
        ptr = strtok(NULL, " ");
        k++;
    }
        cmd_args[k]=NULL;
        int modeI=0,modeO=0;
        char infile[100]="";
        char outfile[100]="";
        /*
            modeI -> inp as input file
            modeO -> out as output file
        */
    for(int it=0;it<k;it++){
        if(strcmp(cmd_args[it], "<") == 0){
            modeI=1;
            it++;
            strcpy(infile, cmd_args[it]);
        }else if(strcmp(cmd_args[it], ">") == 0){
            modeO=1;
            it++;
            strcpy(outfile, cmd_args[it]);
        }
    }
    runcmd(cmd_args, modeI, modeO, infile, outfile);
    dup2(stdoutCopy,1);
    dup2(stdinCopy,0);
    
}
int main(){
    //declarations
    int x=0,input_length=0;
    char input[1000];
    char str_inp[1000];
    //starting the shell
    start_shell();
    //loop
    while(1){
        //the current directory and prompt
        prompt();
        //get input from the user
        x=get_input(input);
        if(x == 0){
            continue;//if the input is empty, prompt again
        }else{
            //input from user in obtained in input
            char* word[100];//words in the current input
            char* c1[100];
            char* c2[100];
            input_length = 0;//number of words in current input
            strcpy(str_inp,input);
            char *ptr = strtok(input, " ");
            while(ptr != NULL){
                word[input_length]=ptr;
                ptr = strtok(NULL, " ");
                input_length++;
            }
            //FOR PRINTING WORDS IN INPUT
            // for(int k=0;k<input_length;k++){
            //     printf("%s|\n",word[k]);
            // }
            //word[0] is the command
            if (strcmp(word[0], "pwd") == 0){
                if(input_length==1){
                    call_pwd();
                }else{
                    printf("Executing pwd command\n");
                    /////////////////////
                    char* commands[100];
                int l;
                l=parselvl1(str_inp,commands);
                if(l==1){
                    //no piping
                    char *command=commands[0];
                    parseExe(command);
                }else{
                    char *c1[3];c1[0]=commands[0];c1[1]=NULL;
                    char *c2[3];c2[0]=commands[1];c2[1]=NULL;
                    if(c1[0][strlen(c1[0])-1]==' '){
                        c1[0][strlen(c1[0])-1]='\0';
                    }
                    if(c1[0][0]==' '){
                        memmove(&c1[0][0], &c1[0][1], strlen(c1[0]));
                    }
                    if(c2[0][strlen(c2[0])-1]==' '){
                        c2[0][strlen(c2[0])-1]='\0';
                    }
                    if(c2[0][0]==' '){
                        memmove(&c2[0][0], &c2[0][1], strlen(c2[0]));
                    }
                    //printf("%s %s %s %s %s %s ",c1[0],c2[0],c1[1],c2[1],commands[0],commands[1]);
                    exePiped(c1,c2);
                    /////////////////////
                }
                }
            }else if(strcmp(word[0], "cd") == 0){
                if(input_length==1){
                    printf("please enter in the form: cd directory\n");
                }else if(input_length>2){
                    printf("too many arguments. Please enter in the form: cd directory\n");
                }else{
                    call_cd(word);//@sourav handle names of folders with more than one words
                }
            }else if(strcmp(word[0], "mkdir") == 0){
                if(input_length==1){
                    printf("please enter in the form: mkdir directory\n");
                }else{
                    call_mkdir(word, input_length);
                }
            }else if(strcmp(word[0], "rmdir") == 0){
                if(input_length==1){
                    printf("please enter in the form: rmdir directory\n");
                }else{
                    call_rmdir(word, input_length);
                }
            }else if(strcmp(word[0], "exit") == 0){
                if(input_length>1){
                    printf("too many arguments. Please enter only exit to exit the shell\n");
                }else{
                    //printf("exit");
                    call_exit();
                }
            // }else if(pipe_present(word, c1, c2, input_length)==1){
            //     printf("%s %s %s %s %s %s ",c1[0],c2[0],c1[1],c2[1],c1[2],c2[2]);
            //     exePiped(c1,c2);
            }else{
                printf("Executing the command-->\n");
                char* commands[100];
                int l;
                l=parselvl1(str_inp,commands);
                if(l==1){
                    //no piping
                    char *command=commands[0];
                    parseExe(command);
                }else{
                    char *c1[3];c1[0]=commands[0];c1[1]=NULL;
                    char *c2[3];c2[0]=commands[1];c2[1]=NULL;
                    if(c1[0][strlen(c1[0])-1]==' '){
                        c1[0][strlen(c1[0])-1]='\0';
                    }
                    if(c1[0][0]==' '){
                        memmove(&c1[0][0], &c1[0][1], strlen(c1[0]));
                    }
                    if(c2[0][strlen(c2[0])-1]==' '){
                        c2[0][strlen(c2[0])-1]='\0';
                    }
                    if(c2[0][0]==' '){
                        memmove(&c2[0][0], &c2[0][1], strlen(c2[0]));
                    }
                    //printf("%s %s %s %s %s %s ",c1[0],c2[0],c1[1],c2[1],commands[0],commands[1]);
                    exePiped(c1,c2);
                }
            }
        }

    }
}
