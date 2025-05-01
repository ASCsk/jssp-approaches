#include <stdio.h>
#include <stdbool.h>
#include "../include/main.h"

// Manual for now, file-based later
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
 * Initializes the machine schedules by setting the number of operations to 0.
 * Iterate through all machines (0 to num_machines - 1) and set number of operations to 0
 */

void initialize_machines_schedules(JobShop* shop, MachineSchedule* machines) {
    for (int m = 0; m < shop->num_machines; ++m) {
        machines[m].num_operations = 0;
    }
}
/**
 * Assigns operations to the corresponding machine schedules.
 * Iterate through all jobs (0 to num_jobs - 1) and for each job, iterate through its operations (0 to num_operations - 1).
 * For each operation, assign it to the corresponding machine schedule based on its machine_id.
 */
void assing_operations_to_machines(JobShop* shop, MachineSchedule* machines) {
    for (int j = 0; j < shop->num_jobs; ++j) {
        for (int o = 0; o < shop->jobs[j].num_operations; ++o) {
            Operation op = shop->jobs[j].operations[o];
            machines[op.machine_id].operations[machines[op.machine_id].num_operations++] = op;
        }
    }
}

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

    // Print result
    printf("\nEarliest start times:\n");
    for (int j = 0; j < shop.num_jobs; ++j) {
        printf("Job %d:\n", j);
        for (int o = 0; o < shop.jobs[j].num_operations; ++o) {
            Operation op = shop.jobs[j].operations[o];
            printf("  Op on M%d: start=%d, end=%d\n", op.machine_id, op.start_time, op.end_time);
        }
    }

    return 0;
}
