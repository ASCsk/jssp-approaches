#include "debug.h"

int** load_and_print_jssp_matrix(const char* jss_filename, int* num_jobs, int* num_machines, int* optimum_value) {
    int** matrix = load_jssp_matrix(jss_filename, num_jobs, num_machines, optimum_value);

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
        printf("Optimum makespan: %d\n", *optimum_value);
        print_matrix(matrix, *num_jobs, *num_machines * 2);
    }
    else {
        fprintf(stderr, "Failed to load JSSP matrix from '%s'\n", jss_filename);
    }
    return matrix;
}

void print_jssp_data(const JSSPData* data) {
    printf("JSSP Instance: %d jobs, %d machines\n", data->num_jobs, data->num_machines);
    for (int i = 0; i < data->num_jobs; ++i) {
        printf("Job %d:\n", i);
        for (int j = 0; j < data->num_machines; ++j) {
            printf("  Op %d: Machine %d, Duration %d\n",
                j,
                data->operations[i][j].machine,
                data->operations[i][j].duration);
        }
    }
}

void print_schedule(Schedule* sched, JSSPData* data) {
    printf("\n--- Final Schedule ---\n");
    for (int i = 0; i < data->num_jobs; ++i) {
        printf("Job %d:\n", i);
        for (int j = 0; j < data->num_machines; ++j) {
            Task t = data->operations[i][j];
            int start = sched->start_time[i][j];
            int end = sched->end_time[i][j];
            printf("  Op %d (Machine %d): Start=%2d End=%2d Duration=%2d\n",
                j, t.machine, start, end, t.duration);
        }
        printf("\n");
    }

    int makespan = 0;
    for (int i = 0; i < data->num_jobs; ++i) {
        for (int j = 0; j < data->num_machines; ++j) {
            if (sched->end_time[i][j] > makespan) {
                makespan = sched->end_time[i][j];
            }
        }
    }
    printf("Final makespan: %d\n", makespan);
}

void print_schedule_metrics(Schedule* sched, JSSPData* data) {
    int makespan = 0;
    int total_idle_time = 0;

    printf("=== Schedule Metrics ===\n");

    // Compute makespan by looking for the latest end time across all jobs and machines
    for (int i = 0; i < data->num_jobs; ++i) {
        for (int j = 0; j < data->num_machines; ++j) {
            if (sched->end_time[i][j] > makespan) {
                makespan = sched->end_time[i][j];
            }
        }
    }
    printf("Makespan: %d\n", makespan);

    // Per-machine metrics
    for (int m = 0; m < data->num_machines; ++m) {
        int busy_time = 0;

        // Find all tasks assigned to machine m
        for (int j = 0; j < data->num_jobs; ++j) {
            for (int o = 0; o < data->num_machines; ++o) {
                if (data->operations[j][o].machine == m) {
                    busy_time += data->operations[j][o].duration;
                }
            }
        }

        int idle_time = makespan - busy_time;
        total_idle_time += idle_time;
        float utilization = (makespan > 0) ? (100.0f * busy_time / makespan) : 0.0f;

        printf("Machine %d:\n", m);
        printf("  Busy time: %d\n", busy_time);
        printf("  Idle time: %d\n", idle_time);
        printf("  Utilization: %.2f%%\n", utilization);
    }

    printf("Total idle time (all machines): %d\n", total_idle_time);
}

void print_disjunctive_graph(OperationNode* nodes, int num_operations) {
    printf("\n=== Disjunctive Graph ===\n");

    for (int i = 0; i < num_operations; ++i) {
        OperationNode* node = &nodes[i];
        printf("Op %2d (Job %d, Op %d, Machine %d, Dur %2d):\n",
            i, node->job_id, node->op_index, node->machine, node->duration);

        printf("  Predecessors (%d): ", node->num_predecessors);
        for (int j = 0; j < node->num_predecessors; ++j) {
            printf("%d ", node->predecessors[j]);
        }
        printf("\n");

        printf("  Successors (%d):   ", node->num_successors);
        for (int j = 0; j < node->num_successors; ++j) {
            printf("%d ", node->successors[j]);
        }
        printf("\n\n");
    }
}

void print_disjunctive_candidates(const GraphData* data) {
    printf("GraphData: %d arcs\n", data->num_arcs);
    for (int i = 0; i < data->num_arcs; ++i) {
        printf("  Arc %3d: from %d to %d\n", i, data->arcs[i].from, data->arcs[i].to);
    }
    /**
     * Detect if the arcs really come in pairs (bidirectional).
     * Detect duplicates.
     * Print total distinct edges (half of arcs).
     */
    for (int i = 0; i < data->num_arcs; i += 2) {
        int from1 = data->arcs[i].from;
        int to1 = data->arcs[i].to;
        int from2 = data->arcs[i + 1].from;
        int to2 = data->arcs[i + 1].to;
        if (!(from1 == to2 && to1 == from2)) {
            printf("WARNING: arcs %d and %d are not symmetric!\n", i, i + 1);
        }
    }
}

void print_ops_subset(const OperationNode* ops_subset, int num_ops) {
    printf("ops_subset (num_ops = %d):\n", num_ops);
    for (int i = 0; i < num_ops; ++i) {
        printf("  [%2d] Job %d, Op %d, Machine %d, Duration %d\n",
            i,
            ops_subset[i].job_id,
            ops_subset[i].op_index,
            ops_subset[i].machine,
            ops_subset[i].duration
        );
    }
}

void validate_best_sequence(const int* best_sequence, int num_ops, int total_ops) {
    bool seen[MAX_OPERATIONS] = { false };
    for (int i = 0; i < num_ops; i++) {
        int idx = best_sequence[i];
        if (idx < 0 || idx >= total_ops) {
            printf("Invalid index %d in best_sequence at pos %d (total_ops = %d)\n", idx, i, total_ops);
            exit(1);
        }
        if (seen[idx]) {
            printf("Duplicate value %d in best_sequence\n", idx);
            exit(1);
        }
        seen[idx] = true;
    }
}


void print_machine_sequence(int machine_id, const int* best_sequence, int num_ops) {
    printf("Sequence for Machine %d: ", machine_id);
    for (int i = 0; i < num_ops; i++) {
        int node_idx = best_sequence[i];  // global index directly
        printf("%d ", node_idx);
    }
    printf("\n");
}

void validate_schedule(Schedule* sched, JSSPData* data) {
    // 1. Check precedence constraints within each job
    for (int j = 0; j < data->num_jobs; j++) {
        for (int o = 1; o < data->num_machines; o++) {
            int prev_end = sched->end_time[j][o - 1];
            int curr_start = sched->start_time[j][o];
            if (curr_start < prev_end) {
                printf("Precedence violation in Job %d: Op %d starts at %d before Op %d ends at %d\n",
                    j, o, curr_start, o - 1, prev_end);
                exit(1);
            }
        }
    }

    // 2. Check for machine conflicts
    for (int m = 0; m < data->num_machines; m++) {
        // Collect all tasks that run on machine m
        int op_count = 0;
        struct {
            int start;
            int end;
            int job;
            int op;
        } ops[MAX_JOBS];

        for (int j = 0; j < data->num_jobs; j++) {
            for (int o = 0; o < data->num_machines; o++) {
                Task t = data->operations[j][o];
                if (t.machine == m) {
                    ops[op_count].start = sched->start_time[j][o];
                    ops[op_count].end = sched->end_time[j][o];
                    ops[op_count].job = j;
                    ops[op_count].op = o;
                    op_count++;
                }
            }
        }

        // Check for overlaps (naive O(n^2))
        for (int i = 0; i < op_count; i++) {
            for (int j = i + 1; j < op_count; j++) {
                if (!(ops[i].end <= ops[j].start || ops[j].end <= ops[i].start)) {
                    printf("Machine conflict on Machine %d:\n", m);
                    printf("  Job %d Op %d [%d, %d] overlaps with Job %d Op %d [%d, %d]\n",
                        ops[i].job, ops[i].op, ops[i].start, ops[i].end,
                        ops[j].job, ops[j].op, ops[j].start, ops[j].end);
                    exit(1);
                }
            }
        }
    }

    printf("Schedule validated: no conflicts or precedence violations.\n");
}