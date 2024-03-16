#include <stdio.h>
#include "yang/nbh.h" // assuming yang.h contains declarations for functions defined in yang.c
#include "yang/nbhextract.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s input_file output_file\n", argv[0]);
        return 1;
    }

    char *input_file = argv[1];
    char *output_file = argv[2];

    // Call function from yang.c with input_file and output_file
    int result = process_files(input_file, output_file);
    if (result != 0) {
        printf("Error processing files\n");
        return 1;
    }

    printf("Files processed successfully\n");
    return 0;
}
