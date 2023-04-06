#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


struct section{
  char sect_name[18];
  unsigned int sect_type;
  unsigned int sect_size;
}secton;

int parse_file(const char* filepth){
    
    // variabile de validare
    
      int ver_magic;
      int ver_version = 0;
      int ver_nr_of_sections =0 ; 
    
    // variabile pentru citire
    int fd=-1;
    
    fd = open(filepth, O_RDONLY);
    if(fd == -1){
       perror("Couldn't open the file\n");
       return -1;
    }
    
    char magic;
    if(read(fd, &magic, 1)==-1){
       perror("Error while reading magic\n");
    }
    
    if(magic=='o'){
       ver_magic=1;
    }else{
       printf("ERROR\nwrong magic\n");
       return -1;
    }
    
    lseek(fd, 2, SEEK_CUR);
    // reading version
    unsigned char version;
    if(read(fd, &version, 1)==-1){
       perror("Error while reading version\n");
    } 
    int version_value=(int)version;
    if(version_value>=103 && version_value<=133){
       ver_version=1;
      // printf("version=%d\n", version_value);
    }else{
       printf("ERROR\nwrong version");
       return -1;
    }
    
    
    unsigned char no_of_sections;
    if(read(fd, &no_of_sections, 1)==-1){
       perror("Error while reading nr_of_sections\n");
    } 
    int nr_sections=(int)no_of_sections;
    if(nr_sections>=7 && nr_sections<=19){
       ver_nr_of_sections=1;
       //printf("nr_sections=%d\n", nr_sections);
    }else{
       printf("ERROR\nwrong sect_nr\n");
       return -1;
    }
    
    
    struct section* sections = (struct section*)malloc(nr_sections*sizeof(struct section));
    if(sections==NULL){
      printf("Couldnt allocate the sectons\n");
      return -1;
    }
    
    int ver_sect_type = 0;
    for(int i=0; i<nr_sections; i++){
       if(read(fd, &sections[i].sect_name, 17)==-1){
          printf("Couldnt read the sect_name\n");
          return -1;
       }
       sections->sect_name[17]='\0';
       if(read(fd, &sections[i].sect_type, 4)==-1){
          printf("Couldnt read the sect_name\n");
          return -1;
       }
       if(sections[i].sect_type==49 || sections[i].sect_type==60 || sections[i].sect_type==38 || sections[i].sect_type==87 || sections[i].sect_type==54 || sections[i].sect_type==55 ){
         ver_sect_type++;
       }else{
          printf("ERROR\nwrong sect_types\n");
       }
       lseek(fd, 4, SEEK_CUR);
       if(read(fd, &sections[i].sect_size, 4)==-1){
          printf("Couldnt read the sect_name\n");
          return -1;
       }
       
    }
    
    if(ver_magic==1 && ver_version==1 && ver_nr_of_sections==1 && ver_sect_type==nr_sections){
       printf("SUCCESS\n");
       printf("version=%d\n", version_value);
       printf("nr_sections=%d\n", nr_sections);
       for(int i=0; i<nr_sections; i++){
          printf("section%d: %s %d %d\n", i+1, sections[i].sect_name, sections[i].sect_type, sections[i].sect_size);
       }
    } 
    
    free(sections);
    
    close(fd);   
    
    
    return 0;
}



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
                  if(strcmp(entry->d_name, "..")!=0 && strcmp(entry->d_name, ".")!=0){
                      printf("%s\n", filepath);  
                  }
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
            if(strcmp(argv[i], "parse")==0){
               i++;
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
               if(i<argc && strcmp(argv[i], "recursive")==0){
                  recursive=1;
                  i++;
             //     printf("i at recursive: %d\n", i);
               }
               if(i<argc && strncmp(argv[i], "name_ends_with=", 15)==0){
                 name_filter=1;
                 str=argv[i]+15;
                 i++;
                 //printf("i at name_ends_with: %d\n", i);
               }
               if(i<argc && strcmp(argv[i], "recursive")==0){
                  recursive=1;
                  i++;
             //     printf("i at recursive: %d\n", i);
                }
             }
             if(i<argc && strncmp(argv[i], "path=", 5)==0){
                 dir_path=argv[i]+5;
                // printf("i at path= %d\n", i);
             }
       }
       if(strcmp(argv[1], "variant")!=0 && strcmp(argv[1], "parse")!=0){
           if(dir_path==NULL){
           printf("ERROR: invalid directory path\n");
           return -1;
           }
           //printf("dir_path: %s\nrecursive: %d\nvalue: %ld\nsize_filter: %d\nname_filter: %d\nstr: %s\n", dir_path, recursive, value, size_filter, name_filter, str);
           printf("SUCCESS\n");
           listDir(dir_path, recursive, value, size_filter, name_filter, str);
           
       }
       if(strcmp(argv[1], "parse")==0){
          if(dir_path==NULL){
            printf("ERROR: invalid directory path(in parse)\n");
            return -1;
          }
          //printf("SUCCESS\n");
          parse_file(dir_path);
       }
   }
   return 0;

    
}