#include <stdio.h>

#define MAX_JOBS 10
#define MAX_MACHINES 10
#define MAX_OPERATIONS 20

typedef struct
{
    int job_id;
    int machine_id;
    int duration;
    int start_time;
    int end_time;
} Operation;

typedef struct
{
    Operation operations[MAX_MACHINES];
    int num_operations;
} Job;

typedef struct
{
    Job jobs[MAX_JOBS];
    int num_jobs;
    int num_machines;
} JobShop;

void load_instance(JobShop *shop)
{
    shop->num_jobs = 3;
    shop->num_machines = 3;

    // Job 0
    shop->jobs[0].operations[0] = (Operation){0, 0, 3, -1, -1};
    shop->jobs[0].operations[1] = (Operation){0, 1, 2, -1, -1};
    shop->jobs[0].operations[2] = (Operation){0, 2, 2, -1, -1};
    shop->jobs[0].num_operations = 3;

    // Job 1
    shop->jobs[1].operations[0] = (Operation){1, 1, 2, -1, -1};
    shop->jobs[1].operations[1] = (Operation){1, 2, 1, -1, -1};
    shop->jobs[1].operations[2] = (Operation){1, 0, 4, -1, -1};
    shop->jobs[1].num_operations = 3;

    // Job 2
    shop->jobs[2].operations[0] = (Operation){2, 2, 4, -1, -1};
    shop->jobs[2].operations[1] = (Operation){2, 0, 3, -1, -1};
    shop->jobs[2].operations[2] = (Operation){2, 1, 2, -1, -1};
    shop->jobs[2].num_operations = 3;
}

void print_instance(const JobShop *shop)
{
    for (int j = 0; j < shop->num_jobs; ++j)
    {
        printf("\n\nJob %d:\n", j);
        for (int o = 0; o < shop->jobs[j].num_operations; ++o)
        {
            Operation op = shop->jobs[j].operations[o];
            printf("  Machine %d, Duration %d\n", op.machine_id, op.duration);
        }
    }
}

int main()
{
    JobShop shop;
    load_instance(&shop);
    print_instance(&shop);
    return 0;
}
