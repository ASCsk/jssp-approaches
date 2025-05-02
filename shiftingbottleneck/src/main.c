#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "file_utils.h"
#include "main.h"

/**
 * Prints the current working directory to the console.
 * Checks if the number of jobs and machines exceeds the maximum allowed values.
 * If the matrix is successfully loaded, it prints the matrix.
 * This function uses the getcwd function to retrieve the current working directory and prints it.
 * @param jss_filename is the name of the jss file to load.
 */
int** load_and_print_jssp_matrix(const char* jss_filename, int* num_jobs, int* num_machines) {
    int** matrix = load_jssp_matrix(jss_filename, num_jobs, num_machines);

    // Check if the number of jobs or machines exceeds the maximum allowed values
    if (*num_jobs > MAX_JOBS) {
        fprintf(stderr, "Error: Number of jobs exceeds the maximum allowed (%d). Found %d jobs.\n", MAX_JOBS, *num_jobs);
        exit(1);
    }

    if (*num_machines > MAX_MACHINES) {
        fprintf(stderr, "Error: Number of machines exceeds the maximum allowed (%d). Found %d machines.\n", MAX_MACHINES, *num_machines);
        exit(1);
    }

    if (matrix) {
        printf("Loaded JSSP matrix: %d jobs, %d machines\n", *num_jobs, *num_machines);
        print_matrix(matrix, *num_jobs, *num_machines * 2);
        // free_matrix(matrix, *num_jobs); <-- Commenting this because it will invalidate the conversion to static_matrix
    }
    else {
        fprintf(stderr, "Failed to load JSSP matrix from '%s'\n", jss_filename);
    }
    return matrix;
}

int main() {

    const char* jss_filename = "ft06.jss";

    print_current_working_directory();

    int num_jobs = 0, num_machines = 0;
    int** matrix = load_and_print_jssp_matrix(jss_filename, &num_jobs, &num_machines);

    free_matrix(matrix, num_jobs);

    return 0;
}

