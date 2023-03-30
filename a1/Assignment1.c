#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

int listDir(const char *path)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char filePath[512];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL) {
        perror("ERROR\nCould not open directory");
        return -1;
    }
    printf("SUCCESS\n");
    while((entry = readdir(dir)) != NULL) {
        snprintf(filePath, 512, "%s/%s", path, entry->d_name);
        if(lstat(filePath, &statbuf) == 0) {
            if(S_ISREG(statbuf.st_mode)) {
                printf("[reg] ");
            } else if(S_ISDIR(statbuf.st_mode)) {
                printf("[dir] ");
            } else if(S_ISLNK(statbuf.st_mode)) {
                printf("[lnk] ");
            } else {
                printf("[unk] ");
            }
            printf("[%o] ", statbuf.st_mode & 0777);
        } else {
            printf("[err] ");
        }
        printf("%s\n", entry->d_name);
    }
    closedir(dir);
    return 0;
}

void listRec(const char *path)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);
    if(dir == NULL) {
        perror("ERROR\nCouldn't open directory");
        return;
    }
    printf("SUCCESS\n");
    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0) {
                printf("%s\n", fullPath);
                if(S_ISDIR(statbuf.st_mode)) {
                    listRec(fullPath);
                }
            }
        }
    }
    closedir(dir);
    //return 0;
}

int main(int argc, char **argv)
{
    
    int variant=25195;

    
    if(strcmp(argv[1], "variant")==0){
       printf("%d\n", variant);
    }
    if(strcmp(argv[1], "list")==0){
       listDir(argv[2]);
    }
    if(strcmp(argv[1], "list")==0 && strcmp(argv[2], "recursive")==0){
          listRec(argv[3]);
    }
    
    return 0;
}
