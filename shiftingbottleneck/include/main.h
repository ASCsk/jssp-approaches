#ifndef MAIN_H
#define MAIN_H

#define MAX_JOBS 50
#define MAX_MACHINES 50
#define MAX_OPS_PER_MACHINE (MAX_JOBS)
#define MAX_OPERATIONS (MAX_JOBS * MAX_MACHINES)
#define MAX_EDGES_PER_NODE (MAX_JOBS)  // upper bound on successors/predecessors
#define MAX_DISJ_ARCS (MAX_MACHINES * MAX_JOBS * (MAX_JOBS - 1))

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

typedef struct {
    int job_id;         // Job index (0 to num_jobs-1)
    int op_index;       // Operation index within job (0 to num_machines-1)
    int machine;        // Machine assigned to this operation
    int duration;       // Duration of the operation

    int successors[MAX_EDGES_PER_NODE];     // Indices of successor nodes in global array
    int num_successors;

    int predecessors[MAX_EDGES_PER_NODE];   // Indices of predecessor nodes in global array
    int num_predecessors;

    int earliest_start;    // For scheduling calculations
    int latest_finish;
} OperationNode;

// This way we have a list of MachineOps (length = num_machines) where each entry contains the indices of operations that run on that machine.
typedef struct {
    int machine_id;
    int op_indices[MAX_OPERATIONS];  // Indices into OperationNode[]
    int num_ops;
} MachineOps;

typedef struct {
    struct {
        int from;
        int to;
    } arcs[MAX_DISJ_ARCS];
    int num_arcs;
} GraphData;

// Function prototypes (soon)

#endif