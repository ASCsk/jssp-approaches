#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

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
int** load_and_print_jssp_matrix(const char* jss_filename, int* num_jobs, int* num_machines) {
    int** matrix = load_jssp_matrix(jss_filename, num_jobs, num_machines);

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

/* 
    This assumes that the previous operation has already been scheduled. 
    We’ll need to schedule jobs in topological order later (Shifting Bottleneck handles that).
*/ 
/**
 * Computes the earliest start times (EST) for each operation in the schedule.
 * This function iterates through the operations and calculates the earliest start time based on the previous operation's end time.
 * @param schedule is the schedule containing the jobs and their operations.
 * @param ops is an array of MachineOpStub structures containing the operations to compute EST for.
 * @param count is the number of operations in the ops array.
 */
void compute_earliest_start_times(Schedule* schedule, MachineOpStub* ops, int count) {
    for (int i = 0; i < count; ++i) {
        int job_id = ops[i].job_id;
        int op_index = ops[i].op_index;

        if (op_index == 0) {
            ops[i].est = 0;  // first op in the job
        }
        else {
            Operation* prev_op = &schedule->jobs[job_id].ops[op_index - 1];
            int prev_machine = prev_op->machine_id;

            // Find when the previous operation finishes on its machine
            MachineSchedule* ms = &schedule->machine_schedules[prev_machine];
            for (int j = 0; j < ms->count; ++j) {
                ScheduledOp* s = &ms->scheduled_op[j];
                if (s->job_id == job_id && s->op_index == op_index - 1) {
                    ops[i].est = s->end_time;
                    break;
                }
            }
        }
    }
}

// Basic sort
/**
 * Compares two MachineOpStub structures based on their earliest start time (EST).
 */
int compare_by_est(const void* a, const void* b) {
    MachineOpStub* op1 = (MachineOpStub*)a;
    MachineOpStub* op2 = (MachineOpStub*)b;
    return op1->est - op2->est;
}

/**
 * Schedules operations for a specific machine in the schedule.
 * This function takes a schedule, a machine ID, and an array of MachineOpStub structures, and schedules the operations
 * on the specified machine. It sorts the operations by their earliest start time (EST) and assigns them to the machine's schedule.
 * @param schedule is the schedule containing the jobs and their operations.
 * @param machine_id is the ID of the machine to schedule operations for.
 * @param ops is an array of MachineOpStub structures containing the operations to be scheduled.
 * @param count is the number of operations in the ops array.
 */
void schedule_machine_ops(Schedule* schedule, int machine_id, MachineOpStub* ops, int count) {
    // Retrieve the schedule for the specific machine
    MachineSchedule* ms = &schedule->machine_schedules[machine_id];

    /**
     * Sort the operations by their earliest start time (EST) using the compare_by_est function
     * It returns the difference: op1->est - op2->est.
     *  - If the result is negative, op1 comes before op2 in the sorted order.
     *  - If the result is positive, op2 comes before op1.
     *  - If the result is zero, the two elements are considered equal in terms of sorting.
    */
    qsort(ops, count, sizeof(MachineOpStub), compare_by_est);

    int current_time = 0;

    // Iterate through the sorted operations
    for (int i = 0; i < count; ++i) {
        // Determine the start time for the operation
        // If the earliest start time (ops[i].est) is greater than the current time, use it
        // Otherwise, use the current time
        int start_time = (ops[i].est > current_time) ? ops[i].est : current_time;

        // Calculate the end time by adding the operation's duration to the start time
        int end_time = start_time + ops[i].duration;

        // Create an actual ScheduledOp structure to represent the scheduled operation
        ScheduledOp s = {
            .job_id = ops[i].job_id,
            .op_index = ops[i].op_index,
            .start_time = start_time,
            .end_time = end_time
        };

        // Add the scheduled operation to the machine's schedule
        ms->scheduled_op[ms->count++] = s;

        // Update the current time to the end time of the scheduled operation
        current_time = end_time;
    }
}

// Next steps:



// ## DEBUG FUNCTIONS ##

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
        printf("Job %d, Op %d, Duration %d, est %d\n", ops[i].job_id, ops[i].op_index, ops[i].duration, ops[i].est);
    }
}

void debug_print_machine_schedule(Schedule* schedule, int machine_id) {
    MachineSchedule* ms = &schedule->machine_schedules[machine_id];
    printf("\nSchedule for Machine %d:\n", machine_id);

    for (int i = 0; i < ms->count; ++i) {
        ScheduledOp* s = &ms->scheduled_op[i];
        printf("  Job %d - Op %d: Start %d, End %d (Duration %d)\n",
            s->job_id, s->op_index, s->start_time, s->end_time,
            s->end_time - s->start_time);
    }
}

void validate_machine_schedule(const Schedule* schedule, int machine_id) {
    const MachineSchedule* ms = &schedule->machine_schedules[machine_id];
    int last_end_time = -1;

    for (int i = 0; i < ms->count; ++i) {
        const ScheduledOp* s = &ms->scheduled_op[i];
        const Operation* op = &schedule->jobs[s->job_id].ops[s->op_index];

        // Assert correct duration
        assert(s->end_time - s->start_time == op->duration);

        // Assert no overlap
        assert(s->start_time >= last_end_time);

        last_end_time = s->end_time;
    }

    printf("\nSchedule for machine %d is valid.\n", machine_id);
}

int main() {

    Schedule schedule;

    MachineOpStub ops[MAX_OPS_PER_MACHINE]; 

    const char* jss_filename = "ft06.jss";

    int num_jobs = 0, num_machines = 0;
    int** matrix = load_and_print_jssp_matrix(jss_filename, &num_jobs, &num_machines);

    printf("\n");

    load_matrix_into_schedule(matrix, num_jobs, num_machines, &schedule);

    free_matrix(matrix, num_jobs);

    debug_print_loaded_schedule(&schedule);

    int machine_id = 0;  
    int count = 0;

    collect_ops_for_machine(&schedule, machine_id, ops, &count);

    debug_print_collected_ops(ops, count);

    compute_earliest_start_times(&schedule, ops, count);  // initial EST = 0 for now

    schedule_machine_ops(&schedule, machine_id, ops, count);

    debug_print_machine_schedule(&schedule, machine_id);

    validate_machine_schedule(&schedule, machine_id);


    return 0;
}

