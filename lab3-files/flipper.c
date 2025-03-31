#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

void reverse(FILE * input_file, FILE * output_file){
    int buffer_size = 256;
    char buffer[buffer_size];

    // while the next line exists
    while (fgets(buffer, buffer_size, input_file) != NULL) {

        // we need to reverse a line without "\n", so that "\n" remains in buffer at the last position of a line
        size_t length = strlen(buffer)-1;

        // reversing, using temporary variable
        for (size_t i = 0; i < length / 2; i++) {
            char temp = buffer[i];
            buffer[i] = buffer[length - i - 1];
            buffer[length - i - 1] = temp;
        }

        // write down reversed line to the output file
        fputs(buffer, output_file);
    }
}

/**
 * argv[0] - execution command - always passed as default
 * argv[1] - input file path
 * argv[2] - output file path
*/

int main(int argc, char** argv) {
    // we need 3 arguments to start: the source file, input and output directories
    if (argc != 3) {
        printf("Please, use: flipper input_directory output_directory\n");
        return 1;
    }

    // try to open input directory
    DIR* input_dir = opendir(argv[1]);
    if (input_dir == NULL) {
        printf("Error: Cannot open directory '%s'\n", argv[1]);
        return 1;
    }

    // try to open output directory
    DIR* output_dir = opendir(argv[2]);
    if (output_dir == NULL) {
        printf("Error: Cannot open directory '%s'\n", argv[2]);
        return 1;
    }

    struct dirent *entry;
    struct stat fileStat;

    while ((entry = readdir(input_dir)) != NULL){
        // check if the next element if file and if yes, check if it has .txt extension
        if (stat(entry->d_name, &fileStat) && strstr(entry->d_name, ".txt")){
            printf("%s\n", entry->d_name);

            // create a path to input file, which is located in an input directory
            char file_path[1024];
            snprintf(file_path, sizeof(file_path), "%s/%s", argv[1], entry->d_name);

            // try to open input file
            FILE* input_file = fopen(file_path, "r");
            if (input_file == NULL) {
                printf("Error: Cannot open input file '%s'\n", entry->d_name);
                return 1;
            }

            // create a path to output file, which will be created in an output directory
            snprintf(file_path, sizeof(file_path), "%s/%s", argv[2], entry->d_name);

            // try to open output file
            FILE* output_file = fopen(file_path, "w");
            if (output_file == NULL) {
                printf("Error: Cannot open output file '%s'\n", file_path);
                return 1;
            }

            // call reverse function
            reverse(input_file, output_file);

            // files will be freed automatically, but it is good practise to close it in the code to make the code clean
            fclose(input_file);
            fclose(output_file);
        }
    }

    // dirs will be freed automatically, but it is good practise to close it in the code to make it clean
    closedir(input_dir);
    closedir(output_dir);
}
