/*
 * student.h
 * Multithreaded OS Simulation for CS 2200
 *
 * YOU WILL NOT NEED TO MODIFY THIS FILE
 *
 */

#pragma once

#include "os-sim.h"
#include "stdbool.h"

/* Ready queue struct definition */
typedef struct {
    pcb_t *head;
    pcb_t *tail;
} queue_t;

/* Scheduling function declarations */
extern void idle(unsigned int cpu_id);
extern void preempt(unsigned int cpu_id);
extern void yield(unsigned int cpu_id);
extern void terminate(unsigned int cpu_id);
extern void wake_up(pcb_t *process);

/* Ready queue function declarations */
void enqueue(queue_t *queue, pcb_t *process);
pcb_t* dequeue(queue_t *queue);
bool is_empty(queue_t *queue);
