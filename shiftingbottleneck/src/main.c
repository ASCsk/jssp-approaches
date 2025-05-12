#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "file_utils.h"
#include "main.h"

/**
 * Prints the current working directory to the console.
 * Checks if the number of jobs and machines exceeds the maximum allowed values.
 * If the matrix is successfully loaded, it prints the matrix.
 * This function uses the getcwd function to retrieve the current working directory and prints it.
 * @param jss_filename is the name of the jss file to load.
 * @param num_jobs is a pointer to an integer where the number of jobs will be stored.
 * @param num_machines is a pointer to an integer where the number of machines will be stored.
 */
int** load_and_print_jssp_matrix(const char* jss_filename, int* num_jobs, int* num_machines, int* optimum_value) {
    int** matrix = load_jssp_matrix(jss_filename, num_jobs, num_machines, optimum_value);

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
        printf("Optimum makespan: %d\n", *optimum_value);
        print_matrix(matrix, *num_jobs, *num_machines * 2); 
    }
    else {
        fprintf(stderr, "Failed to load JSSP matrix from '%s'\n", jss_filename);
    }
    return matrix;
}

void compute_shifting_bottleneck(JSSPData* data, Schedule* sched) {
    // Initialize ready times
    for (int i = 0; i < data->num_jobs; i++) sched->job_ready[i] = 0;
    for (int m = 0; m < data->num_machines; m++) sched->machine_ready[m] = 0;

    for (int step = 0; step < data->num_machines; step++) {
        for (int i = 0; i < data->num_jobs; i++) {
            Task task = data->operations[i][step];
            int m = task.machine;

            int start = (sched->job_ready[i] > sched->machine_ready[m]) ?
                sched->job_ready[i] : sched->machine_ready[m];
            int end = start + task.duration;

            sched->start_time[i][step] = start;
            sched->end_time[i][step] = end;

            sched->job_ready[i] = end;
            sched->machine_ready[m] = end;
        }
    }
}

void print_schedule(Schedule* sched, JSSPData* data) {
    printf("\n--- Final Schedule ---\n");
    for (int i = 0; i < data->num_jobs; ++i) {
        printf("Job %d:\n", i);
        for (int j = 0; j < data->num_machines; ++j) {
            Task t = data->operations[i][j];
            int start = sched->start_time[i][j];
            int end = sched->end_time[i][j];
            printf("  Op %d (Machine %d): Start=%2d End=%2d Duration=%2d\n",
                j, t.machine, start, end, t.duration);
        }
        printf("\n");
    }
}

void print_schedule_metrics(Schedule* sched, JSSPData* data) {
    int makespan = 0;
    int total_idle_time = 0;

    printf("=== Schedule Metrics ===\n");

    // Compute makespan by looking for the latest end time across all jobs and machines
    for (int i = 0; i < data->num_jobs; ++i) {
        for (int j = 0; j < data->num_machines; ++j) {
            if (sched->end_time[i][j] > makespan) {
                makespan = sched->end_time[i][j];
            }
        }
    }
    printf("Makespan: %d\n", makespan);

    // Per-machine metrics
    for (int m = 0; m < data->num_machines; ++m) {
        int busy_time = 0;

        // Find all tasks assigned to machine m
        for (int j = 0; j < data->num_jobs; ++j) {
            for (int o = 0; o < data->num_machines; ++o) {
                if (data->operations[j][o].machine == m) {
                    busy_time += data->operations[j][o].duration;
                }
            }
        }

        int idle_time = makespan - busy_time;
        total_idle_time += idle_time;
        float utilization = (makespan > 0) ? (100.0f * busy_time / makespan) : 0.0f;

        printf("Machine %d:\n", m);
        printf("  Busy time: %d\n", busy_time);
        printf("  Idle time: %d\n", idle_time);
        printf("  Utilization: %.2f%%\n", utilization);
    }

    printf("Total idle time (all machines): %d\n", total_idle_time);
}


void load_matrix_into_schedule(int** matrix, int num_jobs, int num_machines) {
    JSSPData data;
    Schedule sched;

    data.num_jobs = num_jobs;
    data.num_machines = num_machines;

    for (int i = 0; i < num_jobs; ++i) {
        for (int j = 0; j < num_machines; ++j) {
            data.operations[i][j].machine = matrix[i][2 * j];
            data.operations[i][j].duration = matrix[i][2 * j + 1];
        }
    }

    compute_shifting_bottleneck(&data, &sched);
    print_schedule(&sched, &data);
    print_schedule_metrics(&sched, &data);
}




int main() {

    const char* jss_filename = "ft06.jss";

    int num_jobs = 0, num_machines = 0, optimum_value = -1;
    int** matrix = load_and_print_jssp_matrix(jss_filename, &num_jobs, &num_machines, &optimum_value);

    if (matrix == NULL) {
        return EXIT_FAILURE;
    }

    printf("\n");

    load_matrix_into_schedule(matrix, num_jobs, num_machines);

    free_matrix(matrix, num_jobs);

    return 0;
}
