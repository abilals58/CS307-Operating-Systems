// Ahmet Bilal Yildiz

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>


int main(int argc, char *argv[]){
    
    printf("I'm SHELL process, with PID: %d - Main command is: man ls | grep -A 2 -e '-c ' > output.txt\n", getpid());
    // initialization of pipe
    int fd[2];
    pipe(fd);
    
    int rc1 = fork(); // creating first child (man)
    
    if(rc1 < 0){ // exit if fork fails
        
        exit(1);
    }
    else if(rc1 == 0){ // first child
        
         printf("I'm MAN process, with PID: %d - My command is: man ls\n", getpid());
        
        dup2(fd[1], STDOUT_FILENO); // result of the man process is written to the write-end of the pipe
        
        // closing the pipe ends
        close(fd[0]);
        close(fd[1]);
        
        // execution of man
        char* args[] = {"man", "ls", NULL};
        execvp(args[0], args);
        printf("everyting is ok");
        
    }
    else { // shell process goes on
    
        waitpid(rc1, NULL, 0); // shell process waits for the first child
        int rc2 = fork(); // creating the second child (grep)
        
        if(rc2 < 0){ // exit if fork fails
            
            exit(1);
        }
        else if(rc2 == 0){ // second child
            
            printf("I'm GREP process, with PID: %d - My command is: grep -A 2 -e '-c ' > output.txt\n", getpid());
            
            dup2(fd[0], STDIN_FILENO); // result of the man process readed from the read-end of the pipe
             
             //closing the pipe ends
            close(fd[0]);
            close(fd[1]);
             
            // creating the output.txt and writing the results of the grep to there 
            int fd_out = open("output.txt", O_WRONLY | O_CREAT, 0666); 
            dup2(fd_out, 1);
            close(fd_out);
            
            // execution of the grep 
            char* args[] = {"grep", "-A 2", "-e", "-c ", NULL};
            execvp(args[0], args);
            
        }
        else { // shell process goes on 
            
            //closing the pipe ends
            close(fd[1]);
            close(fd[0]);
            waitpid(rc2, NULL, 0); // shell process waits the second child
            
            printf("I'm SHELL process, with PID: %d - execution is completed, you can find the results in output.txt\n", getpid());
        
        }
        
    }
    

    return 0;
}
