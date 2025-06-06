#ifndef DEBUG_H
#define DEBUG_H

#include "main.h"

void print_jssp_data(const JSSPData* data);
void print_schedule(Schedule* sched, JSSPData* data);
void print_schedule_metrics(Schedule* sched, JSSPData* data);
int** load_and_print_jssp_matrix(const char* jss_filename, int* num_jobs, int* num_machines, int* optimum_value);
void print_disjunctive_graph(OperationNode* nodes, int num_operations);

#endif