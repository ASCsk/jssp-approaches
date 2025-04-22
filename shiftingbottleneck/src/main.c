#include <stdio.h>
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

void initialize_machines_schedules(JobShop* shop, MachineSchedule* machines) {
    for (int m = 0; m < shop->num_machines; ++m) {
        machines[m].num_operations = 0;
    }
}

void assing_operations_to_machines(JobShop* shop, MachineSchedule* machines) {
    for (int j = 0; j < shop->num_jobs; ++j) {
        for (int o = 0; o < shop->jobs[j].num_operations; ++o) {
            Operation op = shop->jobs[j].operations[o];
            machines[op.machine_id].operations[machines[op.machine_id].num_operations++] = op;
        }
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

    return 0;
}


