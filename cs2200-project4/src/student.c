/*
 * student.c
 * Multithreaded OS Simulation for CS 2200
 *
 * This file contains the CPU scheduler for the simulation. ./os-sim 1 -rp 4
 */

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "student.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/** Function prototypes **/
extern void idle(unsigned int cpu_id);
extern void preempt(unsigned int cpu_id);
extern void yield(unsigned int cpu_id);
extern void terminate(unsigned int cpu_id);
extern void wake_up(pcb_t *process);
/** 
* global priority lable
* 0 for non priority, 1 for priority
**/
static int global_priority;
static int time_slice;
/*
 * current[] is an array of pointers to the currently running processes.
 * There is one array element corresponding to each CPU in the simulation.
 *
 * current[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the current[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  current_mutex has been provided
 * for your use.
 *
 * ready is a pointer to a struct you should use for your ready queue
 * implementation. The head of the queue corresponds to the process
 * that is about to be scheduled onto the CPU, and the tail is for
 * convenience in the enqueue function. See student.h for the 
 * relevant function and struct declarations.
 *
 * Similar to current[], ready is accessed by multiple threads,
 * so you will need to use a mutex to protect it. ready_mutex has been
 * provided for that purpose.
 *
 * The condition variable queue_not_empty has been provided for you
 * to use in conditional waits and signals.
 *
 * Please look up documentation on how to properly use pthread_mutex_t
 * and pthread_cond_t.
 */
static pcb_t **current;
static queue_t *rq;

static pthread_mutex_t current_mutex;
static pthread_mutex_t queue_mutex;
static pthread_cond_t queue_not_empty;

unsigned int cpu_count;

/*
 * enqueue() is a helper function to add a process to the ready queue.
 *
 * This function should add the process to the ready queue at the
 * appropriate location.
 *
 * NOTE: For priority scheduling, you will need to have additional logic
 * in either this function or the dequeue function to make the ready queue
 * a priority queue.
 */
void enqueue(queue_t *queue, pcb_t *process)
{
    process -> state = PROCESS_READY;
    if (global_priority == 1) {
        pthread_mutex_lock(&queue_mutex);
        if (is_empty(queue)) {
            queue -> head = process;
            queue -> tail = process;
        } else {
            if (queue -> head -> priority > process -> priority) {
                process -> next = queue -> head;
                queue -> head = process;
            } else {
                pcb_t* curr = queue -> head;
                while (curr -> next && curr -> next -> priority <= process -> priority) {
                    curr = curr -> next;
                }
                if (curr -> next) {
                    process -> next = curr -> next;
                    curr -> next = process;
                } else {
                    queue -> tail = process;
                    curr -> next = process;
                }
            }
        }
        pthread_cond_signal(&queue_not_empty);
        pthread_mutex_unlock(&queue_mutex);
    } else {
        pthread_mutex_lock(&queue_mutex);
        if (is_empty(queue)) {
            queue -> head = process;
            queue -> tail = process;
        } else {
            queue -> tail -> next = process;
            queue -> tail = process;
        }
        pthread_cond_signal(&queue_not_empty);
        pthread_mutex_unlock(&queue_mutex);
    }
}
/*
 * dequeue() is a helper function to remove a process to the ready queue.
 *
 * This function should remove the process at the head of the ready queue
 * and return a pointer to that process. If the queue is empty, simply return
 * NULL.
 *
 * NOTE: For priority scheduling, you will need to have additional logic
 * in either this function or the enqueue function to make the ready queue
 * a priority queue.
 */
pcb_t* dequeue(queue_t *queue)
{   
    pthread_mutex_lock(&queue_mutex);
    if (is_empty(queue)) {
        pthread_mutex_unlock(&queue_mutex);
        return NULL;
    } else {
        pcb_t *temp = queue -> head;
        queue -> head = temp -> next;
        temp -> next = NULL;
        temp -> state = PROCESS_RUNNING;
        pthread_mutex_unlock(&queue_mutex);
        return temp;
    }
}

/*
 * is_empty() is a helper function that returns whether the ready queue
 * has any processes in it.
 *
 * If the ready queue has no processes in it, is_empty() should return true.
 * Otherwise, return false.
 */
bool is_empty(queue_t *queue)
{
    return queue -> head == NULL;
}

/*
 * schedule() is your CPU scheduler.  It should perform the following tasks:
 *
 *   1. Select and remove a runnable process from your ready queue which
 *  you will have to implement with a linked list or something of the sort.
 *
 *   2. Set the process state to RUNNING and change the priority value for the process for Round Robin with Priority
 *
 *   3. Set the currently running process using the current array
 *
 *   4. Call context_switch(), to tell the simulator which process to execute
 *      next on the CPU.  If no process is runnable, call context_switch()
 *      with a pointer to NULL to select the idle process.
 *
 *  The current array (see above) is how you access the currently running process indexed by the cpu id.
 *  See above for full description.
 *  context_switch() is prototyped in os-sim.h. Look there for more information
 *  about it and its parameters.
 */
static void schedule(unsigned int cpu_id)
{
    /*
    pthread_mutex_lock(&current_mutex);
    pcb_t *process_run = dequeue(rq);
    current[cpu_id] = process_run;
    context_switch(cpu_id, process_run, time_slice);
    pthread_mutex_unlock(&current_mutex);
    */
    /*
    pthread_mutex_lock(&current_mutex);
    pcb_t *process_run = dequeue(rq);
    current[cpu_id] = process_run;
    
    if (process_run) { 
        if (global_priority == 1) {
            unsigned int pr = process_run -> priority;
            if (pr < 20) {
                pr = pr + 1;
                process_run -> priority = pr;
            }
        }
    }
    
    pthread_mutex_unlock(&current_mutex);
    context_switch(cpu_id, process_run, time_slice);
    */
    pthread_mutex_lock(&current_mutex);
    pcb_t *process_run = dequeue(rq);
    current[cpu_id] = process_run;
    
    if (process_run != NULL) {
        if (global_priority == 1) {
            unsigned int pr = process_run -> priority;
            if (pr < 20) {
                pr = pr + 1;
                process_run -> priority = pr;
            }
        }
    }

    context_switch(cpu_id, process_run, time_slice);
    pthread_mutex_unlock(&current_mutex);
}


/*
 * idle() is your idle process.  It is called by the simulator when the idle
 * process is scheduled.
 *
 * This function should block until a process is added to your ready queue.
 * It should then call schedule() to select the process to run on the CPU.
 */
extern void idle(unsigned int cpu_id)
{
    pthread_mutex_lock(&queue_mutex);
    while (is_empty(rq)) {
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }
    pthread_mutex_unlock(&queue_mutex);
    schedule(cpu_id);
}


/*
 * preempt() is the handler called by the simulator when a process is
 * preempted due to its timeslice expiring in the case of Round Robin
 * scheduling or if a process with a higher priority is ready in the case of
 * Round Robin with Priority scheduling.
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 *
 * Remember to set the status of the process to the proper value.
 */
extern void preempt(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    pcb_t *process_curr = current[cpu_id];
    enqueue(rq, process_curr);
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}


/*
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * It should mark the process as WAITING, then call schedule() to select
 * a new process for the CPU.
 */
extern void yield(unsigned int cpu_id)
{
    /*
    pthread_mutex_lock(&current_mutex);
    pcb_t *process_curr = current[cpu_id];
    process_curr -> state = PROCESS_WAITING;
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
    */
    pthread_mutex_lock(&current_mutex);
    pcb_t *process_curr = current[cpu_id];
    process_curr -> state = PROCESS_WAITING;

    if (global_priority == 1) {
        unsigned int pr = process_curr -> priority;
        if (pr > 0) {
            pr = pr - 1;
            process_curr -> priority = pr;
        }
    }

    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}


/*
 * terminate() is the handler called by the simulator when a process completes.
 * It should mark the process as terminated, then call schedule() to select
 * a new process for the CPU.
 */
extern void terminate(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    current[cpu_id] -> state = PROCESS_TERMINATED;
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}

/*
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes.  It should perform the following tasks:
 *
 *   1. Mark the process as READY, and insert it into the ready queue.
 *
 *   2. If the scheduling algorithm is Round Robin with Priority, wake_up() may need
 *      to preempt the process that is currently executing on the CPU to allow it to
 *      execute the process which just woke up.  However, if any CPU is
 *      currently running idle, or all of the CPUs are running processes
 *      with an equal or higher priority than the one which just woke up,
 *      wake_up() should not preempt any CPUs.
 *  To preempt a process, use force_preempt(). Look in os-sim.h for
 *  its prototype and the parameters it takes in.
 */
extern void wake_up(pcb_t *process)
{
    enqueue(rq, process);

    if (global_priority == 1) {
        pthread_mutex_lock(&current_mutex);
        int lowest_priority_cpu = -1;
        unsigned int lowest_priority = process -> priority;

        for (unsigned int i = 0; i < cpu_count; i++) {
            if (current[i] == NULL) {
                lowest_priority_cpu = -1;
                break;
            }
            if (current[i]->priority > lowest_priority) {
                lowest_priority = current[i]->priority;
                lowest_priority_cpu = (int) i;
            }
        }

        pthread_mutex_unlock(&current_mutex);
        if (lowest_priority_cpu != -1) {
            force_preempt((unsigned int)lowest_priority_cpu);
        }
    }
}


/*
 * main() simply parses command line arguments, then calls start_simulator().
 * You will need to modify it to support the -r and -rp command-line parameters.
 */
int main(int argc, char *argv[])
{
    /*
     * Check here if the number of arguments provided is valid.
     * You will need to modify this when you add more arguments.
     */

    /* FIX ME - Add support for -r and -rp parameters*/

    if (argc < 2) {
        fprintf(stderr, "CS 2200 Project 4 -- Multithreaded OS Simulator\n"
            "Usage: ./os-sim <# CPUs> [ -r <time slice> | -s ]\n"
            "    Default : FCFS Scheduler\n"
            "         -r : Round-Robin Scheduler\n"
            "         -rp : Round-Robin Scheduler with Priority\n\n");
        return -1;
    }

    if (argc == 2) {
        global_priority = 0;
        time_slice = -1;
    }
    else if (strcmp(argv[2], "-r") == 0) {
        global_priority = 0;
        time_slice = atoi(argv[3]);
    }
    else if (strcmp(argv[2], "-rp") == 0) {
        global_priority = 1;
        time_slice = atoi(argv[3]);
    }

    /* Parse the command line arguments */
    cpu_count = strtoul(argv[1], NULL, 0);

    /* Allocate the current[] array and its mutex */
    current = malloc(sizeof(pcb_t*) * cpu_count);
    assert(current != NULL);
    pthread_mutex_init(&current_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_not_empty, NULL);
    rq = (queue_t*) malloc(sizeof(queue_t));
    assert(rq != NULL);

    /* Start the simulator in the library */
    start_simulator(cpu_count);

    return 0;
}

#pragma GCC diagnostic pop
