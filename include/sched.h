/******************************************************************************/
/*
  Name: Idan Nave
  Project: Scheduler
  Reviewer: Ahmad
  Date: 10/09/23
  Version: 1.0

  Overview: Implementation of a Scheduler Manager.
*/

#ifndef __ILRD_SCHEDULER_H__
#define __ILRD_SCHEDULER_H__

/******************************** Libraries  **********************************/

#include <stddef.h>         /*size_t*/
#include <sys/types.h>      /*pid_t, time_t*/
#include <time.h>           /*time_t*/
#include "/home/idan/Documents/Dev/Git_WD/ds/uid/include/uid.h" /*uid_ty*/

/******************************** Definitions  ********************************/

typedef struct scheduler sched_t;

typedef int (*sched_operation_t)(void*); /*returns op_res_t*/
typedef void (*sched_cleanup_t)(void*);
typedef enum {UNSCHED = 0, RESCHED} op_res_t;

typedef enum {SUCCESS = 0, FAILURE, STOPPED} sched_status_t;
/*typedef enum {FALSE = 0, TRUE} sched_boolean_ty;*/
typedef enum {NOT_RUNNING = 0, RUNNING} sched_mode_t;

/********************* Scheduler API Declerations  ****************************/

/*Function that creates a scheduler.  
  Arguments: void.  
  Return value: pointer to a scheduler.
  Complexity: O(1). 
  Undefined B.:*/
sched_t *SchedCreate(void);

/*Function that destroys a scheduler.
  Arguments: 'sched' - pointer to scheduler.
  Return value: NA.
  Complexity: O(n).
  Undefined B.: NULL pointer to scheduler*/
void SchedDestroy(sched_t *sched);

/*Function that adds a task to the scheduler. The return value of 
  sched_func is responsible for rescheduling: if TRUE: task will be
   reschedule, except for when the task conducts clear operation.
  Arguments:  'sched' - pointer to scheduler.
              'time_to_exec' - Task's Time To Run. 
              'op_func' - task's operation function
              'op_params' - operation function-related parameters
              'cleanup_func' - task's optional clenaup operation for tasks
                               that allocate memory.
              'cleanup_params' - If Cleanup Function isn't available, 
                                 freeing of the params won't be attempted.
                                 Return value: pointer to newly generated Task. On Error, returns NULL.
  Complexity: O(n).
  Undefined B.: NULL ptr of scheduler, NULL time_to_exec
                NULL functions & params delivered by user 
                Memory leaks unhandled by user logics prior to calling Destroy\
                Addidn a task that does not returns op_res_t
                assigning functions that do not adhere to either sched_operation_t or sched_cleanup_t*/
uid_ty AddTask(sched_t *sched, time_t time_to_exec,
              sched_operation_t op_func, void *op_params,
              sched_cleanup_t cleanup_func, void *cleanup_params);

/*Function that removes a task from the scheduler by a uid identifier.
  Arguments: 'sched' - pointer to scheduler,
             'uid' - unique identifier for the task.
  Return value: N.A.
  Complexity: O(1).
  Undefined B.: NULL pointer to scheduler, invalid uid.*/
void RemoveTask(sched_t *sched, uid_ty uid);

/*Function that Sets the scheduler to 'Run',
            Queueud tasks are being executed by priority.
  Arguments: 'sched' - pointer to scheduler,
  Return value: sched_status_t: when queue is empty, returns SUCCESS\STOPPED,
                On Error, returns FAILURE.  
  Complexity: O(n*Operation Functions).
  Undefined B.: NULL pointer to scheduler,
                Calling to Run from within a running schedule*/
int SchedRun(sched_t *sched);

/*Function that Sets the scheduler to 'Stop',
            current task is first to be completed.
  Arguments: 'sched' - pointer to scheduler,
  Return value: N.A.
  Complexity: O(1).
  Undefined B.: NULL pointer to scheduler*/
void SchedStop(sched_t *sched);

/*Function that returns size of the scheduler.
  Arguments: 'sched' - pointer to scheduler,
  Return value: size of scheduler queue; . Running task is counted.
  Complexity: O(n).
  Undefined B.: NULL pointer to scheduler*/
size_t SchedSize(const sched_t *sched);

/*Function that returns if scheduler is empty.
  Arguments: 'sched' - pointer to scheduler.
  Return value: if empty = TRUE, othewise = FALSE. Running task is taken into consideration.
  Complexity: O(1).
  Undefined B.: NULL pointer to scheduler*/
int SchedIsEmpty(const sched_t *sched);

/*Function that clear all the elements in the scheduler.
  Arguments: 'sched' - pointer to scheduler.
  Return value: NA.
  Complexity: O(n).
  Undefined B.: NULL pointer to scheduler*/
void SchedClear(sched_t *sched);

#endif /*__ILRD_SCHEDULER_H__*/