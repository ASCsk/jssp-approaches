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
   → This is your current **bottleneck**.

3. **Fix the sequence** of operations on that machine.

4. **Update the partial schedule** to include and respect this machine’s order.

5. **Repeat** until all machines are scheduled.

# TODO:

- ✅ **[Done]** Load instance and assign operations to machines
- ✅ Build the earliest start time calculator for each operation (respecting job order + machine order)
```
   Initialized machine schedules with operation counts:
   Machine 0: 3 operations
   Machine 1: 3 operations
   Machine 2: 3 operations

   Earliest start times:
   Job 0:
   Op on M0: start=0, end=3
   Op on M1: start=3, end=5
   Op on M2: start=5, end=7
   Job 1:
   Op on M1: start=0, end=2
   Op on M2: start=2, end=3
   Op on M0: start=3, end=7
   Job 2:
   Op on M2: start=0, end=4
   Op on M0: start=4, end=7
   Op on M1: start=7, end=9
```
- ✅ Build the earliest start time calculator for each operation (respecting job order + machine order)
```
Machine 0: 3 operations
Machine 1: 3 operations
Machine 2: 3 operations

Earliest start times:  
Job 0:
  Op on M0: start=0, end=3
  Op on M1: start=3, end=5
  Op on M2: start=5, end=7
Job 1:
  Op on M1: start=5, end=7
  Op on M2: start=7, end=8
  Op on M0: start=8, end=12
Job 2:
  Op on M2: start=8, end=12
  Op on M0: start=12, end=15
  Op on M1: start=15, end=17
```
- [ ] Implement a simple machine scheduler (e.g., sort ops by job order first, later improve with dispatching rules)
- [ ] Compute makespan for current partial schedule
- [ ] Find bottleneck machine (greedy: max delay/makespan)
- [ ] Fix that machine's operation order and repeat