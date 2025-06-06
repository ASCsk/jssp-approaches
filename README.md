# jssp-approaches
A collection of implementations and approaches to solve the Job Shop Scheduling Problem (JSSP), including various algorithms and techniques for optimization and scheduling.


# High-Level Shifting Bottleneck Procedure

The goal is to schedule one machine at a time, always choosing the one that's currently the biggest bottleneck — i.e., the one causing the most delay in the overall schedule.
- The machine causing the most delay in the context of the Shifting Bottleneck Procedure is the one that, when optimally scheduled in isolation (given current constraints), still contributes the most to the overall makespan — not simply the one with the highest local makespan.

## Step-by-step Breakdown

### Key Variables
- sched->job_ready[i]:
  - The earliest time when job i can start its next operation (i.e., the job is not blocked waiting on a previous task).

- sched->machine_ready[m]:
  - The earliest time when machine m is available to execute its next operation.

- data->operations[i][step]:
  - For job i, this is the operation at step step. Since each job has exactly one operation per machine, this array is traversed column-wise (fixed step, varying job).

### How the Loop Works
``` c
for (int step = 0; step < data->num_machines; step++) {
    for (int i = 0; i < data->num_jobs; i++) {
        ...
    }
}
```
This scheduling goes operation by operation for each step across all jobs.

- **step** is the current operation index for each job. All jobs have num_machines operations (one per machine).
- At each **step**, you look at the next operation of every job.


###  Scheduling Logic
``` c
Task task = data->operations[i][step];
int m = task.machine;

int start = (sched->job_ready[i] > sched->machine_ready[m]) ?
             sched->job_ready[i] : sched->machine_ready[m];
int end = start + task.duration;

sched->start_time[i][step] = start;
sched->end_time[i][step] = end;

sched->job_ready[i] = end;
sched->machine_ready[m] = end;
```

1. Choose the Operation to Schedule
- Pick job i’s operation at step step, which needs to run on machine m for task.duration time.

2. Compute Start Time
- You can’t start before:

  - The job is done with its previous operation (job_ready[i])

  - The machine is free (machine_ready[m])
``` c
start = max(job_ready[i], machine_ready[m]);
```
3. Update Schedule
- Set the start and end times of this operation.

- Mark job i as not ready until end.

- Mark machine m as busy until end.

### Need to: 
- Identify bottleneck machine per iteration.

- Reorder operations or solve subproblems for that machine.

- Possibly use disjunctive graph models or other optimization strategies.
  - [Disjunctive Graph Construction](#disjunctive-graph-construction)

## Step 1: Identify the Bottleneck Machine - [ ]
For each machine:

Extract the set of operations assigned to it.

Consider the precedence constraints (job order).

Solve the single-machine scheduling problem with precedence constraints.
- [Brute Force Permutation with EST Constraints](#Brute-Force-Permutation-with-EST-Constraints)

Compute the makespan for that machine schedule.

The bottleneck is the machine with the maximum makespan.

### A correct Shifting Bottleneck Procedure (SBP) step would:

1. Isolate the operations assigned to machine m.

2. Model their constraints:

3. Precedence: operations in a job must follow order (Op1 → Op2 → ...).

4. Resource: machine can handle only one task at a time.

5. Solve this machine's sequencing problem as a single-machine scheduling subproblem with precedence constraints.

6. Compute makespan for that subproblem.

## Step 2: Solve the Subproblem for the Bottleneck Machine - [ ]
Treat it as a 1-machine scheduling problem with precedence constraints.

Options:

Use branch & bound or list scheduling.

Build a disjunctive graph, solve the constrained path.

Lock this schedule into the global solution.

### Next steps:

1. Build the disjunctive graph: Nodes = operations; Arcs = job precedence (conjunctive) + machine constraints (disjunctive).

2. Topological sort / longest path to compute start times.

3. Detect and resolve conflicts: Add orientation to disjunctive arcs iteratively (like solving a single-machine problem for that machine).

4. Re-evaluate the critical path and bottlenecks after each fix.


## Step 3: Fix the Machine Order - [ ]
Update the global schedule:

All decisions made for the bottleneck machine become constraints.

Ensure future machines respect these decisions.

## Step 4: Iterate - [ ]
Remove the solved bottleneck machine from the list of unscheduled ones.

Repeat from Step 1 with the remaining machines.

## Optional Step: Optimization Strategy - [ ]
Model using Disjunctive Graphs (nodes: operations; arcs: precedences and disjunctions).

Use topological sorting, critical path, etc.

    
## Disjunctive Graph Construction

Conjunctive arcs:
Represent job precedence.

E.g., in Job 2: Op1 → Op2 → Op3

You can’t change their order.

Disjunctive arcs:
Represent machine exclusivity.

If machine M3 is used by Job 1 Op2 and Job 4 Op1, they both must be ordered, but either order is valid.

So: the goal is to find the best order for the disjunctive arcs (i.e., the ops on machine m) that results in the smallest makespan. We eventually want to solve a single-machine scheduling problem with precedence constraints to compute the critical path (i.e., longest processing time path).

```yaml
Job 0: [M2, M3, M1]
Job 1: [M1, M3, M2]

We are analyzing M3.

Ops on M3:
  - Job 0, Op 1 (comes after Job 0, Op 0)
  - Job 1, Op 1 (comes after Job 1, Op 0)

→ We must:
- Respect: Job 0, Op 0 → Job 0, Op 1
- Respect: Job 1, Op 0 → Job 1, Op 1
- Choose: Should Job 0’s M3 op happen before or after Job 1’s?
```

## Brute Force Permutation with EST Constraints
What it does:
- Enumerate all permutations of operations on the bottleneck machine.
- For each permutation, check if the permutation respects the precedence (EST) constraints:
- No operation should start before its EST.
- Operations must be scheduled sequentially on the machine (no overlap).
- Calculate the makespan or completion time for each valid permutation.
- Pick the permutation with the minimum makespan.

Why brute force?
- Simple and straightforward, especially useful for a small number of operations.
- Good for prototyping and baseline comparison.
- The EST constraints prune some invalid permutations, speeding up a bit.

Downsides:
- Scales factorially with the number of operations (huge for >10 ops).
- EST constraints only prune invalid sequences but don't avoid combinatorial explosion.

### Other approaches to solve single-machine subproblem (more scalable/efficient):
#### Branch & Bound
- Systematic search with pruning by bounds (e.g., lower bound on makespan).
- Use EST and Latest Finish Times (LFT) to prune branches early.
- Can incorporate heuristics and dominance rules.
#### Dynamic Programming
- For some special cases or small instances, DP can solve optimally.
#### Heuristics (for bigger instances)
- NEH heuristic (Nawaz-Enscore-Ham) adapted for precedence constraints.
- Greedy sequencing based on EST, processing times, slack, etc.
- Local search / metaheuristics like tabu search, simulated annealing, genetic algorithms.
#### Constraint Programming / ILP formulations
- Model single-machine scheduling with precedence and sequencing constraints.
- Use solver libraries (CPLEX, Gurobi, or CP solvers) for exact or heuristic solutions.