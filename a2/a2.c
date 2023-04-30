#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <pthread.h>
#include <semaphore.h>


typedef struct {
   int thread_id;
   sem_t* logSem;
}th_struct_p8;

typedef struct {
   int thread_id;
   sem_t* mutex;
   sem_t* barrier1;
   sem_t* barrier2;
   pthread_mutex_t* lock;
}th_struct_p7;

int count=0;
int nrThreads=0;



void* thread_fn_for_p8(void* arg){

   th_struct_p8* s = (th_struct_p8*)arg;
   if(s->thread_id == 4){
       sem_wait(s->logSem);
       info(BEGIN, 8, 4);
       //cnt++;
       info(END, 8, 4);
       //sem_post(s->logSem);
   }
   else if(s->thread_id==1){
      //cnt++;
      info(BEGIN, 8, 1);
      sem_post(s->logSem);
     // sem_wait(s->logSem);
      //info(END, 8, 1);
   }else if(s->thread_id==2){
      info(BEGIN, 8, 2);
      //cnt++;
      info(END, 8, 2);
   }else if(s->thread_id==3){
      info(BEGIN, 8, 3);
      //cnt++;
      info(END, 8, 3);
   }
    
   return NULL;
}


void* thread_fn_for_p7(void* arg){
   
   th_struct_p7* s = (th_struct_p7*)arg;
      //part1
      sem_wait(s->mutex);
      count = count+1;
      
      info(BEGIN, 7, s->thread_id);
      if(count == 6){
         sem_wait(s->barrier2);
         sem_post(s->barrier1);
      }
      sem_post(s->mutex);
      sem_wait(s->barrier1);
      sem_post(s->barrier2);
      
      //part2
      sem_wait(s->mutex);
      count = count -1;     
      
      info(END, 7, s->thread_id);
      if(count == 0){
         sem_wait(s->barrier1);
         sem_post(s->barrier2);
      }
      sem_post(s->mutex);
      sem_wait(s->barrier2);
      sem_post(s->barrier2);
   return NULL;
}

void* thrd_fun(void* arg){
    
    th_struct_p7* s = (th_struct_p7*)arg; 
    
    sem_wait(s->mutex);
    info(BEGIN, 7, s->thread_id);
    info(END, 7, s->thread_id);
    sem_post(s->mutex);
    
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
        //printf("Parent: %d\n", getppid());
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
             pthread_t thr_arr_p7[46];
             th_struct_p7 prms[46];
             sem_t mutex;
             sem_t barrier1;
             sem_t barrier2;
             pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
             if(sem_init(&mutex, 0, 6)){
                perror("Couldnt initialize the semaphore mutex\n");
                return -1;
             }
             if(sem_init(&barrier1, 0, 0)){
                perror("Couldnt initialize the semaphore barrier1\n");
                return -1;
             }
             if(sem_init(&barrier2, 0, 1)){
                perror("Couldnt initialize the semaphore barrier2\n");
                return -1;
             }
             for(int i=0; i<46; i++){
                prms[i].thread_id = i+1;
                prms[i].mutex = &mutex;
                //prms[i].barrier1 = &barrier1;
                //prms[i].barrier2 = &barrier2;
                //prms[i].lock = &lock;
                if(pthread_create(&thr_arr_p7[i], NULL, thrd_fun, &prms[i])){
                   perror("Couldnt create thread\n");
                   return -1;
                }
                //info(BEGIN, 7, i+1);
             }
             for(int i=0; i<46; i++){
                if(pthread_join(thr_arr_p7[i], NULL)){
                   perror("Couldnt join the thread\n");
                   return -1;
                }
                //info(END, 7, i+1);
             }
             sem_destroy(&mutex);
             sem_destroy(&barrier1);
             sem_destroy(&barrier2);
             pthread_mutex_destroy(&lock);
             info(END, 7, 0);
           } else{
             waitpid(P7, 0, 0);
             P8=fork();
             if(P8==-1){
               perror("Couldnt create P8\n");
               return -1;
             } else if(P8==0){
                info(BEGIN, 8, 0);
                pthread_t thr_arr_p8[4];
                th_struct_p8 params[4];
                sem_t logSem;
               
                if(sem_init(&logSem, 0, 0)){
                    perror("Error initializing the semaphore\n");
                    return -1;
                }
                
                for(int i=0; i<4; i++){
                  params[i].thread_id=i+1;
                  params[i].logSem=&logSem;
                   if(pthread_create(&thr_arr_p8[i], NULL, thread_fn_for_p8, &params[i])){
                      perror("Couldnt create thread\n");
                      return -1;
                   }
                  // info(BEGIN, 8, i+1);
                }
                
                
                for(int i=0; i<4; i++){
                   if(pthread_join(thr_arr_p8[i], NULL)){
                      perror("Couldnt join the thread\n");
                      return -1;
                   }
                   //info(END, 8, i+1);
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