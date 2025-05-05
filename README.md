# jssp-approaches
A collection of implementations and approaches to solve the Job Shop Scheduling Problem (JSSP), including various algorithms and techniques for optimization and scheduling.


# High-Level Shifting Bottleneck Procedure

The goal is to schedule one machine at a time, always choosing the one that's currently the biggest bottleneck — i.e., the one causing the most delay in the overall schedule.
- The machine causing the most delay in the context of the Shifting Bottleneck Procedure is the one that, when optimally scheduled in isolation (given current constraints), still contributes the most to the overall makespan — not simply the one with the highest local makespan.

## Step-by-step Breakdown

### Initialize

- Start with all machines **unscheduled**.
- Maintain a **partial schedule** that grows with each scheduled machine.

### Iterative Scheduling

**While not all machines are scheduled:**

1. **For each unscheduled machine:**
   - Build a **disjunctive graph** representing dependencies between operations.
   - Solve a **single-machine scheduling subproblem** for that machine:
     - Determine the best order to schedule its operations to minimize the **maximum completion time (Cmax)**, while respecting job order constraints.
   - Compute the **makespan (Cmax)** or the delay this ordering causes.

2. **Select the machine with the maximum delay**  
   → This is the current **bottleneck**.

3. **Fix the sequence** of operations on that machine.

4. **Update the partial schedule** to include and respect this machine’s order.

5. **Repeat** until all machines are scheduled.

# TODO:
```
```
- ✅ **[Done]** Load instance from file and present matrix
```shell
Loaded JSSP matrix: 10 jobs, 10 machines
 0 29  1 78  2  9  3 36  4 49  5 11  6 62  7 56  8 44  9 21 
 0 43  2 90  4 75  9 11  3 69  1 28  6 46  5 46  7 72  8 30 
 1 91  0 85  3 39  2 74  8 90  5 10  7 12  6 89  9 45  4 33 
 1 81  2 95  0 71  4 99  6  9  8 52  7 85  3 98  9 22  5 43 
 2 14  0  6  1 22  5 61  3 26  4 69  8 21  7 49  9 72  6 53 
 2 84  1  2  5 52  3 95  8 48  9 72  0 47  6 65  4  6  7 25 
 1 46  0 37  3 61  2 13  6 32  5 21  9 32  8 89  7 30  4 55 
 2 31  0 86  1 46  5 74  4 32  6 88  8 19  9 48  7 36  3 79 
 0 76  1 69  3 76  5 51  2 85  9 11  6 40  7 89  4 26  8 74 
 1 85  0 13  2 61  6  7  8 64  9 76  5 47  3 52  4 90  7 45 
```
- ✅ **[Done]** Load schedule structure with current data matrix
```shell
Loaded schedule:
Job 0: Machine 0, Duration 29; Machine 1, Duration 78; Machine 2, Duration 9; Machine 3, Duration 36; Machine 4, Duration 49; Machine 5, Duration 11; Machine 6, Duration 62; Machine 7, Duration 56; Machine 8, Duration 44; Machine 9, Duration 21;
Job 1: Machine 0, Duration 43; Machine 2, Duration 90; Machine 4, Duration 75; Machine 9, Duration 11; Machine 3, Duration 69; Machine 1, Duration 28; Machine 6, Duration 46; Machine 5, Duration 46; Machine 7, Duration 72; Machine 8, Duration 30;
Job 2: Machine 1, Duration 91; Machine 0, Duration 85; Machine 3, Duration 39; Machine 2, Duration 74; Machine 8, Duration 90; Machine 5, Duration 10; Machine 7, Duration 12; Machine 6, Duration 89; Machine 9, Duration 45; Machine 4, Duration 33;
Job 3: Machine 1, Duration 81; Machine 2, Duration 95; Machine 0, Duration 71; Machine 4, Duration 99; Machine 6, Duration 9; Machine 8, Duration 52; Machine 7, Duration 85; Machine 3, Duration 98; Machine 9, Duration 22; Machine 5, Duration 43;
Job 4: Machine 2, Duration 14; Machine 0, Duration 6; Machine 1, Duration 22; Machine 5, Duration 61; Machine 3, Duration 26; Machine 4, Duration 69; Machine 8, Duration 21; Machine 7, Duration 49; Machine 9, Duration 72; Machine 6, Duration 53;
Job 5: Machine 2, Duration 84; Machine 1, Duration 2; Machine 5, Duration 52; Machine 3, Duration 95; Machine 8, Duration 48; Machine 9, Duration 72; Machine 0, Duration 47; Machine 6, Duration 65; Machine 4, Duration 6; Machine 7, Duration 25;
Job 6: Machine 1, Duration 46; Machine 0, Duration 37; Machine 3, Duration 61; Machine 2, Duration 13; Machine 6, Duration 32; Machine 5, Duration 21; Machine 9, Duration 32; Machine 8, Duration 89; Machine 7, Duration 30; Machine 4, Duration 55;
Job 7: Machine 2, Duration 31; Machine 0, Duration 86; Machine 1, Duration 46; Machine 5, Duration 74; Machine 4, Duration 32; Machine 6, Duration 88; Machine 8, Duration 19; Machine 9, Duration 48; Machine 7, Duration 36; Machine 3, Duration 79;
Job 8: Machine 0, Duration 76; Machine 1, Duration 69; Machine 3, Duration 76; Machine 5, Duration 51; Machine 2, Duration 85; Machine 9, Duration 11; Machine 6, Duration 40; Machine 7, Duration 89; Machine 4, Duration 26; Machine 8, Duration 74;
Job 9: Machine 1, Duration 85; Machine 0, Duration 13; Machine 2, Duration 61; Machine 6, Duration 7; Machine 8, Duration 64; Machine 9, Duration 76; Machine 5, Duration 47; Machine 3, Duration 52; Machine 4, Duration 90; Machine 7, Duration 45;
```
- [ ] Build a disjunctive graph (or equivalent structure) for operations on m considering:
  - Precedence constraints (job order)
  - Machine constraints (no overlapping tasks)

Conjunctive arcs:
Represent job precedence.

E.g., in Job 2: Op1 → Op2 → Op3

You can’t change their order.

Disjunctive arcs:
Represent machine exclusivity.

If machine M3 is used by Job 1 Op2 and Job 4 Op1, they both must be ordered, but either order is valid.

So: the goal is to find the best order for the disjunctive arcs (i.e., the ops on machine m) that results in the smallest makespan.
We eventually want to solve a single-machine scheduling problem with precedence constraints to compute the critical path (i.e., longest processing time path).

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

- [ ] Implement Earliest Start Time Scheduling
  - For a given machine m, implement a greedy scheduling routine that:
    - Schedules operations on m in some order (initially just order of appearance)
    - For each operation:
      - Determines its earliest possible start time considering:
        - Its predecessor in the same job
        - The previous operation on the machine
      - Stores the resulting ScheduledOp entries into MachineSchedule
  - This will enable us to : 
    - Build a Gantt-like schedule for a single machine
    - Compute makespan for that machine

- [ ] Identify the Bottleneck
  - For each machine:
    - Run the scheduling logic above
    - Compute makespan of that machine
    - Store the critical path
  - Then:
    - Pick the machine with maximum makespan → this is your bottleneck
    - Fix the schedule for that machine (lock its order)
    - Update global constraints (e.g., precedence edges for future operations)
- [ ] Loop:
    - Identify unscheduled machines
    - Schedule each temporarily
    - Find the bottleneck machine
    - Fix its schedule (permanent order)
    - Repeat until all machines are fixed
    
# Details1:
