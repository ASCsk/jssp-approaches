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

void assert_valid_edge(int from, int to) {
    if (from == to) {
        printf("Error: Attempted to add self-loop from Op %d to itself\n", from);
        exit(1);
    }
}

// Helper to get the index in nodes[] from job and op index
int op_node_index(int job, int num_machines, int op) {
    return job * num_machines + op;
}

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
    if (succ_id == op_node_index(node->job_id, node->op_index, MAX_MACHINES)) return;
    if (!has_successor(node, succ_id)) {
        node->successors[node->num_successors++] = succ_id;
    }
}

void add_predecessor_unique(OperationNode* node, int pred_id) {
    if (pred_id == op_node_index(node->job_id, node->op_index, MAX_MACHINES)) return;
    if (!has_predecessor(node, pred_id)) {
        node->predecessors[node->num_predecessors++] = pred_id;
    }
}

// This does not and should not contemplate the disjunctive edges from the beggining.
void build_disjunctive_graph(JSSPData* data, OperationNode* nodes, int num_operations) {
    int num_jobs = data->num_jobs;
    int num_machines = data->num_machines;

    // Initialize nodes with operations data
    for (int job = 0; job < num_jobs; job++) {
        for (int op = 0; op < num_machines; op++) {
            int idx = op_node_index(job, num_machines, op);
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
            int from = op_node_index(job, num_machines, op);
            int to = op_node_index(job, num_machines, (op + 1));

            assert_valid_edge(from, to);
            // from -> to
            add_successor_unique(&nodes[from], to);
            add_predecessor_unique(&nodes[to], from);
        }
    }

}

int find_bottleneck_machine(JSSPData* data, bool machine_scheduled[MAX_MACHINES]) {
    int max_makespan = -1;
    int bottleneck = -1;

    for (int m = 0; m < data->num_machines; ++m) {
        if (machine_scheduled[m])
            continue;  // Skip already scheduled machines

        // 1. Collect all operations assigned to machine m
        // 2. Sort or simulate to get a makespan (we can initially just sum durations as a proxy)
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

int count_operations_on_bottleneck_machine(OperationNode* nodes, int num_operations, int bottleneck_machine, int* ops_on_machine) {
    int count = 0;
    for (int i = 0; i < num_operations; i++) {
        if (nodes[i].machine == bottleneck_machine) {
            ops_on_machine[count++] = i;
        }
    }
    return count;
}

void orient_disjunctive_arcs(OperationNode* nodes,
    int machine_id,
    int num_operations,
    int* ops_on_machine,
    int num_ops,
    int* best_sequence) {

    for (int i = 0; i < num_ops - 1; i++) {
        int from = best_sequence[i];     
        int to = best_sequence[i + 1];   

        assert_valid_edge(from, to);

        add_successor_unique(&nodes[from], to);
        add_predecessor_unique(&nodes[to], from);
    }
}

void compute_earliest_start_times(OperationNode* nodes, int  num_operations) {
    int in_degree[MAX_OPERATIONS] = { 0 }; // How many unfinished predecessors does this op still have
    int queue[MAX_OPERATIONS];
    int front = 0, rear = 0;

    // Initialize in-degrees
    for (int i = 0; i < num_operations; ++i) {
        in_degree[i] = nodes[i].num_predecessors;
        nodes[i].earliest_start = 0; // Initialize EST
    }

    // Enqueue all nodes with zero in-degree
    for (int i = 0; i < num_operations; ++i) {
        if (in_degree[i] == 0) { // Find operations with no predecessors:
            queue[rear++] = i;
        }
    }

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

void compute_shifting_bottleneck(JSSPData* data, Schedule* sched) {
    int num_jobs = data->num_jobs;
    int num_machines = data->num_machines;
    int num_operations = num_jobs * num_machines;

    OperationNode nodes[MAX_OPERATIONS];
    bool machine_scheduled[MAX_MACHINES] = { false };
    build_disjunctive_graph(data, nodes, num_operations);

    // print_disjunctive_graph(nodes, num_operations); // DEBUG

    for (int scheduled = 0; scheduled < num_machines; scheduled++) {
        static GraphData graph = { .num_arcs = 0 };

        int bottleneck_machine = find_bottleneck_machine(data, machine_scheduled);

        printf("Step %d: Bottleneck = Machine %d\n", scheduled, bottleneck_machine);

        // Extract ops on bottleneck machine
        int ops_on_machine[MAX_OPERATIONS];
        int num_ops = count_operations_on_bottleneck_machine(nodes, num_operations, bottleneck_machine, ops_on_machine);
        if (num_ops == 0) {
            printf("No operations on bottleneck machine %d\n", bottleneck_machine);
            continue;
        }

        OperationNode ops_subset[num_ops];
        for (int i = 0; i < num_ops; i++) {
            ops_subset[i] = nodes[ops_on_machine[i]];
        }

        // print_ops_subset(ops_subset, num_ops); // DEBUG

        int best_sequence[num_ops];
        
        int makespan = solve_single_machine_subproblem_bf(nodes, ops_on_machine, num_ops, best_sequence);
        printf("Best sequence makespan on machine %d: %d\n", bottleneck_machine, makespan);

        validate_best_sequence(best_sequence, num_ops, num_operations); // DEBUG

        print_machine_sequence(bottleneck_machine, best_sequence, num_ops);  // DEBUG

        orient_disjunctive_arcs(nodes, bottleneck_machine, num_operations, ops_on_machine, num_ops, best_sequence);

        machine_scheduled[bottleneck_machine] = true;

        compute_earliest_start_times(nodes, num_operations);

        for (int i = 0; i < num_operations; i++) {
            printf("Op %2d (J%d, M%d): EST = %d\n",
                i, nodes[i].job_id, nodes[i].machine, nodes[i].earliest_start);
        }

    }

    fill_schedule_from_nodes(sched, nodes, data);
}


int main() {
    const char* jss_filename = "ft03.jss";

    int num_jobs = 0, num_machines = 0, optimum_value = -1;

    int** matrix = load_and_print_jssp_matrix(jss_filename, &num_jobs, &num_machines, &optimum_value);
    if (matrix == NULL) return EXIT_FAILURE;

    JSSPData data;

    initialize_schedule_data(matrix, num_jobs, num_machines, &data);

    free_matrix(matrix, num_jobs);

    // print_jssp_data(&data); // DEBUG

    Schedule sched;

    compute_shifting_bottleneck(&data, &sched);

    validate_schedule(&sched, &data);

    print_schedule(&sched, &data); // DEBUG

    return 0;
}
