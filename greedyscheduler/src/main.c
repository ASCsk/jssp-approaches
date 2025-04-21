#include <stdio.h>
#include "../include/main.h"

void load_instance(JobShop *shop)
{
    shop->num_jobs = 3;
    shop->num_machines = 3;

    // Job 0

    Operation temp_op;
    temp_op.job_id = 0;
    temp_op.machine_id = 0;
    temp_op.duration = 3;
    temp_op.start_time = -1;
    temp_op.end_time = -1;

    shop->jobs[0].operations[0] = temp_op;
    // job_id | machine_id | duration | start_time | end_time;
    // shop->jobs[0].operations[0] = (Operation){0, 0, 3, -1, -1};
    shop->jobs[0].operations[1] = (Operation){0, 1, 2, -1, -1};
    shop->jobs[0].operations[2] = (Operation){0, 2, 2, -1, -1};
    shop->jobs[0].num_operations = 3;

    // Job 1
    // job_id | machine_id | duration | start_time | end_time; [designated initialization or compound literal initialization]
    shop->jobs[1].operations[0] = (Operation){1, 1, 2, -1, -1};
    shop->jobs[1].operations[1] = (Operation){1, 2, 1, -1, -1};
    shop->jobs[1].operations[2] = (Operation){1, 0, 4, -1, -1};
    shop->jobs[1].num_operations = 3;

    // Job 2
    shop->jobs[2].operations[0] = (Operation){2, 2, 4, -1, -1};
    shop->jobs[2].operations[1] = (Operation){2, 0, 3, -1, -1};
    shop->jobs[2].operations[2] = (Operation){2, 1, 2, -1, -1};
    shop->jobs[2].num_operations = 3;
}

void print_instance(const JobShop *shop)
{
    for (int j = 0; j < shop->num_jobs; ++j)
    {
        printf("\n\nJob %d:\n", j);
        for (int o = 0; o < shop->jobs[j].num_operations; ++o)
        {
            Operation op = shop->jobs[j].operations[o];
            printf("  Machine %d, Duration %d\n", op.machine_id, op.duration);
        }
    }
}

void naive_schedule(JobShop *shop)
{
    int machine_available_time[MAX_MACHINES] = {0};
    int job_ready_time[MAX_JOBS] = {0};
    int next_op_index[MAX_JOBS] = {0};
    int total_operations = shop->num_jobs * shop->num_machines;

    for (int scheduled_ops = 0; scheduled_ops < total_operations; ++scheduled_ops)
    {
        // Naive loop: schedule next available operation for each job (in order)
        for (int j = 0; j < shop->num_jobs; ++j)
        {
            int op_index = next_op_index[j];
            if (op_index >= shop->jobs[j].num_operations)
                continue;

            Operation *op = &shop->jobs[j].operations[op_index];
            int machine = op->machine_id;

            // Schedule operation
            int start_time = (machine_available_time[machine] > job_ready_time[j])
                                 ? machine_available_time[machine]
                                 : job_ready_time[j];
            int end_time = start_time + op->duration;

            op->start_time = start_time;
            op->end_time = end_time;

            machine_available_time[machine] = end_time;
            job_ready_time[j] = end_time;
            next_op_index[j]++;
        }
    }

    // Print scheduled operations
    printf("\nScheduled Operations:\n");
    for (int j = 0; j < shop->num_jobs; ++j)
    {
        printf("Job %d:\n", j);
        for (int o = 0; o < shop->jobs[j].num_operations; ++o)
        {
            Operation op = shop->jobs[j].operations[o];
            printf("  Machine %d | Duration %d | Start %d | End %d\n",
                   op.machine_id, op.duration, op.start_time, op.end_time);
        }
        printf("\n");
    }
}

void analyze_machine_usage(const JobShop *shop)
{
    printf("Machine Schedules:\n");

    for (int m = 0; m < shop->num_machines; ++m)
    {
        printf("\nMachine %d:\n", m);
        int timeline[MAX_JOBS];  // Start times
        int end_times[MAX_JOBS]; // End times
        int job_ids[MAX_JOBS];   // Which job used it
        int op_count = 0;

        // Collect operations for this machine
        for (int j = 0; j < shop->num_jobs; ++j)
        {
            for (int o = 0; o < shop->jobs[j].num_operations; ++o)
            {
                Operation op = shop->jobs[j].operations[o];
                if (op.machine_id == m)
                {
                    timeline[op_count] = op.start_time;
                    end_times[op_count] = op.end_time;
                    job_ids[op_count] = j;
                    op_count++;
                }
            }
        }

        // Sort by start time (simple bubble sort)
        for (int i = 0; i < op_count - 1; ++i)
        {
            for (int k = 0; k < op_count - i - 1; ++k)
            {
                if (timeline[k] > timeline[k + 1])
                {
                    // Swap start
                    int tmp = timeline[k];
                    timeline[k] = timeline[k + 1];
                    timeline[k + 1] = tmp;
                    // Swap end
                    tmp = end_times[k];
                    end_times[k] = end_times[k + 1];
                    end_times[k + 1] = tmp;
                    // Swap job id
                    tmp = job_ids[k];
                    job_ids[k] = job_ids[k + 1];
                    job_ids[k + 1] = tmp;
                }
            }
        }

        // Print usage and idle
        int current_time = 0;
        for (int i = 0; i < op_count; ++i)
        {
            if (timeline[i] > current_time)
            {
                printf("  Idle from %d to %d (Duration: %d)\n",
                       current_time, timeline[i], timeline[i] - current_time);
            }
            printf("  Used by Job %d from %d to %d\n",
                   job_ids[i], timeline[i], end_times[i]);
            current_time = end_times[i];
        }
    }
}

void grant_style_visualization(const JobShop *shop)
{
    const int MAX_TIME = 100; // Assume no schedule goes beyond 100 units
    char timeline[MAX_MACHINES][MAX_TIME];

    printf("\nGrant Style Visualization(by Machine):\n");

    for (int m = 0; m < shop->num_machines; ++m)
        for (int t = 0; t < MAX_TIME; ++t)
            timeline[m][t] = ' ';

    int latest_time = 0;

    // Fill the timeline with job IDs for each machine
    for (int j = 0; j < shop->num_jobs; ++j)
    {
        for (int o = 0; o < shop->jobs[j].num_operations; ++o)
        {
            Operation op = shop->jobs[j].operations[o];
            for (int t = op.start_time; t < op.end_time; ++t)
            {
                timeline[op.machine_id][t] = '0' + j; // '0' + job ID as char
            }
            if (op.end_time > latest_time)
                latest_time = op.end_time;
        }
    }

    // Print time header
    printf("\nGantt Chart (per machine):\n\n");
    printf("Time →  ");
    for (int t = 0; t < latest_time; ++t)
        printf("%2d ", t);
    printf("\n");

    // Print each machine's timeline
    for (int m = 0; m < shop->num_machines; ++m)
    {
        printf("M%d      ", m);
        for (int t = 0; t < latest_time; ++t)
        {
            printf(" %c ", timeline[m][t]);
        }
        printf("\n");
    }
    printf("\nLegend: '0' = Job 0, '1' = Job 1, etc.\n\n");
}

int main()
{
    JobShop shop;
    load_instance(&shop);
    print_instance(&shop);
    naive_schedule(&shop);
    analyze_machine_usage(&shop);
    grant_style_visualization(&shop);
    return 0;
}
