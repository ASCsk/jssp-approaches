#ifndef MAIN_H
#define MAIN_H

#define MAX_JOBS 50
#define MAX_MACHINES 50
#define MAX_OPS_PER_MACHINE (MAX_JOBS)

typedef struct {
    int machine;
    int duration;
} Task;

typedef struct {
    int num_jobs;
    int num_machines;
    Task operations[MAX_JOBS][MAX_MACHINES];
} JSSPData;

typedef struct {
    int start_time[MAX_JOBS][MAX_MACHINES];
    int end_time[MAX_JOBS][MAX_MACHINES];
    int job_ready[MAX_JOBS];
    int machine_ready[MAX_MACHINES];
} Schedule;

void parse_matrix_to_struct(int** matrix, JSSPData* data);
void compute_shifting_bottleneck(JSSPData* data, Schedule* sched);
void print_schedule(Schedule* sched, JSSPData* data);
void print_schedule_metrics(Schedule* sched, JSSPData* data);

#endif