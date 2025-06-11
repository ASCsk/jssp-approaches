#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "file_utils.h"
#include "debug.h"
#include "ssms.h"
#include "main.h"

void initialize_schedule_data(int** matrix, int num_jobs, int num_machines, JSSPData* data) {
    data->num_jobs = num_jobs;
    data->num_machines = num_machines;

    for (int i = 0; i < num_jobs; ++i) {
        for (int j = 0; j < num_machines; ++j) {
            data->operations[i][j].machine = matrix[i][2 * j];
            data->operations[i][j].duration = matrix[i][2 * j + 1];
        }
    }
}

int find_bottleneck_machine(JSSPData* data, bool machine_scheduled[MAX_MACHINES]) {
    int num_machines = data->num_machines;
    int max_makespan = -1;
    int bottleneck = -1;

    for (int m = 0; m < data->num_machines; ++m) {
        if (machine_scheduled[m])
            continue;  // Skip already scheduled machines

        // 1. Collect all operations assigned to machine m
        // 2. Sort or simulate to get a makespan (you can initially just sum durations as a proxy)
        // 3. Track max
        int machine_makespan = 0;

        for (int j = 0; j < data->num_jobs; ++j) {
            for (int o = 0; o < data->num_machines; ++o) {
                if (data->operations[j][o].machine == m) {
                    machine_makespan += data->operations[j][o].duration;
                    break; // Only one op per job per machine
                }
            }
        }

        if (machine_makespan > max_makespan) {
            max_makespan = machine_makespan;
            bottleneck = m;
        }
    }

    return bottleneck;
}

// int find_bottleneck_machine_by_est(OperationNode* nodes, int num_operations, int num_machines) {
//     int max_makespan = -1;
//     int bottleneck = -1;

//     for (int m = 0; m < num_machines; ++m) {
//         int makespan = 0;
//         for (int i = 0; i < num_operations; ++i) {
//             if (nodes[i].machine == m) {
//                 int finish = nodes[i].earliest_start + nodes[i].duration;
//                 if (finish > makespan) {
//                     makespan = finish;
//                 }
//             }
//         }
//         if (makespan > max_makespan) {
//             max_makespan = makespan;
//             bottleneck = m;
//         }
//     }
//     return bottleneck;
// }

bool has_successor(OperationNode* node, int succ_id) {
    for (int i = 0; i < node->num_successors; ++i) {
        if (node->successors[i] == succ_id)
            return true;
    }
    return false;
}

bool has_predecessor(OperationNode* node, int pred_id) {
    for (int i = 0; i < node->num_predecessors; ++i) {
        if (node->predecessors[i] == pred_id)
            return true;
    }
    return false;
}

void add_successor_unique(OperationNode* node, int succ_id) {
    if (!has_successor(node, succ_id)) {
        node->successors[node->num_successors++] = succ_id;
    }
}

void add_predecessor_unique(OperationNode* node, int pred_id) {
    if (!has_predecessor(node, pred_id)) {
        node->predecessors[node->num_predecessors++] = pred_id;
    }
}

// Helper to get the index in nodes[] from job and op index
int op_node_index(int job, int op, int num_machines) {
    return job * num_machines + op;
}
// This does not and should not contemplate the disjunctive edges from the beggining.
void build_disjunctive_graph(JSSPData* data, OperationNode* nodes, int num_operations) {
    int num_jobs = data->num_jobs;
    int num_machines = data->num_machines;

    // Initialize nodes with operations data
    for (int job = 0; job < num_jobs; job++) {
        for (int op = 0; op < num_machines; op++) {
            int idx = op_node_index(job, op, num_machines);
            Task t = data->operations[job][op];

            nodes[idx].job_id = job;
            nodes[idx].op_index = op;
            nodes[idx].machine = t.machine;
            nodes[idx].duration = t.duration;

            nodes[idx].num_successors = 0;
            nodes[idx].num_predecessors = 0;

            nodes[idx].earliest_start = 0;
            nodes[idx].latest_finish = 0;
        }
    }

    // Add conjunctive edges (job order)
    for (int job = 0; job < num_jobs; job++) {
        for (int op = 0; op < num_machines - 1; op++) {
            int from = op_node_index(job, op, num_machines);
            int to = op_node_index(job, op + 1, num_machines);

            // from -> to
            add_successor_unique(&nodes[from], to);
            add_predecessor_unique(&nodes[to], from);    
        }
    }
}

// void add_disjunctive_arcs(OperationNode* nodes, int num_operations, bool* machine_scheduled) {
//     for (int i = 0; i < num_operations; ++i) {
//         for (int j = i + 1; j < num_operations; ++j) {
//             // Same machine, not yet scheduled, different jobs
//             int m = nodes[i].machine;
//             if (m == nodes[j].machine &&
//                 !machine_scheduled[m] &&
//                 nodes[i].job_id != nodes[j].job_id) {

//                 // Add undirected disjunctive arcs (each operation sees the other as a possible successor)
//                 nodes[i].successors[nodes[i].num_successors++] = j;
//                 nodes[j].successors[nodes[j].num_successors++] = i;

//                 nodes[i].predecessors[nodes[i].num_predecessors++] = j;
//                 nodes[j].predecessors[nodes[j].num_predecessors++] = i;
//             }
//         }
//     }
// }

/**
 * Extract all operations assigned to machine m.
 *
 * @param nodes Global array of OperationNode
 * @param num_operations Total number of operations
 * @param m Machine ID to filter by
 * @param ops_on_machine Output array to store indices of operations on machine m (pre-allocated)
 * @return Number of operations assigned to machine m
 */
int extract_operations_on_machine(OperationNode* nodes, int num_operations, int m, int* ops_on_machine) {
    int count = 0;
    for (int i = 0; i < num_operations; i++) {
        if (nodes[i].machine == m) {
            ops_on_machine[count++] = i;
        }
    }
    return count;
}

/**
 * has_cycle validation stuff
 */
// bool dfs_visit(OperationNode* nodes, int index, bool* visited, bool* rec_stack) {
//     if (rec_stack[index]) return true;  // cycle detected
//     if (visited[index]) return false;   // already processed

//     visited[index] = true;
//     rec_stack[index] = true;

//     OperationNode* node = &nodes[index];
//     for (int i = 0; i < node->num_successors; ++i) {
//         int succ_idx = node->successors[i];
//         if (dfs_visit(nodes, succ_idx, visited, rec_stack)) {
//             return true;
//         }
//     }

//     rec_stack[index] = false;
//     return false;
// }

// bool has_cycle(OperationNode* nodes, int  num_operations) {
//     bool visited[MAX_OPERATIONS] = { false };
//     bool rec_stack[MAX_OPERATIONS] = { false };

//     for (int i = 0; i < num_operations; ++i) {
//         if (!visited[i]) {
//             if (dfs_visit(nodes, i, visited, rec_stack)) {
//                 return true;
//             }
//         }
//     }

//     return false;
// }

void compute_earliest_start_times(OperationNode* nodes, int  num_operations) {
    int in_degree[MAX_OPERATIONS] = { 0 };
    int queue[MAX_OPERATIONS];
    int front = 0, rear = 0;

    // Initialize in-degrees
    for (int i = 0; i < num_operations; ++i) {
        in_degree[i] = nodes[i].num_predecessors;
        nodes[i].earliest_start = 0; // Initialize EST
    }

    // Enqueue all nodes with zero in-degree
    for (int i = 0; i < num_operations; ++i) {
        if (in_degree[i] == 0) {
            queue[rear++] = i;
        }
    }

    // Kahn's algorithm
    while (front < rear) {
        int u_idx = queue[front++];
        OperationNode* u = &nodes[u_idx];

        for (int i = 0; i < u->num_successors; ++i) {
            int v_idx = u->successors[i];
            OperationNode* v = &nodes[v_idx];

            // Update earliest start time if needed
            int candidate_start = u->earliest_start + u->duration;
            if (candidate_start > v->earliest_start) {
                v->earliest_start = candidate_start;
            }

            // Decrease in-degree and enqueue if ready
            if (--in_degree[v_idx] == 0) {
                queue[rear++] = v_idx;
            }
        }
    }
}

void orient_disjunctive_arcs(OperationNode* nodes,
    int machine_id,
    int num_operations,
    int* ops_on_machine,
    int num_ops,
    int* best_sequence) {
    for (int i = 0; i < num_ops - 1; i++) {
        int from_idx = ops_on_machine[best_sequence[i]];
        int to_idx = ops_on_machine[best_sequence[i + 1]];

        add_successor_unique(&nodes[from_idx], to_idx);
        add_predecessor_unique(&nodes[to_idx], from_idx);
    }
}

void fill_schedule_from_nodes(Schedule* sched, OperationNode* nodes, JSSPData* data) {
    int num_jobs = data->num_jobs;
    int num_machines = data->num_machines;

    // Clear existing schedule times
    memset(sched->start_time, 0, sizeof(sched->start_time));
    memset(sched->end_time, 0, sizeof(sched->end_time));

    for (int i = 0; i < num_jobs * num_machines; i++) {
        OperationNode* op = &nodes[i];
        int job = op->job_id;
        int op_idx = op->op_index;

        int start = op->earliest_start;
        int end = start + op->duration;

        sched->start_time[job][op_idx] = start;
        sched->end_time[job][op_idx] = end;

        // Optional: update ready times if wanted (not strictly necessary here)
        if (sched->job_ready[job] < end) {
            sched->job_ready[job] = end;
        }
        if (sched->machine_ready[op->machine] < end) {
            sched->machine_ready[op->machine] = end;
        }
    }
}

void add_disjunctive_arc(GraphData* graph, int from, int to) {
    if (graph->num_arcs >= MAX_DISJ_ARCS) {
        // Optional: handle overflow
        return;
    }

    graph->arcs[graph->num_arcs].from = from;
    graph->arcs[graph->num_arcs].to = to;
    graph->num_arcs++;
}

void store_disjunctive_candidates(OperationNode* nodes, int total_operations, GraphData* data) {
    static int machine_buckets[MAX_MACHINES][MAX_OPS_PER_MACHINE];
    static int bucket_sizes[MAX_MACHINES];
    // Reset bucket sizes
    for (int m = 0; m < MAX_MACHINES; m++) {
        bucket_sizes[m] = 0;
    }

    // Fill machine buckets
    for (int i = 0; i < total_operations; i++) {
        int machine = nodes[i].machine;
        int index = bucket_sizes[machine]++;
        machine_buckets[machine][index] = i;
    }

    // Store disjunctive arcs for each machine
    for (int m = 0; m < MAX_MACHINES; m++) {
        int size = bucket_sizes[m];

        for (int i = 0; i < size - 1; i++) {
            for (int j = i + 1; j < size; j++) {
                int op_i = machine_buckets[m][i];
                int op_j = machine_buckets[m][j];

                add_disjunctive_arc(data, op_i, op_j);
                add_disjunctive_arc(data, op_j, op_i);
            }
        }

        if (data->num_arcs >= MAX_DISJ_ARCS) {
            printf("Warning: graph arcs overflow!\n");
            return;
        }
    }
}


void compute_shifting_bottleneck(JSSPData* data, Schedule* sched) {
    int num_jobs = data->num_jobs;
    int num_machines = data->num_machines;
    int num_operations = num_jobs * num_machines;

    OperationNode nodes[MAX_OPERATIONS];
    bool machine_scheduled[MAX_MACHINES] = { false };
    build_disjunctive_graph(data, nodes, num_operations);

    for (int scheduled = 0; scheduled < num_machines; scheduled++) {
        static GraphData graph = { .num_arcs = 0 };

        // build_disjunctive_graph(data, nodes, num_operations);

        print_disjunctive_graph(nodes, num_operations); // DEBUG 

        store_disjunctive_candidates(nodes, num_operations, &graph);

        // print_disjunctive_candidates(&graph); // DEBUG <- correct untill here

        int bottleneck = find_bottleneck_machine(data, machine_scheduled);
        // int bottleneck = find_bottleneck_machine_by_est(nodes, num_operations, num_machines);
        printf("Step %d: Bottleneck = Machine %d\n", scheduled, bottleneck);

        // Extract ops on bottleneck machine
        int ops_on_machine[MAX_OPERATIONS];
        int num_ops = extract_operations_on_machine(nodes, num_operations, bottleneck, ops_on_machine);
        if (num_ops == 0) {
            printf("No operations on bottleneck machine %d\n", bottleneck);
            continue;
        }

        OperationNode ops_subset[num_ops];
        for (int i = 0; i < num_ops; i++) {
            ops_subset[i] = nodes[ops_on_machine[i]];
        }

        int best_sequence[num_ops];

        int makespan = solve_single_machine_subproblem(nodes, ops_on_machine, num_ops, best_sequence);
        printf("Best sequence makespan on machine %d: %d\n", bottleneck, makespan);

        orient_disjunctive_arcs(nodes, bottleneck, num_operations, ops_on_machine, num_ops, best_sequence);

        machine_scheduled[bottleneck] = true;

        compute_earliest_start_times(nodes, num_operations);

        for (int i = 0; i < num_operations; i++) {
            printf("Op %2d (J%d, M%d): EST = %d\n",
                i, nodes[i].job_id, nodes[i].machine, nodes[i].earliest_start);
        }

        // if (has_cycle(nodes, num_operations)) {
        //     printf("Cycle detected after scheduling machine %d, aborting.\n", bottleneck);
        //     return;
        // }

        // add_disjunctive_arcs(nodes, num_operations, machine_scheduled);

        // print_disjunctive_graph(nodes, num_operations); // DEBUG 
    }

    fill_schedule_from_nodes(sched, nodes, data);
}


int main() {
    const char* jss_filename = "ft06.jss";

    int num_jobs = 0, num_machines = 0, optimum_value = -1;

    int** matrix = load_and_print_jssp_matrix(jss_filename, &num_jobs, &num_machines, &optimum_value);
    if (matrix == NULL) return EXIT_FAILURE;

    JSSPData data;

    initialize_schedule_data(matrix, num_jobs, num_machines, &data);

    // print_jssp_data(&data); // <- correct untill here

    Schedule sched;
    compute_shifting_bottleneck(&data, &sched);

    print_schedule(&sched, &data);

    free_matrix(matrix, num_jobs);

    return 0;
}
