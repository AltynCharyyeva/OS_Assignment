#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void list_directory_contents(const char *dir_path, int recursive, const char *filter) {
    // Open the directory
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }
    // Read the directory contents
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip entries for "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // Build the full path to the entry
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
        // Apply filtering options
        struct stat st;
        if (stat(path, &st) == -1) {
            perror("stat");
            continue;
        }
        if (filter != NULL) {
            if (strcmp(filter, "d") == 0 && !S_ISDIR(st.st_mode)) {
                continue;
            }
            if (strcmp(filter, "f") == 0 && !S_ISREG(st.st_mode)) {
                continue;
            }
        }
        // Print the entry name
        printf("%s\n", path);
        // Recursively list sub-directories
        if (recursive && S_ISDIR(st.st_mode)) {
            list_directory_contents(path, 1, filter);
        }
    }
    // Close the directory
    closedir(dir);
}


int main(int argc, char *argv[]) {
    char *dir_path = NULL;
    int recursive = 0;
    char *filter = NULL;
    
    if (strcmp(argv[1], "variant") == 0) {
          printf("%d\n", 12345);
    }else{
    
             // Parse command line arguments
             for (int i = 1; i < argc; i++) {
               if (strcmp(argv[i], "list") == 0) {
                  // "list" option specified
                  i++;
                  if (i < argc && strcmp(argv[i], "recursive") == 0) {
                  // "recursive" option specified
                  recursive = 1;
                  i++;
                  }
                  if (i < argc && strncmp(argv[i], "path=", 5) == 0) {
                  // "path" option specified
                  dir_path = argv[i] + 5;
                  }
                  if (i < argc && strncmp(argv[i], "filter=", 7) == 0) {
                  // "filter" option specified
                  filter = argv[i] + 7;
                 }
              }
          }
        // Check that dir_path is not NULL
        if (dir_path == NULL) {
        printf("ERROR: Path not specified.\n");
           return 1;
        }
       // List the directory contents
       list_directory_contents(dir_path, recursive, filter);
       printf("SUCCESS\n");
       
    }
    return 0;
}
