#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <pthread.h>
#include <semaphore.h>


#define NR_THREADS_P2 6
#define NR_THREADS_P7 46
#define NR_THREADS_P8 4



typedef struct {
   int thread_id;
   sem_t* logSem;
}th_struct;

typedef struct{
  int thread_id;
}th_struct_p2;

sem_t shared_sem1;
sem_t shared_sem2;


void* thread_fn_for_p8(void* arg){

   th_struct* s = (th_struct*)arg;
   if(s->thread_id == 4){
       sem_wait(s->logSem);
       info(BEGIN, 8, 4);
       info(END, 8, 4);
   }
   else if(s->thread_id==1){
      info(BEGIN, 8, 1);
      sem_post(s->logSem);
   }else if(s->thread_id==2){
      sem_wait(&shared_sem1);
      info(BEGIN, 8, 2);
      info(END, 8, 2);
      sem_post(&shared_sem2);
   }else if(s->thread_id==3){
      info(BEGIN, 8, 3);
      info(END, 8, 3);
   }
   
    
   return NULL;
}


void* thread_fn_for_p7(void* arg){
    
    th_struct* s = (th_struct*)arg; 
    
    sem_wait(s->logSem);
    info(BEGIN, 7, s->thread_id);
    info(END, 7, s->thread_id);
    sem_post(s->logSem);
    
    return NULL;
    
}


void* thread_fn_for_p2(void* arg){
    
    th_struct_p2* stru = (th_struct_p2*)arg;

    
    
    if(stru->thread_id==1){
       info(BEGIN, 2, 1);
       info(END, 2, 1);
       sem_post(&shared_sem1);
    }else if(stru->thread_id==3){
       sem_wait(&shared_sem2);
       info(BEGIN, 2, 3);
       info(END, 2, 3);
    }else if(stru->thread_id==2){
      info(BEGIN, 2, 2);
      info(END, 2, 2);
    }else if(stru->thread_id==4){
       info(BEGIN, 2, 4);
       info(END, 2, 4);
    }else if(stru->thread_id==5){
       info(BEGIN, 2, 5);
       info(END, 2, 5);
    }else if(stru->thread_id==6){
       info(BEGIN, 2, 6);
       info(END, 2, 6);
    }
    return NULL;

}





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
        pthread_t thrd_arr_p2[NR_THREADS_P2];
        th_struct_p2 params_p2[NR_THREADS_P2];
        if(sem_init(&shared_sem1, 1, 1)){
           perror("Couldnt create the semaphore shared_sem1\n");
           return -1;
        }
        if(sem_init(&shared_sem2, 1, 1)){
           perror("Couldnt create the semaphore shared_sem2\n");
           return -1;
        }        
        for(int i=0; i<NR_THREADS_P2; i++){
            params_p2[i].thread_id = i+1;
            if(pthread_create(&thrd_arr_p2[i], NULL, thread_fn_for_p2, &params_p2[i])){
               perror("Couldnt create the threads\n");
               return -1;
            }
        }
        
        for(int i=0; i<NR_THREADS_P2; i++){
           if(pthread_join(thrd_arr_p2[i], NULL)){
              perror("Couldnt join the threads\n");
              return -1;
           }
        }
        
        sem_destroy(&shared_sem1);
        sem_destroy(&shared_sem2);
        
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
            } else {
            waitpid(P6, 0, 0);
            info(END, 3, 0);
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
        }else{
           waitpid(P5, 0, 0);
           P7=fork();
           if(P7==-1){
             perror("Couldnt create P7\n");
             return -1;
           }else if(P7==0){
             info(BEGIN, 7, 0);
             pthread_t thr_arr_p7[NR_THREADS_P7];
             th_struct params_p7[NR_THREADS_P7];
             sem_t sem;
             if(sem_init(&sem, 0, 6)){
                perror("Couldnt initialize the semaphore mutex\n");
                return -1;
             }
             for(int i=0; i<NR_THREADS_P7; i++){
                params_p7[i].thread_id = i+1;
                params_p7[i].logSem = &sem;
                if(pthread_create(&thr_arr_p7[i], NULL, thread_fn_for_p7, &params_p7[i])){
                   perror("Couldnt create thread\n");
                   return -1;
                }
             }
             for(int i=0; i<NR_THREADS_P7; i++){
                if(pthread_join(thr_arr_p7[i], NULL)){
                   perror("Couldnt join the thread\n");
                   return -1;
                }
             }
             sem_destroy(&sem);
             info(END, 7, 0);
           } else{
             waitpid(P7, 0, 0);
             P8=fork();
             if(P8==-1){
               perror("Couldnt create P8\n");
               return -1;
             } else if(P8==0){
                info(BEGIN, 8, 0);
                pthread_t thr_arr_p8[NR_THREADS_P8];
                th_struct params_p8[NR_THREADS_P8];
                sem_t logSem;
               
                if(sem_init(&logSem, 0, 0)){
                    perror("Error initializing the semaphore\n");
                    return -1;
                }
                
                for(int i=0; i<NR_THREADS_P8; i++){
                  params_p8[i].thread_id=i+1;
                  params_p8[i].logSem=&logSem;
                   if(pthread_create(&thr_arr_p8[i], NULL, thread_fn_for_p8, &params_p8[i])){
                      perror("Couldnt create thread\n");
                      return -1;
                   }
                }
                
                
                for(int i=0; i<NR_THREADS_P8; i++){
                   if(pthread_join(thr_arr_p8[i], NULL)){
                      perror("Couldnt join the thread\n");
                      return -1;
                   }
                }
                info(END, 8, 1);
                
                sem_destroy(&logSem);
                info(END, 8, 0);
             } else {
                waitpid(P8, 0, 0);
                info(END, 2, 0);
               
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
      }else{
      waitpid(P4, 0, 0);
      waitpid(P2, 0, 0);  
      info(END, 1, 0);
      }
    }   
    return 0;
  
}