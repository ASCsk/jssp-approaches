#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "ssms.h"

int solve_single_machine_subproblem_naive(OperationNode* nodes, int* ops_on_machine, int num_ops, int* best_sequence) {
    // Naive sequence: preserve order of ops_on_machine
    for (int i = 0; i < num_ops; i++) {
        best_sequence[i] = i;
    }

    // Compute makespan using ESTs
    int current_time = 0;
    for (int i = 0; i < num_ops; i++) {
        int op_index = ops_on_machine[i];
        current_time += nodes[op_index].duration;
    }

    return current_time;
}


// Global pointers to track best solution during permutation
static int* best_perm = NULL;
static int best_makespan;
static OperationNode* global_nodes = NULL;  // global access to nodes array

// Helper: evaluate makespan of a permutation
static int evaluate_permutation(OperationNode* nodes, int* ops_on_machine, const int* perm, int n) {
    int job_ready[MAX_JOBS] = { 0 };      // When each job is ready for its next op
    int machine_ready = 0;                // When the machine is ready for the next op
    int current_time = 0;
    for (int i = 0; i < n; i++) {
        int op_idx = ops_on_machine[perm[i]];
        OperationNode* op = &nodes[op_idx];
        // The operation can start when both the job and the machine are ready
        int est = job_ready[op->job_id] > machine_ready ? job_ready[op->job_id] : machine_ready;
        if (est < op->earliest_start)
            est = op->earliest_start;
        int end = est + op->duration;
        job_ready[op->job_id] = end;
        machine_ready = end;
        current_time = end;
    }
    return current_time;
}

// Helper: permute indices and track best
static void permute(OperationNode* nodes, int* ops_on_machine, int* arr, int start, int n, int* best_makespan, int* best_perm) {
    if (start == n) {

        // Print the current permutation (local indices and global node indices)
        printf("Permutation (local): ");
        for (int i = 0; i < n; ++i) printf("%d ", arr[i]);
        printf("| Global: ");
        for (int i = 0; i < n; ++i) printf("%d ", ops_on_machine[arr[i]]);

        int makespan = evaluate_permutation(nodes, ops_on_machine, arr, n);
        printf("makespan: %d\n", makespan);
        
        if (makespan < *best_makespan) {
            *best_makespan = makespan;
            memcpy(best_perm, arr, n * sizeof(int));
        }
        return;
    }
    for (int i = start; i < n; i++) {
        int tmp = arr[start]; arr[start] = arr[i]; arr[i] = tmp;
        permute(nodes, ops_on_machine, arr, start + 1, n, best_makespan, best_perm);
        tmp = arr[start]; arr[start] = arr[i]; arr[i] = tmp; // backtrack
    }
}

/**
 * Solve the single-machine sequencing subproblem by brute force,
 * using indices into the global OperationNode array.
 *
 * @param nodes Global array of OperationNode
 * @param ops_on_machine Array of indices of operations on the machine
 * @param n Number of operations on machine
 * @param best_sequence Output: array of indices into global nodes for best sequence (preallocated)
 * @return best makespan found
 */
int solve_single_machine_subproblem_bf(OperationNode* nodes, int* ops_on_machine, int num_ops, int* best_sequence) {
    if (num_ops == 0) return 0;

    int indices[MAX_OPS_PER_MACHINE];
    int best_perm[MAX_OPS_PER_MACHINE];
    int best_makespan = INT_MAX;

    for (int i = 0; i < num_ops; ++i)
        indices[i] = i;

    permute(nodes, ops_on_machine, indices, 0, num_ops, &best_makespan, best_perm);

    memcpy(best_sequence, best_perm, num_ops * sizeof(int));
    return best_makespan;
}
