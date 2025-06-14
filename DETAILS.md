## Task operations[MAX_JOBS][MAX_MACHINES];

- Each row represents a job (up to MAX_JOBS jobs).
- Each column represents a machine (up to MAX_MACHINES machines).
- Each cell operations[j][m] holds the Task for job j on machine m.

Visual Representation:  
Suppose MAX_JOBS = 3 and MAX_MACHINES = 3 for simplicity:

|       | Machine 0 | Machine 1 | Machine 2 |
|-------|------------------|------------------|------------------|
| Job 0 | operations[0][0] | operations[0][1] | operations[0][2] |
| Job 1 | operations[1][0] | operations[1][1] | operations[1][2] | 
| Job 2 | operations[2][0] | operations[2][1] | operations[2][2] | 

##  OperationNode Indexing
- op_node_index(job, op, num_machines) gives a unique index for each operation:  
    - Job 0: [0][0], [0][1], [0][2] → indices 0, 1, 2
    - Job 1: [1][0], [1][1], [1][2] → indices 3, 4, 5

### Conjunctive Edges for Each Job
#### What is a conjunctive arc?
Definition:  
A conjunctive arc says:  
“This operation must come before another because they belong to the same job.”

#### What is a disjunctive arc?
Definition:  
A disjunctive arc says:  
“These two operations must not overlap because they use the same machine, but their order is not known yet.”

Job 0:  
[0][0] ---> [0][1] ---> [0][2]  
-- 0 --------- 1 --------- 2
Job 0:  
[1][0] ---> [1][1] ---> [1][2]  
-- 3 --------- 4 --------- 5

| Job   | Operation | Node Index | Edge To (next op) |
|-------|-----------|------------|-------------------|
| 0     |   0       |   0        |  1                |
| 0     |   1       |   1        |  2                |
| 1     |   0       |   3        |  4                |
| 1     |   1       |   4        |  5                |
....

- These edges guarantee the sequential execution of operations within each job.
- They form the backbone of the JSSP graph structure, to which disjunctive edges (machine constraints) are later added.

## Permutations

This function generates all possible permutations of the array arr (of length n), starting from index start.  
For each permutation, it evaluates the makespan using evaluate_permutation.  
If a permutation yields a better (smaller) makespan, it updates the global best.  


- Base case:  
    If start == n, a complete permutation has been formed in arr.
    - It evaluates the makespan for this permutation.
    - If this makespan is better than the current best, it updates best_makespan and copies the permutation to best_perm.
    - Then it returns.

- Recursive case:  
    For each possible element to place at position start:
    - Swap the current element at start with the element at i.
    - Recursively permute the rest of the array (start + 1 to n).
    - Swap back (backtrack) to restore the original order for the next iteration.

Suppose arr = [A, B, C], n = 3:

- At start = 0, it tries:
    - [A, B, C]
    - [B, A, C]
    - [C, B, A]
- For each, it recursively permutes the remaining elements.
