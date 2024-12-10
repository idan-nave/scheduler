/******************************************************************************/
/*
  Name: Idan Nave
  Project: Scheduler
  Reviewer: Ahmad
  Date: 10/09/23
  Version: 1.0

  Overview: Implementation of a Scheduler Manager.
*/

/******************************** Libraries  **********************************/
#include <assert.h>             /* assert() */
#include <stdio.h>              /* printf */
#include <stdlib.h>             /* malloc(), free() */
#include <stddef.h>             /* size_t, NULL */
#include <time.h>               /*time_t*/
#include <unistd.h>             /*sleep()*/

#include "/home/idan/Documents/Dev/Git_WD/ds/pq/include/pq.h"
/*dll_status_t, dll_boolean_t, pq_t*/
#include "/home/idan/Documents/Dev/Git_WD/ds/task/include/task.h" /*task_t*/
#include "/home/idan/Documents/Dev/Git_WD/ds/sched/include/sched.h"

/********************************* Type Definitions **************************/

struct scheduler
{
    pq_t *pq;
    int run_state;
    int is_self_terminate;
    task_t *curr_task;    
};


/************************** Helper Functions Declerations ********************/

/* Function to compare data & param */
static int CompareTasksExecTime(const void *task_in_q, const void *candidate);
/* Function to compare uid tasks */
static int MatchUID(const void *task, const void *task_uid);
/* Function to implement Wait */
static void Sleep(const task_t *task);

/******************************************************************************/
/*************** API Implementation - Functions Definitions *******************/
/******************************************************************************/
 
sched_t *SchedCreate(void)
{
  sched_t* sched = (sched_t*)malloc(sizeof(sched_t));
  if(NULL == sched)
  {
    return NULL;
  }
  
  sched->pq = PQCreate(CompareTasksExecTime);
  if(NULL == sched->pq)
  {
    free(sched);
    return NULL;
  }
  sched->run_state = NOT_RUNNING;
  sched->is_self_terminate = FALSE;
  sched->curr_task = NULL;
  return sched;
}

void SchedDestroy(sched_t *sched)
{
  assert(NULL != sched);

  SchedClear(sched);
  PQDestroy(sched->pq);

  free(sched);
  sched = NULL;
}

/*****************************************************************************/

uid_ty AddTask(sched_t *sched, time_t time_to_exec,
              sched_operation_t op_func, void *op_params,
              sched_cleanup_t cleanup_func, void *cleanup_params)
{
  task_t* new_task = NULL;
  assert(NULL != sched);
  assert( (time_t)(-1) != time_to_exec); 
  assert(NULL != op_func);

  new_task = TaskCreate(time_to_exec, (task_operation_t)op_func, op_params,
                             (task_cleanup_t)cleanup_func, cleanup_params);
  if (NULL == new_task)
  {
      return invalid_uid;
  }
  if (!PQEnqueue(sched->pq, new_task))
  {
      return invalid_uid;
  }
  return TaskGetUID(new_task);
}

void RemoveTask(sched_t *sched, uid_ty uid)
{
  task_t* to_rmv_task = NULL;
  assert(NULL != sched);
  assert(TRUE == UIDIsSame(invalid_uid, uid));

  if(sched->curr_task == NULL || !TaskIsMatch(sched->curr_task, uid))
  {
    to_rmv_task = PQErase(sched->pq, MatchUID ,&uid);
    /*printf("removed:"); PrintUID(TaskGetUID(to_rmv_task)); fflush(stdout);*/
    assert(NULL != to_rmv_task);
    TaskDestroy(to_rmv_task);
  }
  else
  {
    sched->is_self_terminate = TRUE;
  } 
}

/*****************************************************************************/

int SchedRun(sched_t *sched)
{
  int exec_status = SUCCESS;
  assert(NULL != sched);

  sched->run_state = RUNNING;

  while(FALSE == PQIsEmpty(sched->pq) )
  {
      if (NOT_RUNNING == sched->run_state)
      {
        sched->curr_task = NULL;
        return STOPPED;
      }

      sched->curr_task = (task_t*)PQDequeue(sched->pq);

      Sleep(sched->curr_task);

      exec_status = TaskExecute(sched->curr_task); /*changes sched->run_state*/

      if(RESCHED == exec_status && FALSE == sched->is_self_terminate)
      {
          TaskUpdateTimeToExecute(sched->curr_task);
          if (FAILURE == PQEnqueue(sched->pq, sched->curr_task))
          {
              sched->run_state = NOT_RUNNING;
              sched->curr_task = NULL;
              return FAILURE;
          }
      } 
      else
      {
        sched->is_self_terminate = FALSE;
        TaskDestroy(sched->curr_task);
      }     
  } 
  sched->run_state = NOT_RUNNING;
  sched->curr_task = NULL;
  return SUCCESS;
}

void SchedStop(sched_t *sched)
{
  assert(NULL != sched);

  sched->run_state = NOT_RUNNING;
}

/*****************************************************************************/

size_t SchedSize(const sched_t *sched)
{
  assert(NULL != sched);

  return PQSize(sched->pq) + sched->run_state;
}

int SchedIsEmpty(const sched_t *sched)
{
  assert(NULL != sched);

  return PQIsEmpty(sched->pq) && (NOT_RUNNING == sched->run_state);
}

void SchedClear(sched_t *sched)
{
  assert(NULL != sched);
  
  while(FALSE == PQIsEmpty(sched->pq))
  {
    TaskDestroy((task_t*)(PQDequeue(sched->pq)));
  }
  sched->run_state = NOT_RUNNING;
  /*PQClear(sched->pq); -- only clears, potential memory leaks*/
}

/*****************************************************************************/
/******************************* Tester Helper Functions  ********************/
/*****************************************************************************/

/* Function to compare data & param */
static int CompareTasksExecTime
      (const void *task_in_q, const void *candidate)
{
    return TaskCompare((task_t*)task_in_q, (task_t*)candidate);
}

/* Function to match data & param */
static int MatchUID(const void *task, const void *task_uid)
{
    return TaskIsMatch((task_t*)task, *(uid_ty*)task_uid);
}

/* Function toImplement Wait */
static void Sleep(const task_t *task)
{
    double remainig = difftime(TaskGetTimeToExecute(task),time(NULL));
    while(0 < remainig) 
    {
      remainig = sleep(remainig);
    }
}