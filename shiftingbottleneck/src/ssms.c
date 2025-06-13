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

static void swap(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

// Evaluate makespan of a permutation of ops indexed in global_nodes
// Returns makespan if valid, else INT_MAX
static int evaluate_permutation(int* perm, int n) {
    int current_time = 0;
    for (int i = 0; i < n; i++) {
        OperationNode* op = &global_nodes[perm[i]];
        if (current_time < op->earliest_start)
            current_time = op->earliest_start;
        current_time += op->duration;
    }
    return current_time;
}

// Generate all permutations and track best sequence
static void permute(int* arr, int start, int n) {
    if (start == n) {
        int makespan = evaluate_permutation(arr, n);
        if (makespan < best_makespan) {
            best_makespan = makespan;
            memcpy(best_perm, arr, n * sizeof(int));
        }
        return;
    }
    for (int i = start; i < n; i++) {
        swap(&arr[start], &arr[i]);
        permute(arr, start + 1, n);
        swap(&arr[start], &arr[i]);  // backtrack
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

    int* indices = malloc(num_ops * sizeof(int));
    if (!indices) return INT_MAX;  // Allocation failure fallback

    // Copy ops_on_machine into indices for permutation
    memcpy(indices, ops_on_machine, num_ops * sizeof(int));

    best_perm = best_sequence;
    best_makespan = INT_MAX;
    global_nodes = nodes;

    permute(indices, 0, num_ops);

    free(indices);
    return best_makespan;
}