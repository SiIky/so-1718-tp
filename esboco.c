#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define stdin 0
#define stdout 1

char* outputs[];

int analyze_line(char* line, int r){
    char aux[r];
    if (!strcmp(line[0],'$') {
        return -1;
    }
    for (int i = 1; i<r; i++) {
        if(strcmp(line[i],'|') {
            return atoi(aux);
        }
        aux[i] = line[i];
    }
    return 0;
    
} 
int main(){
   pid_t pid = fork();
   if (!pid) {
       /* son running */
       void * buf = NULL;
       size_t r = 0;
       /* read file name */
       r = file_readline(stdin,&buf);
       /* convert buf to string */ 
       // ...
       struct file * f = file_open(buf, O_APPEND | O_RDONLY);
       int counter_exec = 0;

       while((r = file_readline(f, &buf)) > 0){
           int n = analyze_line(buf,r); 
           switch(r) {
                case 0 : /* command without input */
                    int fd[2];
                    pipe(fd);
                    pid_t p = fork();
//                    write (fd[1] ,...)
//                    read(fd[0],...)
                    if (!p) {
                       /* son running */
                        close(fd[0]);
                        dup2(fd[1],stdout);
                        close(fd[1]);
                        // convert buf to char**
                        execvp(buf[0], buf);
                        _exit(1);
                    }
                    else{
                        close(fd[1]);
                        void * buff = NULL;
                        size_t r2 = 0;
                        while((r2 = file_readline(fd[0],&buff)) > 0){
                            // add outputs[ counter_exec ];
                            // write in f
                        }
                    }
                    break;

                case -1: /* documentation */
                    // ignorar??

                    break;

                default : /* Stdin = outputs[r] */
                    int fd[2];
                    int fd2[2];
                    pipe(fd);
                    pipe(fd2);
                    pid_t p = fork();
//                    write (fd[1] ,...)
//                    read(fd[0],...)
                    if (!p) {
                       /* son running */
                        dup2(fd[1],stdout);
                        dup2(fd2[0],stdin);
                        close(fd[1]);
                        close(fd[0]);
                        close(fd2[1]);
                        close(fd2[0]);
                        // convert buf to char**
                        execvp(buf[0], buf);
                        _exit(1);
                    }
                    else{
                        write(fd2[1], outputs[r], size(outputs[r]));
                        close(fd[1]);
                        close(fd2[0]);
                        close(fd2[1]);
                        void * buff = NULL;
                        size_t r2 = 0;
                        while((r2 = file_readline(fd[0],&buff)) > 0){
                            // add outputs[ counter_exec ];
                            // write in f
                        }

                   }
                    break;
       }

   }
   else{; // controller }
}
