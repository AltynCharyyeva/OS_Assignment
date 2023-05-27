#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/shm.h>


#define RESP_PIPE_NAME "RESP_PIPE_25195"
#define REQ_PIPE_NAME  "REQ_PIPE_25195"
#define SUCCESS_MESSAGE "SUCCESS"
#define BEGIN_MESSAGE "BEGIN!"
#define ECHO_MESSAGE "ECHO!"
#define VARIANT "VARIANT!"
#define CREATE_SHM_MESSAGE "CREATE_SHM"
#define WRITE_MESSAGE "WRITE_TO_SHM"
#define EXIT_MESSAGE "EXIT!"

int main (void){

   int req_pipe = - 1;
   int resp_pipe = -1;
   char request[250];
   unsigned int my_variant = 25195;

   
   
   // create a pipe for writing
   if(mkfifo(RESP_PIPE_NAME, 0666) != 0){
      printf("ERROR\n");
      perror("cannot create the response pipe\n");
      return -1;
   }
   
   // open a pipe for reading
   req_pipe = open(REQ_PIPE_NAME, O_RDONLY);
   if(req_pipe == -1){
      printf("ERROR\n");
      perror("cannot open the request pipe\n");
      return -1;
   }
   
   // open pipe for writing
   resp_pipe = open(RESP_PIPE_NAME, O_WRONLY);
   if(resp_pipe == -1){
      printf("ERROR\n");
      perror("cannot open the response pipe\n");
      return -1;      
   }
   
   
   write(resp_pipe, "BEGIN!", 6);
   printf("SUCCESS\n");
   
   unsigned int shared_mem_size;
   int shmFd;
   while(1){
       size_t i = 0;
       unsigned char byte;
       while(byte != '!'){
           if(read(req_pipe, &byte, sizeof(byte))==-1){
              printf("couldnt read the byte from the pipe\n");
              return -1;
           }
           request[i++] = byte;
       }
       request[i] = '\0';  //Null-terminate the received message
       if(strncmp(ECHO_MESSAGE, request, 4)==0){
          write(resp_pipe, "ECHO!", 5);
          write(resp_pipe, &my_variant, sizeof(my_variant));
          write(resp_pipe, "VARIANT!", 8);
          
       }else if(strncmp(CREATE_SHM_MESSAGE, request, 10)==0){
            read(req_pipe, &shared_mem_size, sizeof(unsigned int));
            shmFd = shm_open("/9Z9YBGve", O_CREAT | O_RDWR, 0664);
            if(shmFd < 0){
               write(resp_pipe, "CREATE_SHM!ERROR!", 17);
               perror("Couldnt aquire shared memory\n");
               return -1;
            }
            ftruncate(shmFd, shared_mem_size);
            write(resp_pipe, "CREATE_SHM!SUCCESS!", 19);
       }
       else if(strncmp(WRITE_MESSAGE, request, 12)==0){
           unsigned int offset;
           unsigned int value;
           read(req_pipe, &offset, sizeof(unsigned int));
           read(req_pipe, &value, sizeof(unsigned int));
           if(offset >= 0 && offset <= shared_mem_size){
              char* value_written = (char*)mmap(0, sizeof(unsigned int), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, (off_t)offset);
              if(value_written == (void*)-1){
                 write(resp_pipe, "WRITE_TO_SHM!ERROR!", 19);
                 perror("Couldnt map the shared memory\n");
                 return -1;
              }
              write(resp_pipe, "WRITE_TO_SHM!SUCCESS!", 21);
           }
       }
       else if(strncmp(EXIT_MESSAGE, request, i)==0){
            break;
       }
   }
   
   
   
   close(req_pipe);
   close(resp_pipe);
   
   
   unlink (RESP_PIPE_NAME);
   
   return 0;
   

}



