#include <stdio.h>
#include <stdlib.h>

#include "hardware.h"
#include "drivers.h"
#include "kernel.h"

int COUNTER = 0;

/* You may use the definitions below, if you are so inclined, to
   define the process table entries. Feel free to use your own
   definitions, though. */


typedef enum { RUNNING, READY, BLOCKED , UNINITIALIZED } PROCESS_STATE;

typedef struct process_table_entry {
  PROCESS_STATE state;
  int CPU_time_used;
  int quantum_start_time;
} PROCESS_TABLE_ENTRY;

/* Process table array that holds PIDS
 *
 *  process_table[i].quantum_start_time checks quantum start time for
 *  pid = i,
 *
 *  NOTE: MAXIMUM NUMBER OF PROCESSES
 *
 *
 *
 *
 */
extern PROCESS_TABLE_ENTRY process_table[];


/* A quantum is 40 ms */

#define QUANTUM 40

PROCESS_TABLE_ENTRY process_table[MAX_NUMBER_OF_PROCESSES];


/* You may use this code for maintaining a queue of
   ready processes. It's simply a linked list structure
   with each element containing the PID of a ready process */    

typedef struct ready_queue_elt {
  struct ready_queue_elt *next;
  PID_type pid;
} READY_QUEUE_ELT;


READY_QUEUE_ELT *ready_queue_head = NULL;  /* head of the event queue */
READY_QUEUE_ELT *ready_queue_tail = NULL;


//places process id at end of ready queue

void queue_ready_process(PID_type pid)
{
  //printf("QUEUEING ready PID %d\n", pid);
  READY_QUEUE_ELT *p = (READY_QUEUE_ELT *) malloc(sizeof(READY_QUEUE_ELT));
  p->pid = pid;
  p->next = NULL;
  if (ready_queue_tail == NULL) 
    if (ready_queue_head == NULL) {
      ready_queue_head = ready_queue_tail = p;
      p->next = NULL;
    }
    else {
      printf("Error: ready queue tail is NULL but ready_queue_head is not\n");
      exit(1);
    }
  else {
    ready_queue_tail->next = p;
    ready_queue_tail = p;
  }
}




//removes and returns PID from front of ready queue

PID_type dequeue_ready_process()
{
  if (ready_queue_head == NULL)
    if (ready_queue_tail == NULL)
      return IDLE_PROCESS;        // indicates no active process is ready
    else {
      printf("Error: ready_queue_head is NULL but ready_queue_tail is not\n");
      exit(1);
    }
  else {      
    READY_QUEUE_ELT *p = ready_queue_head;
    ready_queue_head = ready_queue_head->next;
    if (ready_queue_head == NULL)
      ready_queue_tail = NULL;
    return p->pid;
  }
}

/* Initialize INTERRUPT_TABLE Handlers
 *
 * trap_handler() - handles R1 calls and peforms exec
 *
 */



void next_process(){    
    
    PID_type newProcess;
    newProcess = dequeue_ready_process();
    
    
    if(newProcess == IDLE_PROCESS){
        printf("Time: %d: Processor is idle\n", clock);
        current_pid = IDLE_PROCESS;
    } else {
        current_pid = newProcess;
        process_table[current_pid].quantum_start_time = clock;
        
        process_table[current_pid].state = RUNNING;
        
        printf("Time: %d: Process %d runs\n", clock, current_pid);

    }
}

void trap_handler() {
    switch (R1) {
        case DISK_READ:
            
            process_table[current_pid].CPU_time_used += (clock + process_table[current_pid].quantum_start_time);
            
            
            disk_read_req(current_pid, R2);
            process_table[current_pid].state = BLOCKED;
            
            next_process();
            break;
        case DISK_WRITE:
            disk_write_req(current_pid);
            break;
        case KEYBOARD_READ:
            
            process_table[current_pid].CPU_time_used += (clock + process_table[current_pid].quantum_start_time);
            
            process_table[current_pid].state = BLOCKED;
            keyboard_read_req(current_pid);
            
            next_process();
            
            break;
        case FORK_PROGRAM:
            COUNTER++;
            process_table[R2].state = READY;
            queue_ready_process(R2);
            
            int a = clock;
            
            printf("Time: %d: CLOCK INTERRUPT for pid %d\n", clock, R2);

            break;
        case END_PROGRAM:
            //infinite loop starts here, program doesn't end
            //printf("ENDPROGRAM\n");
            
            printf("Time: %d: Process %d exits. Total CPU time = %d\n", clock, R2, process_table[R2].CPU_time_used);
            process_table[R2].state = UNINITIALIZED;
            
            COUNTER--;
            
            if(COUNTER == 0){
                printf("ENDPROG No more processes\n");
                exit(0);
            }
            
            next_process();
            
            break;
        default:
            printf("FAIL\n");
    }
}



void clock_handler() {
    
    if(current_pid != IDLE_PROCESS) {
        
        if( (clock - process_table[current_pid].quantum_start_time) >= QUANTUM){
            process_table[current_pid].state = READY;
            queue_ready_process(current_pid);
            next_process();
        }
    }
    
    
}


void disk_handler() {
   printf("Time: %d: handled DISK INTERRUPT for pid %d\n", clock, R1);
   process_table[R1].state = READY;
    queue_ready_process(R1);
    
    if(current_pid == IDLE_PROCESS) {
        next_process();
    }
}


void keyboard_handler() {
    printf("Time: %d: handled KEYBOARD INTERRUPT for pid %d\n", clock, R1);
    process_table[R1].state = READY;
    queue_ready_process(R1);
    
    if(current_pid == IDLE_PROCESS) {
        next_process();
    }
}
/* This procedure is automatically called when the 
   (simulated) machine boots up */

void initialize_kernel()
{
    PROCESS_TABLE_ENTRY process_table[MAX_NUMBER_OF_PROCESSES];
    /* Initialize process table entry array
     * 
     * int process_table[]
     *
     *
     *
     */
    process_table[0].state = RUNNING;
    process_table[0].quantum_start_time = clock;
    process_table[0].CPU_time_used = 0;
    
    COUNTER++;
    
    int i = 1;
    
    for (i ; i > MAX_NUMBER_OF_PROCESSES-1; i++) {
        process_table[i].state = UNINITIALIZED;
        process_table[i].CPU_time_used = 0;

    }
    
    INTERRUPT_TABLE[TRAP] = trap_handler;
    INTERRUPT_TABLE[CLOCK_INTERRUPT] = clock_handler;
    INTERRUPT_TABLE[DISK_INTERRUPT] = disk_handler;
    INTERRUPT_TABLE[KEYBOARD_INTERRUPT] = keyboard_handler;
    
    
    /* First code implementaitons and tests
     * main() will go here if works
     */

    
  // Put any initialization code you want here.
  // Remember, the process 0 will automatically be
  // executed after initialization (and current_pid
  // will automatically be set to 0), 
  // so the your process table should reflect that fact.
}
