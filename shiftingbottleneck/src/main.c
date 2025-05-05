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

/**
 * Loads the JSSP matrix into the schedule structure.
 * This function takes a matrix of operations and their durations, and populates the schedule structure with this data.
 * It iterates through each job and each operation, assigning the machine ID and duration to the corresponding job's operation.
 * @param matrix is the matrix of operations and their durations.
 * @param num_jobs is the number of jobs in the matrix.
 * @param num_machines is the number of machines in the matrix.
 * @param schedule is a pointer to the Schedule structure to be populated.
 */
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


/**
 * Collects operations for a specific machine from the schedule.
 * This function takes a schedule and a machine ID, and populates an array of MachineOpStub structure with the operations
 * assigned to that machine. It also keeps track of the number of operations collected.
 * @param schedule is the schedule containing the jobs and their operations.
 * @param machine_id is the ID of the machine for which to collect operations.
 * @param ops is an array of MachineOpStub structures to be populated with the operations.
 * @param count is a pointer to an integer where the number of collected operations will be stored.
 */
void collect_ops_for_machine(Schedule* schedule, int machine_id, MachineOpStub* ops, int* count) {
    *count = 0;
    for (int job = 0; job < schedule->num_jobs; ++job) {
        for (int op_index = 0; op_index < schedule->num_machines; ++op_index) {
            Operation op = schedule->jobs[job].ops[op_index];
            if (op.machine_id == machine_id) {
                ops[*count].job_id = job;
                ops[*count].op_index = op_index;
                ops[*count].duration = op.duration;
                (*count)++;
                break; // One op per job for this machine
            }
        }
    }
}

// ## DEBUG FUNCTIONS ##

/**
 * Frees the allocated memory for the matrix.
 * This function takes a pointer to the matrix and the number of rows, and frees each row and then the matrix itself.
 * @param matrix is the matrix to be freed.
 * @param rows is the number of rows in the matrix.
 */
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

void debug_print_collected_ops(MachineOpStub* ops, int count) {
    printf("\nCollected operations:\n");
    for (int i = 0; i < count; ++i) {
        printf("Job %d, Op %d, Duration %d\n", ops[i].job_id, ops[i].op_index, ops[i].duration);
    }
}


int main() {

    Schedule schedule;

    MachineOpStub ops[MAX_OPS_PER_MACHINE]; 

    const char* jss_filename = "ft10.jss";

    int num_jobs = 0, num_machines = 0;
    int** matrix = load_and_print_jssp_matrix(jss_filename, &num_jobs, &num_machines);

    printf("\n");

    load_matrix_into_schedule(matrix, num_jobs, num_machines, &schedule);

    free_matrix(matrix, num_jobs);

    debug_print_loaded_schedule(&schedule);

    int machine_id = 0;  
    int count = 0;

    // collect_ops_for_machine(&schedule, machine_id, ops, &count);

    return 0;
}

