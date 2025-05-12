#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#define JSSP_ROOT "../../jssp/"

int** load_jssp_matrix(const char* filename, int* num_jobs, int* num_machines);
void print_matrix(int** matrix, int rows, int cols);
void free_matrix(int** matrix, int rows);
void print_current_working_directory();

#endif
