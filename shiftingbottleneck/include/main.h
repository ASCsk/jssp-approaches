#ifndef MAIN_H
#define MAIN_H

#define MAX_JOBS 50
#define MAX_MACHINES 50
#define MAX_OPS_PER_MACHINE (MAX_JOBS)

typedef struct {
    int machine_id;
    int duration;
} Operation;

// Each job is a sequence of operations, in strict order.
typedef struct {
    Operation ops[MAX_MACHINES];
} Job;

// Scheduled operation on a machine with start/end timing.
typedef struct {
    int job_id;
    int op_index;       // Index in the job's ops array
    int start_time;
    int end_time;
} ScheduledOp;

// Each machine has a timeline of operations assigned to it.
typedef struct {
    ScheduledOp schedule[MAX_OPS_PER_MACHINE];
    int count; // How many operations are scheduled
} MachineSchedule;

// The overall schedule state
typedef struct {
    Job jobs[MAX_JOBS];
    MachineSchedule machine_schedules[MAX_MACHINES];
    int num_jobs;
    int num_machines;
} Schedule;

// Function prototypes
//...

#endif