#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "file_utils.h"
#include "main.h"

void print_jssp_data(const JSSPData* data);
void print_schedule(Schedule* sched, JSSPData* data);
void print_schedule_metrics(Schedule* sched, JSSPData* data);
int** load_and_print_jssp_matrix(const char* jss_filename, int* num_jobs, int* num_machines, int* optimum_value);
void print_disjunctive_graph(OperationNode* nodes, int num_operations);
void print_disjunctive_candidates(const GraphData* data);
void print_ops_subset(const OperationNode* ops_subset, int num_ops);
void validate_best_sequence(const int* best_sequence, int num_ops, int total_ops);
void print_machine_sequence(int machine_id, const int* best_sequence, int num_ops);

#endif