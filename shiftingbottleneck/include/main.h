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

// Scheduled operation on a machine with start/end timing.
typedef struct {
    int job_id;
    int op_index;
    int duration;
    int est;  // earliest start time
} MachineOpStub;

// Each machine has a timeline of operations assigned to it.
typedef struct {
    ScheduledOp scheduled_op[MAX_OPS_PER_MACHINE];
    int count; // How many operations are scheduled
} MachineSchedule;

// The overall schedule state
typedef struct {
    Job jobs[MAX_JOBS];
    MachineSchedule machine_schedules[MAX_MACHINES];
    int num_jobs;
    int num_machines;
} Schedule;

typedef struct {
    int numVertices;
    int adj[MAX_OPS_PER_MACHINE][MAX_OPS_PER_MACHINE]; // adjacency matrix for simplicity
    int weight[MAX_OPS_PER_MACHINE][MAX_OPS_PER_MACHINE]; // edge weights
} Graph;

// Function prototypes

// Load file into matrix
int** load_jssp_matrix(const char* filename, int* num_jobs, int* num_machines);
// Load matrix into schedule
void load_matrix_into_schedule(int** matrix, int num_jobs, int num_machines, Schedule* schedule);
// Collect opperations for a single machine (by machine_id)
void collect_ops_for_machine(Schedule* schedule, int machine_id, MachineOpStub* ops, int* count);

void compute_earliest_start_times(Schedule* schedule, MachineOpStub* ops, int count);
// Compare two MachineOpStub structures based on their earliest start time (EST)
int compare_by_est(const void* a, const void* b);

void schedule_machine_ops(Schedule* schedule, int machine_id, MachineOpStub* ops, int count);

void validate_machine_schedule(const Schedule* schedule, int machine_id);

void build_disjunctive_graph(Graph* graph, const Schedule* schedule, int machine_id);

int find_critical_path(const Graph* graph);

int extract_critical_path(const Graph* graph, int path[MAX_OPS_PER_MACHINE]);


int longest_path(Graph* g);

int find_bottleneck_machine(Schedule* schedule);

// Debugging:
void debug_print_loaded_schedule(Schedule* schedule);

#endif