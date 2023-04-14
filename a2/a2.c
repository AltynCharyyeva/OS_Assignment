#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"

int main(){
    init();
    info(BEGIN, 1, 0);

    pid_t P2, P3, P4, P5, P6, P7, P8;

    P2 = fork();
    if(P2==-1){
        perror("Couldnt create P2\n");
        return -1;
    }
    if(P2==0){
        info(BEGIN, 2, 0);
        printf("Parent: %d\n", getppid());
        P3=fork();
        if(P3==-1){
           perror("Couldnt create P2\n");
           return -1;
        }
        if(P3==0){
            info(BEGIN, 3, 0);
            P6=fork();
            if(P6==-1){
              perror("Couldnt create P6\n");
              return -1;
            }
            else if(P6==0){
              info(BEGIN, 6, 0);
              info(END, 6, 0);
            //  sleep(60);
            } else {
            waitpid(P6, 0, 0);
            info(END, 3, 0);
          //  sleep(120);
            }
        }else{
        waitpid(P3, 0, 0);
        P5=fork();
        if(P5==-1){
          perror("Couldnt create P5\n");
          return -1;
        }
        else if(P5==0){
           info(BEGIN, 5, 0);
           info(END, 5, 0);
           //sleep(120);
        }else{
           waitpid(P5, 0, 0);
           P7=fork();
           if(P7==-1){
             perror("Couldnt create P7\n");
             return -1;
           }else if(P7==0){
             info(BEGIN, 7, 0);
             info(END, 7, 0);
            // sleep(120);
           } else{
             waitpid(P7, 0, 0);
             P8=fork();
             if(P8==-1){
               perror("Couldnt create P8\n");
               return -1;
             } else if(P8==0){
                info(BEGIN, 8, 0);
                info(END, 8, 0);
          //      sleep(120);
             } else {
                waitpid(P8, 0, 0);
                info(END, 2, 0);
                //sleep(300);
             }
           }  
         }
       }
       
    } else{
      P4=fork();
      if(P4==-1){
        perror("Couldnt create P4\n");
        return -1;
      }
      else if(P4==0){
        info(BEGIN, 4, 0);
        info(END, 4, 0);
        //sleep(60);
      }else{
      waitpid(P4, 0, 0);
      waitpid(P2, 0, 0);  
      info(END, 1, 0);
      }
    }   
    return 0;
  
}