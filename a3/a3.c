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
#define MEM_MAP_MESSAGE "MAP_FILE"
#define READ_MESSAGE "READ_FROM_FILE_OFFSET"
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
   void* shared_mem = NULL;
   int fd;
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
            shared_mem = (void*)mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
            if(shared_mem == MAP_FAILED){
               if(write(resp_pipe, "CREATE_SHM!ERROR!", 17)<0){
                  printf("Couldnt write error message to pipe\n");
                  return -1;
               }
               printf("Couldnt map the shared memory\n");
               return -1;
           } 
            write(resp_pipe, "CREATE_SHM!SUCCESS!", 19);
            //close(shmFd);
            //shm_unlink("/9Z9YBGve"); 
       }
       else if(strncmp(WRITE_MESSAGE, request, 12)==0){
           printf("The request string: %s\n", request);
           //void* shared_mem = NULL;
           //shared_mem = (unsigned int*)mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);          
           unsigned int offset;
           unsigned int value;
           unsigned int* sharedValue = NULL;
           read(req_pipe, &offset, sizeof(unsigned int));
           read(req_pipe, &value, sizeof(unsigned int));
           printf("Offset: %ld\nValue: %u\n", (off_t)offset, value);
           int shvFd = shm_open("/9Z9YBGve", O_RDWR, 0664);
           if(shvFd < 0){
              write(resp_pipe, "WRITE_TO_SHM!ERROR!", 19);
              perror("Couldnt aquire shared memory\n");
              return -1;
           }
           if(offset >= 0 && offset <= shared_mem_size){
              sharedValue = (unsigned int*)mmap(NULL, shared_mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, shvFd, (off_t)offset);
              if(sharedValue == (void*)-1){
                 write(resp_pipe, "WRITE_TO_SHM!ERROR!", 19);
                 printf("Error mapping shared memory\n");
                 return -1;
              }
              *sharedValue = value;
              printf("SharedValue: %u\n", *sharedValue);
              printf("Inside the offset comparation\n");
              write(resp_pipe, "WRITE_TO_SHM!SUCCESS!", 21);
           }        
       }
       else if(strncmp(MEM_MAP_MESSAGE, request, 8)==0){
          char file_name[250];
          char ch = '\0';
          int j = 0;
          while(ch != '!'){
             if(read(req_pipe, &ch, sizeof(ch))<0){
                printf("Couldnt read from the pipe\n");
                return -1;
             }
             file_name[j++] = ch;
          }
          file_name[--j] = '\0';
          //printf("File name: %s\n", file_name);
          fd = open(file_name, O_RDWR);
          if(fd == -1){
             write(resp_pipe, "MAP_FILE!ERROR!", 15);
             perror("Couldnt open input file\n");
             return -1;
          }
          off_t size = lseek(fd, 0, SEEK_END);
          lseek(fd, 0, SEEK_SET);
          printf("File size: %ld\n", size);
          char* data = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
          if(data == (void*)-1){
             write(resp_pipe, "MAP_FILE!ERROR!", 15);
             perror("Couldnt map file\n");
             close(fd);
             return -1;
          }
          write(resp_pipe, "MAP_FILE!SUCCESS!", 17);
          //munmap(data, size);
          //close(fd);
       }
       else if(strncmp(READ_MESSAGE, request, 21)==0){
           unsigned int read_offset;
           unsigned int no_of_bytes;
           read(req_pipe, &read_offset, sizeof(read_offset));
           read(req_pipe, &no_of_bytes, sizeof(no_of_bytes));
           printf("Read offset: %u\nNumber of bytes: %u\n", read_offset, no_of_bytes);
           char* sharedChar;
           char sharedBytes[no_of_bytes]; 
           int k = 0;          
           while(no_of_bytes != 0){
              sharedChar = (char*)mmap(0, no_of_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
              if(sharedChar == (void*)-1){
                 write(resp_pipe, "READ_FROM_FILE_OFFSET!ERROR!", 28);
                 return -1;
              }
              //offset=offset+1;
              sharedBytes[k++] = *sharedChar;
           }
           printf("Shared char array: %s", sharedBytes);
           write(resp_pipe, "READ_FROM_FILE_OFFSET!SUCCESS!", 30);
           //munmap(data, size);
           close(fd);           
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


