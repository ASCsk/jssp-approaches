#ifndef SSMS_H
#define SSMS_H

#include "main.h"

int solve_single_machine_subproblem_naive(OperationNode* nodes, int* ops_on_machine, int num_ops, int* best_sequence);

// Solves the sequencing subproblem for a single machine by brute force.
// ops: array of OperationNode for the machine
// n: number of operations
// best_sequence: output array of indices (length n) giving the best order
// Returns: best makespan found
int solve_single_machine_subproblem_bf(OperationNode* nodes, int* ops_on_machine, int n, int* best_sequence);

#endif // SSMS_H