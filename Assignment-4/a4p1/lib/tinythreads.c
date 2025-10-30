/*
 * The code below has been developed by Johan Nordlander and Fredrik Bengtsson at LTU.
 * Part of the code has been also developed, modified and extended to ARMv8 by Wagner de Morais and Hazem Ali.
*/
/*
 * Modified by Wagner Morais on Oct 2024.
 */

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>

#include "tinythreads.h"
#include "rpi-interrupts.h"
#include "uart.h"
#include "piface.h"
#include "rpi-systimer.h"

/*----------------------------------------------------------------------------
  Constants
 *----------------------------------------------------------------------------*/
#define STACKSIZE	1024
#define NTHREADS	5
// #define NULL 		0


/*----------------------------------------------------------------------------
  Internal References and Macros
 *----------------------------------------------------------------------------*/

__attribute__(( always_inline )) static inline void enable() {
	__asm volatile("CPSIE  i \n"); //Clear PRIMASK
}

__attribute__(( always_inline )) static inline void disable(){
	__asm volatile("cpsid i \n"); //set PRIMASK
}

#define DISABLE() disable()
#define ENABLE()  enable()
#define SETSTACK(buf,a) *((unsigned int *)(buf)+8) = (unsigned int)(a) + STACKSIZE - 4;	

/*----------------------------------------------------------------------------
  Thread control structures
 *----------------------------------------------------------------------------*/
struct thread_block {
	short idx;								// Unique identifier
	void (*function)(int);					// Code to run, i.e. the routine to run
	int arg;								// Argument to the above
	thread next;							// For use in linked lists
	jmp_buf context;						// Machine state
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

/** @brief Initializes each thread in the threads array.
 * For each thread in the threads array, a unique identifier is assigned
 * along with the task information. 
 */
void initialize(void) {
	initp.idx = -1;
	initp.function = NULL;
	initp.arg = -1;
	initp.next = NULL;
	initp.Period_Deadline = INT_MAX;
	initp.Rel_Period_Deadline = INT_MAX;	
	
	for (int i=0; i < NTHREADS; i++)
	{
		threads[i].idx = i;
		threads[i].function = NULL;
		threads[i].arg = -1;
		threads[i].next = &threads[i+1];
		threads[i].Period_Deadline = INT_MAX;
		threads[i].Rel_Period_Deadline = INT_MAX;
	}
	threads[NTHREADS-1].next = NULL;
	initialized = 1;
}

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
		while (q->next)
			q = q->next;
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
		// PUTTOLDC("%s", "Empty queue!!!");
		// Empty queue, kernel panic!!!
		// while (1) ;  // not much else to do...
		return NULL;
	}
	return p;
}

/** @brief Starts or resumes the execution of the thread
 * select to execute.
 */
static void dispatch(thread next) {
	if (next != NULL) {
		if (setjmp(current->context) == 0) {
			print2uart("Jumping from thread %d to thread %d\n", current->idx, next->idx);
			current = next;
			longjmp(next->context, 1);
		}
	}
	print2uart("------------------------ After Dispatch\n");
	printTinyThreadsUART();
}


/** @brief Creates an thread block instance and assign to it an start routine, 
 * i.e., the procedure that the thread will execute.
 * @param function is a pointer to the start routine
 * @param int arg is the parameter to the start routine
 */
void spawn(void (* function)(int), int arg) {
	thread newp;
	DISABLE();
	if (!initialized) 
		initialize();
	newp = dequeue(&freeQ);
	newp->function = function;
	newp->arg = arg;
	newp->next = NULL;
	if (setjmp(newp->context) == 1) {
		// enabling interrupts before calling the function
		// causes the function to be called in the context of the new thread
		// and it's getting called inside the ISR stack.
		ENABLE();
		print2uart("Function Called\n");
		current->function(current->arg);
		print2uart("function call returned\n");
		DISABLE();
		enqueue(current, &freeQ);
		current = NULL;
		dispatch(dequeue(&readyQ));	
	}
	print2uart("Thread Stack setup %d\n", &newp->idx);
	SETSTACK(&newp->context, &newp->stack);
	enqueue(newp, &readyQ);
	ENABLE();
}

/** @brief Preempts the execution of the current thread and a new 
 * thread gets to run.
 */
void yield(void) {
	// since yield is called inside the ISR stack,
	//  we don't need to disable interrupts, it's already disabled.
	// DISABLE();
	if (readyQ != NULL){
		thread p = dequeue(&readyQ);
		enqueue(current, &readyQ);
		dispatch(p);
	}
	print2uart("Yielded to thread %d\n", current->idx);
	// ENABLE();
}

void lock(mutex *m) {
  // To be implemented in Assignment 4!!!
  DISABLE();
  if(m->locked){
    print2uart("Mutex locked. Thread %d has to wait.\n", current->idx);
    if (&readyQ != NULL){
      thread p = dequeue(&readyQ);
      enqueue(current, &(m->waitQ));
      dispatch(p);
    }
  }
  else{
    print2uart("Thread %d locked Mutex\n", current->idx);
    m->locked = 1;
  }
  ENABLE();
}



/** @brief Activate a thread in the waiting queue of the mutex if it is
 * non-empty, otherwise, the locked flag shall be reset.
 */

void unlock(mutex *m) {
  // To be implemented in Assignment 4!!!
  DISABLE();
  if (m->waitQ != NULL) {
    thread p = dequeue(&(m->waitQ));
    enqueue(current, &readyQ);
    print2uart("Thread %d has done waiting\n", p->idx);
    dispatch(p);
  }
  else{
    print2uart("Thread %d unlocked Mutex\n", current->idx);
    m->locked = 0;
  }
  ENABLE();
}


/** @brief Creates an thread block instance and assign to it an start routine, 
 * i.e., the procedure that the thread will execute.
 * @param function is a pointer to the start routine
 * @param int arg is the parameter to the start routine
 */
void spawnWithDeadline(void (* function)(int), int arg, unsigned int deadline, unsigned int rel_deadline) {
	// To be implemented in Assignment 4!!!
}


/** @brief Sort the elements a given queue container by a given 
 * field or attribute.
 * https://arxiv.org/abs/2110.01111
 */
static void sortX(thread *queue) {
	// To be implemented in Assignment 4!!!
}

/** @brief Removes a specific element from the queue.
 */
static thread dequeueItem(thread *queue, int idx) {
	// You might need it in Assignment 4!!!
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
  print2uart("Current thread %d\n", current->idx);
  if (readyQ != NULL){
    thread p = dequeue(&readyQ);
    enqueue(current, &readyQ);
    dispatch(p);
  }
  print2uart("Yielded to thread %d\n", current->idx);
  print2uart("Scheduler RR After Yielding %d\n", current->idx);
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
	print2uart("Scheduler RR Called with thread %d\n", current->idx);
	scheduler_RR();
	// Note: if context switch happens, we may not return here
	print2uart("Scheduler RR Returned with thread %d\n", current->idx);
}

/** @brief Prints via UART the content of the main variables in TinyThreads
 */
void printTinyThreadsUART(void) {	
	thread t;
	t = threads;
	
	print2uart("\n\nCurrent\n");
	print2uart("t[%i]\n", current->idx);		

	print2uart("\n\nfreeQ\n");
	t=freeQ;
	while(t)
	{
		print2uart("t[%i]\n", t->idx);		
		t = t->next;
	}

	print2uart("\n\nreadyQ\n");
	t=readyQ;
	while(t)
	{
		print2uart("t[%i]\n", t->idx);		
		t = t->next;
	}
	print2uart("\n\ndoneQ\n");
	t=doneQ;
	while(t)
	{
		print2uart("t[%i]\n", t->idx);		
		t = t->next;
	}	
}

/** @brief Prints on the PiFace the content of the main variables in TinyThreads
 */
void printTinyThreadsPiface(void) {	
	thread t;

	piface_clear();
	piface_puts("t for thread");
	RPI_WaitMicroSeconds(2000000);

	t = threads;
	piface_clear();
	piface_puts("Threads");
	RPI_WaitMicroSeconds(2000000);
	for (int i=0; i<NTHREADS; i++)
	{
		piface_clear();
		PUTTOLDC("t[%i] @%#010x (%d)", i, &t[i], t[i].arg);		
		RPI_WaitMicroSeconds(2000000);
	}
	
	piface_clear();
	piface_puts("Current");
	RPI_WaitMicroSeconds(2000000);
	piface_clear();
	PUTTOLDC("t[%i] @%#010x (%d)", current->idx, &current, current->arg);		
	RPI_WaitMicroSeconds(2000000);

	piface_clear();
	t = freeQ;
	piface_puts("freeQ");
	RPI_WaitMicroSeconds(2000000);
	while(t)
	{
		piface_clear();
		PUTTOLDC("t[%i] @%#010x (%d)", t->idx, t, t->arg);		
		RPI_WaitMicroSeconds(2000000);
		t = t->next;
	}
	
	piface_clear();
	t = readyQ;
	piface_puts("readyQ");
	RPI_WaitMicroSeconds(2000000);
	while(t)
	{
		piface_clear();
		PUTTOLDC("t[%i] @%#010x (%d)", t->idx, t, t->arg);		
		RPI_WaitMicroSeconds(2000000);
		t = t->next;
	}
	
	piface_clear();
	t = doneQ;
	piface_puts("doneQ");
	RPI_WaitMicroSeconds(2000000);
	while(t)
	{
		piface_clear();
		PUTTOLDC("t[%i] @%#010x (%d)", t->idx, t, t->arg);		
		RPI_WaitMicroSeconds(2000000);
		t = t->next;
	}
	piface_clear();
}
