/*
 * The code below has been developed by Johan Nordlander and Fredrik Bengtsson at LTU.
 * Part of the code has been also developed, modified and extended to ARMv8 by Wagner de Morais and Hazem Ali.
 * Modified for x86_64/Linux using ucontext for proper cooperative threading
 */

#define _XOPEN_SOURCE 600
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <signal.h>

#include "tinythreads.h"

/*----------------------------------------------------------------------------
  Constants
 *----------------------------------------------------------------------------*/
#define STACKSIZE	8192    // Increased stack size for x86_64
#define NTHREADS	5
#ifndef NULL
#define NULL 		0
#endif


/*----------------------------------------------------------------------------
  Interrupt Control - x86-64 Linux User Mode Emulation
  Uses signal blocking to simulate interrupt disable/enable
 *----------------------------------------------------------------------------*/

static sigset_t signal_mask;
static sigset_t old_mask;
static int signals_initialized = 0;

static void init_signals(void) {
	if (!signals_initialized) {
		sigfillset(&signal_mask);  // Block all signals
		signals_initialized = 1;
	}
}

__attribute__(( always_inline )) static inline void enable() {
	if (!signals_initialized) init_signals();
	sigprocmask(SIG_UNBLOCK, &signal_mask, NULL); // Enable "interrupts" (unblock signals)
}

__attribute__(( always_inline )) static inline void disable(){
	if (!signals_initialized) init_signals();
	sigprocmask(SIG_BLOCK, &signal_mask, &old_mask); // Disable "interrupts" (block signals)
}

#define DISABLE() disable()
#define ENABLE()  enable()


/*----------------------------------------------------------------------------
  Thread control structures
 *----------------------------------------------------------------------------*/
struct thread_block {
	short idx;								// Unique identifier
	void (*function)(int);					// Code to run, i.e. the routine to run
	int arg;								// Argument to the above
	thread next;							// For use in linked lists
	ucontext_t context;						// Machine state (using ucontext instead of jmp_buf)
	char stack[STACKSIZE];					// Execution stack space
	unsigned int Period_Deadline;			// Absolute Period and Deadline of the thread
	unsigned int Rel_Period_Deadline;		// Relative Period and Deadline of the thread
};

struct thread_block threads[NTHREADS];
struct thread_block initp;

// @brief Points to a queue of free thread_block instances/element in the threads array.
thread freeQ 	= threads;
// @brief Points to a queue of thread_block instances in the threads array that are ready to execute.
thread readyQ	= NULL;
// @brief Points to a queue of thread_block instances in the threads array that have finished execution
thread doneQ	= NULL;

thread current	= &initp;

int initialized = 0;


/** @brief Adds an element to the tail of the queue
 * @note In Assignment 4, parts 2 and 3, you might want to change this
 * implementation to enqueue with insertion sort.
 */
static void enqueue(thread p, thread *queue) {
	p->next = NULL;
	if (*queue == NULL) {
		*queue = p;
	} else {
		thread q = *queue;
		while (q->next) {
			q = q->next;
		}
		q->next = p;
	}
}

/** @brief Remove an element from the head of the queue
 */
static thread dequeue(thread *queue) {
	thread p = *queue;
	if (*queue) {
		*queue = (*queue)->next;
	} else {
		// Empty queue, handle this condition gracefully!
		return NULL;
	}
	return p;
}

/** @brief Initialize a single thread
 */
static void initializeThread(thread t, int idx) {
    t->idx = idx;
    t->function = NULL;
    t->arg = -1;
    t->next = &threads[idx + 1];
    t->Period_Deadline = INT_MAX;
    t->Rel_Period_Deadline = INT_MAX;
}


/** @brief Initializes each thread in the threads array.
 * For each thread in the threads array, a unique identifier is assigned
 * along with the task information.
 */
static void initializeThreads(void) {
	initp.idx = -1;
	initp.function = NULL;
	initp.arg = -1;
	initp.next = NULL;
	initp.Period_Deadline = INT_MAX;
	initp.Rel_Period_Deadline = INT_MAX;

	for (int i=0; i < NTHREADS; i++)
	{
		initializeThread(threads + i, i);
	}
	threads[NTHREADS - 1].next = NULL;
	initialized = 1;
}


/** @brief Wrapper function that runs the thread function and handles cleanup
 */
static void thread_wrapper(void) {
	ENABLE();
	current->function(current->arg);
	DISABLE();
	enqueue(current, &freeQ);
	current = NULL;
	if (readyQ != NULL) {
		thread next = dequeue(&readyQ);
		current = next;
		setcontext(&next->context);
	}
	// If no more threads, exit
	exit(0);
}

/** @brief Context switch to the next thread.
 * Starts or resumes the execution of the thread
 * select to execute.
 */
static void dispatch(thread next) {
	thread old = current;
	current = next;
	swapcontext(&old->context, &next->context);
}


/** @brief Creates a new thread and set its context,
 * e.g., the procedure that the thread will execute.
 * @param function is a pointer to the start routine
 * @param int arg is the parameter to the start routine
 */
void spawn(void (* function)(int), int arg) {
	thread newp;
	DISABLE();
	if (!initialized)
		initializeThreads();
	newp = dequeue(&freeQ);
	if (newp == NULL) {
		ENABLE();
		printf("ERROR: No free threads available!\n");
		return;
	}
	newp->function = function;
	newp->arg = arg;
	newp->next = NULL;

	// Set up the context for the new thread
	getcontext(&newp->context);
	newp->context.uc_stack.ss_sp = newp->stack;
	newp->context.uc_stack.ss_size = STACKSIZE;
	newp->context.uc_link = NULL;  // When thread finishes, it will handle cleanup itself
	makecontext(&newp->context, thread_wrapper, 0);

	enqueue(newp, &readyQ);
	ENABLE();
}

/** @brief Preempts the execution of the current thread and a new
 * thread gets to run.
 */
void yield(void) {
	DISABLE();
	if (readyQ != NULL){
		thread p = dequeue(&readyQ);
		enqueue(current, &readyQ);
		dispatch(p);
	}
	ENABLE();
}

/** @brief Sets the locked flag of the mutex if it was previously unlocked,
 * otherwise, the running thread shall be placed in the waiting queue of the
 * mutex and a new thread should be dispatched from the ready queue.
 */
void lock(mutex *m) {
	// To be implemented in Assignment 4!!!
	(void)m;  // Suppress unused parameter warning
}

/** @brief Activate a thread in the waiting queue of the mutex if it is
 * non-empty, otherwise, the locked flag shall be reset.
 */
void unlock(mutex *m) {
	// To be implemented in Assignment 4!!!
	(void)m;  // Suppress unused parameter warning
}

/** @brief Creates an thread block instance and assign to it an start routine,
 * i.e., the procedure that the thread will execute.
 * @param function is a pointer to the start routine
 * @param int arg is the parameter to the start routine
 */
void spawnWithDeadline(void (* function)(int), int arg, unsigned int deadline, unsigned int rel_deadline) {
	// To be implemented in Assignment 4!!!
	(void)function;
	(void)arg;
	(void)deadline;
	(void)rel_deadline;
}


/** @brief Sort the elements a given queue container by a given
 * field or attribute.
 * https://arxiv.org/abs/2110.01111
 */
static void sort(thread *queue) {
	// To be implemented in Assignment 4!!!
	(void)queue;
}

/** @brief Removes a specific element from the queue.
 */
static thread dequeueItem(thread *queue, int idx) {
	// You might need it in Assignment 4!!!
	(void)queue;
	(void)idx;
	return NULL;
}

/** @brief Periodic tasks have to be activated at a given frequency. Their activations are generated by timers .
 */
void respawn_periodic_tasks(void) {
	// To be implemented in Assignment 4!!!
}

/** @brief Schedules tasks using time slicing
 */
static void scheduler_RR(void){
	// To be implemented in Assignment 4!!!
}

/** @brief Schedules periodic tasks using Rate Monotonic (RM)
 */
static void scheduler_RM(void){
	// To be implemented in Assignment 4!!!
}

/** @brief Schedules periodic tasks using Earliest Deadline First  (EDF)
 */
static void scheduler_EDF(void){
	// To be implemented in Assignment 4!!!
}

/** @brief Calls the actual scheduling mechanisms, i.e., Round Robin,
 * Rate monotonic, or Earliest Deadline First.
 * When dealing with periodic tasks with fixed execution time,
 * it will first call the method that re-spawns period tasks.
 */
void scheduler(void){
	// To be implemented in Assignment 4!!!
}

/** @brief Prints via console the content of the main variables in TinyThreads
 */
void printTinyThreadsUART(void) {
	thread t;
	t = threads;
	printf("\nThreads\n");
	for (int i=0; i<NTHREADS; i++)
		printf("t[%i] @%p arg: %d idx: %d dl: %u\n", i, (void*)&t[i], t[i].arg, t[i].idx, t[i].Period_Deadline);

	printf("Current\n");
	printf("t[%i] @%p arg: %d dl: %u\n", current->idx, (void*)&current, current->arg, current->Period_Deadline);

	printf("freeQ\n");
	t=freeQ;
	while(t)
	{
		printf("t[%i] @%p arg: %d dl: %u\n", t->idx, (void*)t, t->arg, t->Period_Deadline);
		t = t->next;
	}

	printf("readyQ\n");
	t=readyQ;
	while(t)
	{
		printf("t[%i] @%p arg: %d dl: %u\n", t->idx, (void*)t, t->arg, t->Period_Deadline);
		t = t->next;
	}
	printf("doneQ\n");
	t=doneQ;
	while(t)
	{
		printf("t[%i] @%p arg: %d dl: %u\n", t->idx, (void*)t, t->arg, t->Period_Deadline);
		t = t->next;
	}
}

/** @brief Prints to console the content of the main variables in TinyThreads
 */
void printTinyThreadsPiface(void) {
	// For x64 version, we just use printf instead of piface
	printTinyThreadsUART();
}
