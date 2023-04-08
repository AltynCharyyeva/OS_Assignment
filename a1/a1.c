#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


struct section_hd{
  char sect_name[18];
  unsigned int sect_type;
  unsigned int sect_size;
}secton_hd;



void free_matrix(char** matrix, unsigned int rows)
{
    for (unsigned int i = 0; i < rows; i++)
    {
        free(matrix[i]);
    }

    free(matrix);
}


int extract_content(const char* filepath, int sect_nr, int line_nr){
    
    // variabile de verificare daca fisierul e SF file sau nu
    int ver_magic=0;
    int ver_version = 0;
    int ver_nr_of_sections = 0 ; 
    
    int fd=-1;
    
    fd = open(filepath, O_RDONLY);
    if(fd == -1){
       perror("Couldn't open the file\n");
       return -1;
    }
    
    // citesc magic si il verific
    char magic;
    if(read(fd, &magic, 1)==-1){
       perror("Error while reading magic\n");
    }
    if(magic=='o'){
       ver_magic=1;
    }else{
       printf("ERROR\ninvalid file\n");
       return -1;
    }
    
    
    lseek(fd, 2, SEEK_CUR);  // sarim peste header_size cu 2 bytes
    // citesc version si il verific
    unsigned char version;
    if(read(fd, &version, 1)==-1){
       perror("Error while reading version\n");
    } 
    int version_value=(int)version;
    if(version_value>=103 && version_value<=133){
       ver_version=1;
    }else{
       printf("ERROR\ninvalid file\n");
       return -1;
    }
    
    
    // citesc no_of_versions si il verific
    unsigned char no_of_sections;
    if(read(fd, &no_of_sections, 1)==-1){
       perror("Error while reading nr_of_sections\n");
    } 
    int nr_sections=(int)no_of_sections;
    if(nr_sections>=7 && nr_sections<=19){
       ver_nr_of_sections=1;
    }else{
       printf("ERROR\ninvalid file\n");
       return -1;
    }
    
    // citesc sect_type-urile si le verific
    int ver_sect_type = 0;
    unsigned int sect_type;
    for(int i=0; i<nr_sections; i++){
       lseek(fd, 17, SEEK_CUR);
       if(read(fd, &sect_type, 4)==-1){
          printf("Couldnt read the sect_type\n");
          return -1;
       }
       if(sect_type==49 || sect_type==60 || sect_type==38 || sect_type==87 || sect_type==54 || sect_type==55 ){
         ver_sect_type++;
       }else{
          printf("ERROR\ninvalid file\n");
       }
       lseek(fd, 8, SEEK_CUR);
       
    }
    
    // verific daca fisierul e SF file si daca da, pun cursorul la inceputul fisierului si caut section-ul dat
    unsigned int sect_offset=0;
    unsigned int sect_size=0;
    if(ver_magic==1 && ver_version==1 && ver_nr_of_sections==1 && ver_sect_type==nr_sections){
        lseek(fd, 5, SEEK_SET);
        for(int i=1; i<=nr_sections; i++){
           if(sect_nr==i){
              lseek(fd, 17, SEEK_CUR);  // sarim peste sect_name
              lseek(fd, 4, SEEK_CUR);   // sarim peste sect_type
              if(read(fd, &sect_offset, 4)==-1){  //citim sect_offset
                  printf("Couldnt read the sect_offset\n");
                  return -1;
              }
              if(read(fd, &sect_size, 4)==-1){
                 printf("Couldnt read sect_size\n");
                 return -1;
              }
              break;
           }
           lseek(fd, 17, SEEK_CUR);
           lseek(fd, 12, SEEK_CUR);
           
        }
        

        lseek(fd, sect_offset, SEEK_SET);

    
    }
    
    // am gasit section-ul si citesc linii-le separat in array of strings si maresc dimensiunea cand citesc 
    unsigned int nr_linie=0;
    unsigned int nr_linie_max=1;
    
    unsigned int nr_chars=0;
    unsigned int nr_chars_max=2;
    
    unsigned int size_checker=0;
    
    char buffer[512]; 
    
    char** section = (char**)malloc(nr_linie_max*sizeof(char*));
    if(section==NULL){
       printf("Couldnt allocate the memory\n");
       return -1;
    }
    
    section[nr_linie]=(char*)malloc(nr_chars_max*sizeof(char));
    if(section[nr_linie]==NULL){
      printf("Couldnt allocate the section line\n");
      free_matrix(section, nr_linie);
      return -1;
    }
    
 
    for(unsigned int i=0; i<sect_size;){
       if(read(fd, &buffer, sizeof(buffer))==-1){ // citesc un group de bytes si le verific daca e sfarsitul liniei sau nu   
         printf("Error reading buffer\n");        // cand le pun in tablou de string-uri numit section
         free_matrix(section, nr_linie);
         return -1;
       }
       for(unsigned int j=0; j<sizeof(buffer); j++){
          if(buffer[j]=='\n' && j>0 && buffer[j-1]=='\r'){  // am ajuns la sfarsitul liniei
            section[nr_linie][nr_chars]='\0';
            // reinitializam varibile de masura
            nr_chars_max=2;
            nr_chars=0;
            if(nr_linie+1>=nr_linie_max){  
              char** aux = realloc(section, 2*nr_linie_max*sizeof(char*));
              if(aux==NULL){
                printf("Couldnt do realloc in lines\n");
                free_matrix(section, nr_linie);
                return -1;
              }
              section=aux;
              nr_linie_max*=2;
            }
            section[nr_linie+1] = (char*)malloc(nr_chars_max+1);
            if(section[nr_linie+1]==NULL){
               printf("Couldnt do malloc for the next line\n");
               free_matrix(section, nr_linie);
               return -1;
            }
            nr_linie++;
            continue;
          }
          if(nr_chars>=nr_chars_max){
             char* aux = realloc(section[nr_linie], 2*nr_chars_max+1);
             if(aux==NULL){
               printf("Couldnt do realloc\n");
               free_matrix(section, nr_linie);
               return -1;
             }
             section[nr_linie]=aux;
             nr_chars_max*=2;
          }
          if(size_checker==sect_size){
            break;
          }
          section[nr_linie][nr_chars]=buffer[j];
          nr_chars++;
          size_checker++;
          
       }
       if(size_checker==sect_size){
         break;
       }
       i+=(sizeof(buffer));
    }
    

     // Add any remaining data to the section matrix
        if(nr_chars > 0) {
            section[nr_linie][nr_chars] = '\0';
        }


     close(fd);

    // citesc linii in ordinea invers
    unsigned int k=nr_linie-1;
    for(unsigned int i=0; i<nr_linie; i++){
      if(line_nr==i+1){
       printf("SUCCESS\n");
       printf("%s\n", section[k+1]); 
       break; 
      }
      k--;
    }
    
    free_matrix(section, nr_linie);

    return 0;
    
}

int parse_file(const char* filepth){
    
    // variabile de validare
    
    int ver_magic = 0;
    int ver_version = 0;
    int ver_nr_of_sections = 0 ; 
    

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
    unsigned char version;
    if(read(fd, &version, 1)==-1){
       perror("Error while reading version\n");
    } 
    int version_value=(int)version;
    if(version_value>=103 && version_value<=133){
       ver_version=1;
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
    }else{
       printf("ERROR\nwrong sect_nr\n");
       return -1;
    }
    
    // pun in section headers header_name-uri, sect_type-uri, sect_offset-uri si sect_size-uri
    struct section_hd* sections = (struct section_hd*)malloc(nr_sections*sizeof(struct section_hd));
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
    int sect_nr=0;
    int line_nr=0;
  
       if(argc>=2){
         for(i=1; i<argc; i++){
            if(strcmp(argv[i], "variant")==0){
               printf("25195\n");
            }
            if(strcmp(argv[i], "parse")==0){
               i++;
            }
            if(strcmp(argv[i], "findall")==0){
               i++;
            }
            if(strcmp(argv[i], "extract")==0){
                 i++;
                 if(i<argc && strncmp(argv[i], "path=", 5)==0){
                     dir_path=argv[i]+5;
                     i++;
                 }
                 if(i<argc && strncmp(argv[i], "section=", 8)==0){
                     sect_nr=atoi(argv[i]+8);
                     i++;
                 }
                 if(i<argc && strncmp(argv[i], "line=", 5)==0){
                     line_nr=atoi(argv[i]+5);

                 }
              
            }
            if(strcmp(argv[i], "list")==0){
                 i++;
                 if(i<argc && strcmp(argv[i], "recursive")==0){
                     recursive=1;
                     i++;
                 }
                 if(i<argc && strncmp(argv[i], "size_smaller=", 13)==0){
                      size_filter=1;
                      value=atoll((argv[i]+13));
                      i++;
                 }
                 if(i<argc && strcmp(argv[i], "recursive")==0){
                     recursive=1;
                     i++;
                  }
                  if(i<argc && strncmp(argv[i], "name_ends_with=", 15)==0){
                      name_filter=1;
                      str=argv[i]+15;
                      i++;
                  }
                  if(i<argc && strcmp(argv[i], "recursive")==0){
                      recursive=1;
                      i++;
                  }
             }
             if(i<argc && strncmp(argv[i], "path=", 5)==0){
                 dir_path=argv[i]+5;
             }
       }
       if(strcmp(argv[1], "extract")==0){
          if(dir_path==NULL){
            printf("ERROR: invalid directory path\n");
            return -1;
          }
          extract_content(dir_path, sect_nr, line_nr);
       }
       if(strcmp(argv[1], "findall")==0){
         if(dir_path==NULL){
            printf("ERROR: invalid directory path\n");
            return -1;
         }
          printf("SUCCESS\n");
          //findall(dir_path);
       }
       if(strcmp(argv[1], "list")==0){
           if(dir_path==NULL){
           printf("ERROR: invalid directory path\n");
           return -1;
           }
           printf("SUCCESS\n");
           listDir(dir_path, recursive, value, size_filter, name_filter, str);
           
       }
       if(strcmp(argv[1], "parse")==0){
          if(dir_path==NULL){
            printf("ERROR: invalid directory path(in parse)\n");
            return -1;
          }
          parse_file(dir_path);
       }
   }
   return 0;

    
}