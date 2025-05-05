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
    }
    else {
        fprintf(stderr, "Failed to load JSSP matrix from '%s'\n", jss_filename);
    }
    return matrix;
}

void load_matrix_into_schedule(int** matrix, int num_jobs, int num_machines, Schedule* schedule) {
    schedule->num_jobs = num_jobs;
    schedule->num_machines = num_machines;

    for (int job = 0; job < num_jobs; ++job) {
        for (int i = 0; i < num_machines * 2; i += 2) {
            int machine = matrix[job][i];
            int duration = matrix[job][i + 1];
            int op_index = i / 2;

            schedule->jobs[job].ops[op_index].machine_id = machine;
            schedule->jobs[job].ops[op_index].duration = duration;
        }
    }
}

void debug_print_loaded_schedule(Schedule* schedule) {
    printf("Loaded schedule:\n");
    for (int job = 0; job < schedule->num_jobs; ++job) {
        printf("Job %d: ", job);
        for (int op = 0; op < schedule->num_machines; ++op) {
            printf("Machine %d, Duration %d; ", schedule->jobs[job].ops[op].machine_id, schedule->jobs[job].ops[op].duration);
        }
        printf("\n");
    }
}

int main() {

    Schedule schedule;
    const char* jss_filename = "ft10.jss";

    print_current_working_directory();

    int num_jobs = 0, num_machines = 0;
    int** matrix = load_and_print_jssp_matrix(jss_filename, &num_jobs, &num_machines);

    printf("\n");

    load_matrix_into_schedule(matrix, num_jobs, num_machines, &schedule);

    free_matrix(matrix, num_jobs);

    debug_print_loaded_schedule(&schedule);

    

    return 0;
}

