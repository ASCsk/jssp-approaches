#ifndef MAIN_H
#define MAIN_H

#define MAX_JOBS 10
#define MAX_MACHINES 10
#define MAX_OPERATIONS 20
#define MAX_OPS_PER_MACHINE 100


typedef struct {
    int job_id;
    int machine_id;
    int duration; //time units
    int start_time;
    int end_time;
} Operation;

typedef struct {
    int job_id;
    int job_index;
    Operation* op;
} IndexedOperation;

typedef struct {
    Operation operations[MAX_MACHINES];
    int num_operations;
} Job;

typedef struct {
    Operation operations[MAX_OPS_PER_MACHINE];
    int num_operations;
} MachineSchedule;

typedef struct {
    Job jobs[MAX_JOBS];
    int num_jobs;
    int num_machines;
} JobShop;

// Function prototypes
void load_instance(JobShop* shop);
void print_instance(const JobShop* shop);

#endif // MAIN_H