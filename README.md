# jssp-approaches
A collection of implementations and approaches to solve the Job Shop Scheduling Problem (JSSP), including various algorithms and techniques for optimization and scheduling.


# High-Level Shifting Bottleneck Procedure

The goal is to schedule one machine at a time, always choosing the one that's currently the biggest bottleneck — i.e., the one causing the most delay in the overall schedule.
- The machine causing the most delay in the context of the Shifting Bottleneck Procedure is the one that, when optimally scheduled in isolation (given current constraints), still contributes the most to the overall makespan — not simply the one with the highest local makespan.

Each job's operations must be done in order (precedence).

Operations sharing the same machine cannot overlap.

E.G. 
##### Input:
|      |      |      |
| ---- | ---- | ---- |
| 0  3 | 1  2 | 2  2 |
| 0  2 | 2  1 | 1  3 |
| 1  4 | 2  3 | 0  1 |

##### Translation:

|       | Op0 | Op1 | Op2 |
|-------|-----|-----|-----|
| Job 0 | M0 T3 | M1 T2 | M2 T2 |
| Job 1 | M0 T2 | M2 T1 | M1 T3 |
| Job 2 | M1 T4 | M2 T3 | M0 T1 |

## Step-by-step Breakdown


    
## Disjunctive Graph Construction

Conjunctive arcs:
Represent job precedence.

E.g., in Job 2: Op1 → Op2 → Op3

You can’t change their order.

Disjunctive arcs:
Represent machine exclusivity.

If machine M3 is used by Job 1 Op2 and Job 4 Op1, they both must be ordered, but either order is valid.

So: the goal is to find the best order for the disjunctive arcs (i.e., the **ops** on machine **m**) that results in the smallest makespan. We eventually want to solve a single-machine scheduling problem with precedence constraints to compute the critical path (i.e., longest processing time path).

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

#### Worst-case scenario:
- If:
  - J = MAX_JOBS
  - M = MAX_MACHINES
  - Each job has one operation on each machine, so:

- There are J operations per machine (1 op per job per machine).
- That means k = J for every machine.
- So, worst-case disjunctive arcs per machine = J * (J - 1)
- Total across all machines = M * J * (J - 1)

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