#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>



void listDir(const char* dirpath, int is_recursive, off_t value, int size_filter, int name_filter, char* str){
     
     DIR* dir = opendir(dirpath);
     if(dir==NULL){
        perror("Error opening directory");
        return;
        
     }
     char filepath[1024];
     struct stat statbuf;
     struct dirent* entry; 
     while((entry = readdir(dir))!= NULL){
     
        if(is_recursive==0){
           snprintf(filepath, 1024, "%s/%s", dirpath, entry->d_name);
           if(lstat(filepath, &statbuf)==0){
              if(size_filter==1){
                 if(S_ISDIR(statbuf.st_mode)==0){
                    if(statbuf.st_size < value){
                       printf("%s\n", filepath);
                    }
                 } 
              
              }else if(name_filter==1){
                 int n = strlen(str);
                 char* file_name=entry->d_name;
                 if(strncmp((file_name+strlen(file_name)-n), str, n)==0){
                     printf("%s\n", filepath);
                 } 
              
              }else{
                  printf("%s\n", filepath);
              }
           } 
        
        }else{
          if(strcmp(entry->d_name, "..")!=0 && strcmp(entry->d_name, ".")!=0){
             snprintf(filepath, 1024, "%s/%s", dirpath, entry->d_name);
             if(lstat(filepath, &statbuf)==0){
                 if(S_ISDIR(statbuf.st_mode)){
                     if(size_filter!=1 && name_filter==1){
                         int n = strlen(str);
                         char* file_name=entry->d_name;
                         if(strncmp((file_name+strlen(file_name)-n), str, n)==0){
                            printf("%s\n", filepath);
                         }
                     }else if(size_filter!=1 && name_filter!=1){
                         printf("%s\n", filepath);
                     }               
                     listDir(filepath, is_recursive, value, size_filter, name_filter, str);
                 }else{
                    if(size_filter!=1 && name_filter!=1){
                       printf("%s\n", filepath);
                    } 
                    if(size_filter==1){
                        if(statbuf.st_size < value){
                               printf("%s\n", filepath);
                        }
                    }
                    if(name_filter==1){
                       int n = strlen(str);
                       char* file_name=entry->d_name;
                       if(strncmp((file_name+strlen(file_name)-n), str, n)==0){
                            printf("%s\n", filepath);
                       }
                   }
                     
                }
                
              }
            }
          }
       }
    closedir(dir);
 }


int main(int argc, char* argv[]){

    char* dir_path=NULL;
    int recursive=0;
    int i;
    off_t value;
    int size_filter=0;
    int name_filter=0;
    char* str=NULL;
  
       if(argc>=2){
         //printf("argc: %d\n", argc);
         for(i=1; i<argc; i++){
            if(strcmp(argv[i], "variant")==0){
               printf("25195\n");
            }
            if(strcmp(argv[i], "list")==0){
               i++;
           //    printf("i at list: %d\n", i);
               if(i<argc && strcmp(argv[i], "recursive")==0){
                  recursive=1;
                  i++;
             //     printf("i at recursive: %d\n", i);
               }
               if(i<argc && strncmp(argv[i], "size_smaller=", 13)==0){
                 size_filter=1;
                 value=atoll((argv[i]+13));
                 i++;
               //  printf("i at size_smaller: %d\n", i);
               }
               if(i<argc && strncmp(argv[i], "name_ends_with=", 15)==0){
                 name_filter=1;
                 str=argv[i]+15;
                 i++;
                 //printf("i at name_ends_with: %d\n", i);
               }
               if(i<argc && strncmp(argv[i], "path=", 5)==0){
                 dir_path=argv[i]+5;
                // printf("i at path= %d\n", i);
             }
          }
       }
       if(strcmp(argv[1], "variant")!=0){
           if(dir_path==NULL){
           printf("ERROR: invalid directory path\n");
           return -1;
           }
           //printf("dir_path: %s\nrecursive: %d\nvalue: %ld\nsize_filter: %d\nname_filter: %d\nstr: %s\n", dir_path, recursive, value, size_filter, name_filter, str);
           printf("SUCCESS\n");
           listDir(dir_path, recursive, value, size_filter, name_filter, str);
           
       }
   }
   return 0;

    
}
