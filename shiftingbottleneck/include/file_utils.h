#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#define JSSP_ROOT "../../jssp/"

#define MAX_FILENAME_LEN 64
#define PATH_LEN 64

typedef struct {
    char filename[MAX_FILENAME_LEN];
    int optimum_value;
} OptimumEntry;

int** load_jssp_matrix(const char* filename, int* num_jobs, int* num_machines, int* optimum_out);
int read_optimum_file(const char* path, const char* target_filename, int* optimum_out);
void print_matrix(int** matrix, int rows, int cols);
void free_matrix(int** matrix, int rows);
void print_current_working_directory();

#endif
