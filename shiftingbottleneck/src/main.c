#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../include/main.h"

/**
 * Prints the Gantt chart for the job shop scheduling problem.
 * The function iterates through all machines and their operations, printing the job ID and start/end times for each operation.
 *
 * @param shop Pointer to the JobShop structure containing jobs and operations.
 */
void print_gantt_chart(const JobShop shop) {
    printf("\n\nGantt Chart (Textual):\n");

    for (int m = 0; m < shop.num_machines; ++m) {
        printf("Machine %d: ", m);
        for (int j = 0; j < shop.num_jobs; ++j) {
            for (int o = 0; o < shop.jobs[j].num_operations; ++o) {
                Operation op = shop.jobs[j].operations[o];
                if (op.machine_id == m) {
                    printf("[J%d %d-%d] ", op.job_id, op.start_time, op.end_time);
                }
            }
        }
        printf("\n");
    }
}

/**
 * Loads a sample instance of the job shop scheduling problem.
 * This function initializes the job shop with a predefined number of jobs and machines,
 */
void load_instance(JobShop* shop) {
    shop->num_jobs = 3;
    shop->num_machines = 3;

    // job_id | machine_id | duration | start_time | end_time;
    shop->jobs[0].operations[0] = (Operation) { 0, 0, 3, -1, -1 };
    shop->jobs[0].operations[1] = (Operation) { 0, 1, 2, -1, -1 };
    shop->jobs[0].operations[2] = (Operation) { 0, 2, 2, -1, -1 };
    shop->jobs[0].num_operations = 3;

    // Job 1
    // job_id | machine_id | duration | start_time | end_time; [designated initialization or compound literal initialization]
    shop->jobs[1].operations[0] = (Operation) { 1, 1, 2, -1, -1 };
    shop->jobs[1].operations[1] = (Operation) { 1, 2, 1, -1, -1 };
    shop->jobs[1].operations[2] = (Operation) { 1, 0, 4, -1, -1 };
    shop->jobs[1].num_operations = 3;

    // Job 2
    shop->jobs[2].operations[0] = (Operation) { 2, 2, 4, -1, -1 };
    shop->jobs[2].operations[1] = (Operation) { 2, 0, 3, -1, -1 };
    shop->jobs[2].operations[2] = (Operation) { 2, 1, 2, -1, -1 };
    shop->jobs[2].num_operations = 3;
}
/**
 * Initializes the machine schedules by setting the number of operations to zero for each machine.
 * The function iterates through all machines and sets their operation count to zero.
 *
 * @param shop Pointer to the JobShop structure containing jobs and operations.
 * @param machines Pointer to an array of MachineSchedule structures representing the schedules of each machine.
 */
void initialize_machines_schedules(JobShop* shop, MachineSchedule* machines) {
    for (int m = 0; m < shop->num_machines; ++m) {
        machines[m].num_operations = 0;
    }
}
/**
 * Assigns operations to their respective machines in the machine schedules.
 * The function iterates through all jobs and their operations, assigning each operation to the corresponding machine schedule.
 *
 * @param shop Pointer to the JobShop structure containing jobs and operations.
 * @param machines Pointer to an array of MachineSchedule structures representing the schedules of each machine.
 */
void assing_operations_to_machines(JobShop* shop, MachineSchedule* machines) {
    for (int j = 0; j < shop->num_jobs; ++j) {
        for (int o = 0; o < shop->jobs[j].num_operations; ++o) {
            Operation op = shop->jobs[j].operations[o];
            machines[op.machine_id].operations[machines[op.machine_id].num_operations++] = op;
        }
    }
}
/**
 * Computes the earliest start times for each operation in the job shop scheduling problem.
 * The function iterates through all jobs and their operations, updating the start and end times based on job order and machine order constraints.
 * The function continues to iterate until no further updates are made to the start and end times.
 *
 * @param shop Pointer to the JobShop structure containing jobs and operations.
 * @param machines Pointer to an array of MachineSchedule structures representing the schedules of each machine.
 */
void compute_earliest_start_times(JobShop* shop, MachineSchedule* machines) {
    // Reset all start and end times
    for (int j = 0; j < shop->num_jobs; ++j) {
        for (int o = 0; o < shop->jobs[j].num_operations; ++o) {
            shop->jobs[j].operations[o].start_time = -1;
            shop->jobs[j].operations[o].end_time = -1;
        }
    }

    bool updated;
    do {
        updated = false;

        for (int j = 0; j < shop->num_jobs; ++j) {
            for (int o = 0; o < shop->jobs[j].num_operations; ++o) {
                Operation* op = &shop->jobs[j].operations[o];

                int earliest_start = 0;
                // Job order constraint
                if (o > 0) {
                    Operation* prev = &shop->jobs[j].operations[o - 1];
                    if (prev->end_time == -1) continue;
                    if (prev->end_time > earliest_start)
                        earliest_start = prev->end_time;
                }
                // Machine order constraint
                MachineSchedule* ms = &machines[op->machine_id];
                for (int idx = 0; idx < ms->num_operations; ++idx) {
                    if (ms->operations[idx].job_id == op->job_id &&
                        ms->operations[idx].machine_id == op->machine_id) {

                        if (idx > 0) {
                            Operation* prev_machine_op = &ms->operations[idx - 1];

                            // Lookup real pointer to get the correct end time
                            Operation* real_prev_op = NULL;
                            for (int pj = 0; pj < shop->num_jobs; ++pj) {
                                for (int po = 0; po < shop->jobs[pj].num_operations; ++po) {
                                    Operation* candidate = &shop->jobs[pj].operations[po];
                                    if (candidate->job_id == prev_machine_op->job_id &&
                                        candidate->machine_id == prev_machine_op->machine_id) {
                                        real_prev_op = candidate;
                                        break;
                                    }
                                }
                            }

                            if (!real_prev_op || real_prev_op->end_time == -1)
                                continue;

                            if (real_prev_op->end_time > earliest_start)
                                earliest_start = real_prev_op->end_time;
                        }

                        break;
                    }
                }

                // If not set or can be improved
                if (op->start_time == -1 || op->start_time != earliest_start) {
                    op->start_time = earliest_start;
                    op->end_time = earliest_start + op->duration;
                    updated = true;
                }
            }
        }
    } while (updated);
}

/**
 * Compares two IndexedOperation structures first by job_id and then by job_index.
 * This function is used for sorting the operations in the machine schedule.
 *
 * @param a Pointer to the first IndexedOperation structure.
 * @param b Pointer to the second IndexedOperation structure.
 * @return An integer less than, equal to, or greater than zero if a is found to be less than,
 *         equal to, or greater than b, respectively.
 */
int compare_by_job_and_index(const void* a, const void* b) {
    IndexedOperation* op_a = (IndexedOperation*)a;
    IndexedOperation* op_b = (IndexedOperation*)b;

    if (op_a->job_id != op_b->job_id) {
        return op_a->job_id - op_b->job_id;
    } else {
        return op_a->job_index - op_b->job_index;
    }
}
/**
 * Generates a placeholder priority for the operations in the job shop scheduling problem.
 * The function flattens the operations into a single array and sorts them by job ID and operation index.
 *
 * @param shop Pointer to the JobShop structure containing jobs and operations.
 * @param out_ops Pointer to an array of IndexedOperation structures to store the flattened operations.
 * @return The number of operations generated.
 */
int generate_placeholder_priority(JobShop* shop, IndexedOperation* out_ops) {
    int count = 0;
    for (int j = 0; j < shop->num_jobs; ++j) {
        for (int o = 0; o < shop->jobs[j].num_operations; ++o) {
            out_ops[count++] = (IndexedOperation) { j, o, &shop->jobs[j].operations[o] };
        }
    }

    qsort(out_ops, count, sizeof(IndexedOperation), compare_by_job_and_index);
    return count;
}
/**
 * Schedules operations from left to right based on their earliest start times.
 * The function iterates through the operations and assigns start and end times based on the last job and machine end times.
 *
 * @param flat_ops Pointer to an array of IndexedOperation structures representing the operations to be scheduled.
 * @param num_flat_ops The number of operations in the flat_ops array.
 * @param num_jobs The number of jobs in the job shop.
 * @param num_machines The number of machines in the job shop.
 */
void schedule_left_to_right(IndexedOperation flat_ops[], int num_flat_ops, int num_jobs, int num_machines) {
    int last_job_end[MAX_JOBS] = { 0 };
    int last_machine_end[MAX_MACHINES] = { 0 };

    for (int i = 0; i < num_flat_ops; ++i) {
        Operation* op = flat_ops[i].op;

        int job_id = flat_ops[i].job_index;
        int machine_id = op->machine_id;

        int earliest_start = last_job_end[job_id] > last_machine_end[machine_id]
            ? last_job_end[job_id]
            : last_machine_end[machine_id];

        op->start_time = earliest_start;
        op->end_time = earliest_start + op->duration;

        last_job_end[job_id] = op->end_time;
        last_machine_end[machine_id] = op->end_time;
    }
}

int main() {
    JobShop shop;
    MachineSchedule machines[MAX_MACHINES];
    
    // Load jobs and operations (manual for now, file-based later)
    load_instance(&shop);
    initialize_machines_schedules(&shop, machines);
    assing_operations_to_machines(&shop, machines);

    printf("Initialized machine schedules with operation counts:\n");
    for (int i = 0; i < shop.num_machines; i++) {
        printf("Machine %d: %d operations\n", i, machines[i].num_operations);
    }

    //## Testing Scheduling respecting only job order##

    bool scheduled_machines[MAX_MACHINES] = { false }; //  unscheduled all machines (for now)

    compute_earliest_start_times(&shop, machines);
    printf("\nEarliest start times:\n");
    for (int j = 0; j < shop.num_jobs; ++j) {
        printf("Job %d:\n", j);
        for (int o = 0; o < shop.jobs[j].num_operations; ++o) {
            Operation op = shop.jobs[j].operations[o];
            printf("  Op on M%d: start=%d, end=%d\n", op.machine_id, op.start_time, op.end_time);
        }
    }
    //## Testing opperations priority ##
    IndexedOperation flat_ops[MAX_JOBS * MAX_OPERATIONS];

    int num_flat_ops = generate_placeholder_priority(&shop, flat_ops);

    printf("\nFlat operations sorted by job and index:\n");
    for (int i = 0; i < num_flat_ops; ++i) {
        Operation* op = flat_ops[i].op;
        printf("Job %d, Op %d (Machine %d, Duration %d)\n",
            flat_ops[i].job_id, flat_ops[i].job_index, op->machine_id, op->duration);
    }
    //## Testing scheduling ##
    printf("\nScheduling operations left to right:\n");
    schedule_left_to_right(flat_ops, num_flat_ops, shop.num_jobs, shop.num_machines);

    print_gantt_chart(shop);
    return 0;
}

