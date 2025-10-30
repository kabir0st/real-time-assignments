# PART 2 - Rate Monotonic Scheduling

The Rate Monotonic (RM) scheduling algorithm is used to schedule real-time periodic tasks in a real-time operating system (RTOS). Tasks are assigned priorities based on their periods, with shorter periods resulting in higher task priorities.

Modifications to thread_block in tinythreads.c:
- `unsigned int Period_Deadline`: Represents the absolute period and deadline of a task.
- `unsigned int Rel_Period_Deadline`: Represents the relative period and deadline of the task.

**Assignment Details**

**Source code**
1. Create a copy of the a4p1 directory and rename it to a4p2.
2. Download the attached a4p2.c and Makefile into the a4p2 directory.

**Exploring a4p2.c**
- The main() function now includes calls to spawnWithDeadline, with parameters for the start routine, its argument, period, and relative deadline for each task.
- Consider the computeSomething function, which has a fixed execution time of 1 tick. For example, spawnWithDeadline(computeSomething, 0, 3, 3) spawns a task that runs computeSomething, taking 1 tick to complete. The deadline and relative deadline parameters indicate a period of 3 ticks, so it activates every 3 ticks.
- Three tasks are spawned when the system starts in main(), and the timer interrupt is initialized.
   - On each timer interrupt, the scheduler() will pick the available task with the shortest period (highest priority).
   - Finished tasks should be respawned in multiples of Rel_Period_Deadline (i.e., the period).

**Implementing the Rate Monotonic Scheduling Algorithm**
3. In lib/tinythreads.c, implement void spawnWithDeadline(void (*function)(int), int arg, unsigned int deadline, unsigned int rel_deadline)
   - Sets the Period_Deadline and Rel_Period_Deadline attributes.

4. In lib/tinythreads.c, implement void respawn_periodic_tasks()
   - Spawns real-time periodic tasks that have completed according to their respective periods. Call from scheduler() at every timer interrupt before scheduling.

5. In lib/tinythreads.c, implement void scheduler_RM()
   - Implements RM scheduler, selects the task with the smallest period (highest priority). Keep readyQ sorted.

6. Compile the code and boot the RPi using the newly created kernel, a4p2.img. Adjust the Makefile as needed.

**Question (Part 2):**
7. Consider computeSomething with a fixed execution time equal to 1 tick. Remove the code while(t==ticks);, compile, and boot using the kernel.
- Create Answers.txt to explain the result and implication of removing while(t==ticks);.

**Deliverables (Part 2):**
- Answers.txt
- a4p2.c
- tinythreads.c

Note:
- All students in the group are equally responsible for the submitted source code.
- The group ensures that the submitted code does not include cheating and plagiarism issues.

***

# PART 3 - Earliest Deadline First Scheduling

The Earliest Deadline First (EDF) scheduling algorithm is a priority-based scheduler that calculates deadlines of real-time tasks dynamically (e.g., at each interrupt), and adjusts priorities accordingly. The closest deadline gives highest priority.

### Assignment Details

**Source code**
1. Create a copy of the a4p2 directory and rename it to a4p3.
2. Download the attached a4p3.c and Makefile into the a4p3 directory.

**Exploring a4p3.c**
- main() uses spawnWithDeadline for multiple tasks.
- Under EDF, priorities change dynamically based on deadlines. The scheduler will adjust the Period_Deadline attribute.

**Implementation of EDF Scheduling**
3. respawn_periodic_tasks() remains as implemented in Part 2 and is called by scheduler() at each timer interrupt.
4. Implement scheduler_EDF(), which schedules tasks based on closest deadline.
5. Adjust Period_Deadline of all ready tasks in the appropriate code segment.
6. Ensure readyQ remains sorted, highest priority (closest deadline) at the head.

**Kernel Generation**
7. Compile and boot using the kernel a4p3.img. Adjust the Makefile as needed.

**Schedulability Analysis**
8. In main(), change spawnWithDeadline parameters from (computeSomething, 2, 7, 7) to (computeSomething, 2, 4, 4).
9. Generate a new kernel and boot the RPi. Observe and determine if any task misses its deadline.

- Completes insights into EDF behavior and deadline misses.
10. **Question:** Does any task miss its deadline? If so, which one?

**Deliverables (Part 3):**
- a4p3.c
- tinythreads.c

