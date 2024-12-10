/******************************************************************************/
/*
  Name: Idan Nave
  Project: Scheduler
  Reviewer: Ahmad
  Date: 10/09/23
  Version: 1.0

  Overview: Implementation of a Scheduler Manager.
*/

/******************************** Libraries  *********************************/

#include <assert.h>             /* assert() */
#include <stdio.h>              /* perror */
#include <stdlib.h>             /* malloc(), free() */
#include <errno.h>              /* errno global */
#include <stddef.h>             /* size_t, NULL */

#include "/home/idan/Documents/Dev/Git_WD/ds/sched/include/sched.h"
#include "/home/idan/Documents/Dev/Git_WD/ds/uid/include/uid.h" /*uid_ty*/

/*typedef enum {FALSE = 0, TRUE} sched_boolean_t;
typedef enum  {SUCCESS = 0, FAILURE} sched_status_t;*/

/*****************************************************************************/
/********************************* Type Definitions **************************/
/*****************************************************************************/
    
#define REPORT(expected	, result, str) \
	{\
	    if (expected != result)\
	    {\
	    	printf("%s has failed\n", str);\
	    }\
	}

typedef struct AddParams
{
	sched_t *sched;
	time_t time_to_exec;
	sched_operation_t op_func;
	void *op_params;
	sched_cleanup_t cleanup_func;
	void *cleanup_params;
} add_params_t;

typedef struct RmvParams
{
	sched_t *sched;
	uid_ty uid;
} rmv_params_t;

/*****************************************************************************/
/************************** Helper Functions Declerations ********************/
/*****************************************************************************/

static int Compare(const void *data1, const void *data2);
static int Match(const void* data, void* param);
static int UnschedPrint(void* param);
static int ReschedPrint(void* param);
static time_t GiveAbsFutureTime(double sec_from_now);
static void Sleep(double remainig);

/*****************************************************************************/
/************************ Sched Wrapper Declerations *************************/
/*****************************************************************************/

static int UnschedPrint(void* param);
static int ReschedPrint(void* param);
static int ClearSchedOp(void *param);
static int RunSchedOp(void *sched);
static int StopSchedOp(void *sched);
static void CleanUp(void* param);
static int AddTaskOp(void *param);
static int RemoveTaskOp(void *param);

/*****************************************************************************/
/**************************** Test Functions Declerations ********************/
/*****************************************************************************/

static void MainTestFunc(void);

static void TestSchedCreate(void);
static void TestSchedDestroy(void);

static void TestAddTask(void);
static void TestRemoveTask(void);
static void TestSchedSize(void);
static void TestSchedIsEmpty(void);
static void TestSchedClear(void);

static void TestSchedRun(void);
static void TestSchedStop(void);

/*****************************************************************************/
/*************************************** Main ********************************/
/*****************************************************************************/

int main()
{
	MainTestFunc();
	return 0;
}

/******************************************************************************/

static void MainTestFunc(void)
{
	TestSchedCreate();
	TestSchedDestroy();

	TestAddTask();
	/*TestRemoveTask();*/
	TestSchedSize();
	TestSchedIsEmpty();
	TestSchedClear();

	/*TestSchedStop();*/
	TestSchedRun();
}

/*****************************************************************************/
/**************************** Test Functions Definitions *********************/
/*****************************************************************************/

static void TestSchedCreate(void)
{
	sched_t *sched = SchedCreate();
	REPORT(TRUE, SchedIsEmpty(sched), "TestSchedCreate()");
	REPORT(FALSE, SchedSize(sched), "TestSchedCreate()");

	printf("TestSchedCreate() tested\n");
	
	SchedDestroy(sched);
}

static void TestSchedDestroy(void)
{
	int data = 5;
	sched_t *sched = SchedCreate();
	time_t t1 = GiveAbsFutureTime(5);
	int* alloc_var = (int*)malloc(sizeof(int));
/*	int* alloc_var2 = (int*)malloc(sizeof(int));
*/
	AddTask(sched, t1, UnschedPrint, &data , CleanUp, (void*)alloc_var);
	REPORT(1, SchedSize(sched), "TestSchedDestroy()");
	SchedClear(sched);
	REPORT(0, SchedSize(sched), "TestSchedDestroy()");

	printf("TestSchedDestroy() tested\n");
	SchedDestroy(sched);
}

/*****************************************************************************/

static void TestAddTask(void)
{
	int data = 5;
	sched_t *sched = SchedCreate();
	time_t t1 = GiveAbsFutureTime(5);
	time_t t2 = GiveAbsFutureTime(10);
	int* alloc_var = (int*)malloc(sizeof(int));
	int* alloc_var2 = (int*)malloc(sizeof(int));

	/*Destroy of unsheduled task*/
	AddTask(sched, t1, UnschedPrint, &data , CleanUp, (void*)alloc_var);
	REPORT(1, SchedSize(sched), "TestAddTask()");
	SchedClear(sched);
	REPORT(0, SchedSize(sched), "TestAddTask()");
	
	/*Destroy of resheduled task*/
	AddTask(sched, t2, ReschedPrint, &data , CleanUp, (void*)alloc_var2);
	REPORT(1, SchedSize(sched), "TestAddTask()");
	SchedClear(sched);

	printf("TestAddTask() tested\n");
	SchedDestroy(sched);
}

static void TestRemoveTask(void)
{
	int data = 5;
	sched_t *sched = SchedCreate();
	time_t t1 = GiveAbsFutureTime(5);
	time_t t2 = GiveAbsFutureTime(10);
	int* alloc_var = (int*)malloc(sizeof(int));
	int* alloc_var2 = (int*)malloc(sizeof(int));
	uid_ty uid1 = {0};
	uid_ty uid2 = {0};

	/*save valid uids*/
	uid1 = AddTask(sched, t1, UnschedPrint, &data , CleanUp, alloc_var);
	/*if (TRUE == UIDIsSame(invalid_uid, uid1))
	{
		printf("\tInvalid UID was detected:");
		return;
	}*/
	REPORT(1, SchedSize(sched), "TestRemoveTask()");
	uid2 = AddTask(sched, t2, ReschedPrint, &data , CleanUp, alloc_var2);
	/*if (TRUE == UIDIsSame(invalid_uid, uid1))
	{
		printf("\tInvalid UID was detected:");
		return;
	}*/
	REPORT(2, SchedSize(sched), "TestRemoveTask()");
	
	/*Destroy of resheduled task*/
	RemoveTask(sched, uid2);
	REPORT(1, SchedSize(sched), "TestRemoveTask()");
	RemoveTask(sched, uid1);
	REPORT(0, SchedSize(sched), "TestRemoveTask()");

	printf("TestRemoveTask() tested\n");
	SchedDestroy(sched);
}

/*****************************************************************************/

static void TestSchedSize(void)
{
	int data = 5;
	sched_t *sched = SchedCreate();
	time_t t1 = GiveAbsFutureTime(5);
	int* alloc_var = (int*)malloc(sizeof(int));
/*	int* alloc_var2 = (int*)malloc(sizeof(int));
*/
	AddTask(sched, t1, UnschedPrint, &data , CleanUp, (void*)alloc_var);
	REPORT(1, SchedSize(sched), "TestSchedSize()");
	SchedClear(sched);
	REPORT(0, SchedSize(sched), "TestSchedSize()");

	printf("TestSchedSize() tested\n");
	SchedDestroy(sched);
}

static void TestSchedIsEmpty(void)
{
	int data = 5;
	sched_t *sched = SchedCreate();
	time_t t1 = GiveAbsFutureTime(5);
	int* alloc_var = (int*)malloc(sizeof(int));
	int* alloc_var2 = (int*)malloc(sizeof(int));

	REPORT(TRUE, SchedIsEmpty(sched), "TestSchedIsEmpty()");
	AddTask(sched, t1, UnschedPrint, &data , CleanUp, (void*)alloc_var);
	REPORT(FALSE, SchedIsEmpty(sched), "TestSchedIsEmpty()");

	AddTask(sched, t1, UnschedPrint, &data , CleanUp, (void*)alloc_var2);
	REPORT(FALSE, SchedIsEmpty(sched), "TestSchedIsEmpty()");

	SchedClear(sched);
	REPORT(TRUE, SchedIsEmpty(sched), "TestSchedIsEmpty()");

	AddTask(sched, t1, UnschedPrint, &data , NULL, NULL);
	REPORT(FALSE, SchedIsEmpty(sched), "TestSchedIsEmpty()");
	
	SchedClear(sched);
	REPORT(TRUE, SchedIsEmpty(sched), "TestSchedIsEmpty()");

	printf("TestSchedIsEmpty() tested\n");
	SchedDestroy(sched);
}

static void TestSchedClear(void)
{
	int data = 5;
	sched_t *sched = SchedCreate();
	time_t t1 = GiveAbsFutureTime(5);
	int* alloc_var = (int*)malloc(sizeof(int));
	int* alloc_var2 = (int*)malloc(sizeof(int));
/*	int* alloc_var3 = (int*)malloc(sizeof(int));
*/
	AddTask(sched, t1, UnschedPrint, &data , CleanUp, (void*)alloc_var);
	REPORT(1, SchedSize(sched), "TestSchedClear()");

	AddTask(sched, t1, UnschedPrint, &data , CleanUp, (void*)alloc_var2);
	REPORT(2, SchedSize(sched), "TestSchedClear()");

	SchedClear(sched);
	REPORT(0, SchedSize(sched), "TestSchedClear()");

	AddTask(sched, t1, UnschedPrint, &data , CleanUp, NULL);
	REPORT(1, SchedSize(sched), "TestSchedDestroy()");
	
	SchedClear(sched);
	REPORT(0, SchedSize(sched), "TestSchedDestroy()");
	
	printf("TestSchedClear() tested\n");
	SchedDestroy(sched);
}

/*****************************************************************************/

static void TestSchedStop(void)
{
	sched_t *sched = SchedCreate();
	time_t t1 = GiveAbsFutureTime(20);
	time_t t2 = GiveAbsFutureTime(7);
	time_t t3 = GiveAbsFutureTime(17);
	time_t t4 = GiveAbsFutureTime(13);
	time_t t5 = GiveAbsFutureTime(5);
	time_t t6 = GiveAbsFutureTime(15);
	time_t t7 = GiveAbsFutureTime(10);

	int to_print = 1; 	int* alloc_var = (int*)malloc(sizeof(int));
	int to_print2 = 2;	int* alloc_var2 = (int*)malloc(sizeof(int));
	/*int to_print3 = 3;*/
	int to_print4 = 4;	int* alloc_var4 = (int*)malloc(sizeof(int));
	int to_print5 = 5;	int* alloc_var5= (int*)malloc(sizeof(int));
	int to_print6 = 6;	int* alloc_var6 = (int*)malloc(sizeof(int));
	int to_print7 = 7;	int* alloc_var7 = (int*)malloc(sizeof(int));

	/*SPECIAL CASE: 	---R--------Re-Re-St-------*/
	/*EXPECTED TASK: 	t5-t2-t7-t4-t2-t6-t3-t1-t6-*/
	/*EXPECTED PRINT:   -5--2--7--4--2--6-NA-NA-NA-*/
	/*Seconds:			-5--7-10-13-14-15-17-20-30-*/

	AddTask(sched, t1, UnschedPrint, &to_print , CleanUp, alloc_var);
	AddTask(sched, t2, ReschedPrint, &to_print2 , CleanUp, alloc_var2);
	AddTask(sched, t3, StopSchedOp, sched , CleanUp, NULL);
	AddTask(sched, t4, UnschedPrint, &to_print4 , CleanUp, alloc_var4);
	AddTask(sched, t5, UnschedPrint, &to_print5 , CleanUp, alloc_var5);
	AddTask(sched, t6, ReschedPrint, &to_print6 , CleanUp, alloc_var6);
	AddTask(sched, t7, UnschedPrint, &to_print7 , CleanUp, alloc_var7);

	SchedRun(sched);
	printf("TestSchedStop() tested\n");
	SchedDestroy(sched);
}

static void TestSchedRun(void)
{
	sched_t *sched = SchedCreate();
	time_t t1 = GiveAbsFutureTime(29);
	time_t t2 = GiveAbsFutureTime(7);
	time_t t3 = GiveAbsFutureTime(17);
	time_t t4 = GiveAbsFutureTime(13);
	time_t t5 = GiveAbsFutureTime(5);
	time_t t6 = GiveAbsFutureTime(15);
	time_t t7 = GiveAbsFutureTime(10);
	time_t t_added = GiveAbsFutureTime(27);
	time_t t8 = GiveAbsFutureTime(5); /*new wave*/
	time_t t10 = GiveAbsFutureTime(7);/*new wave*/
	time_t t11 = GiveAbsFutureTime(15); /*new wave*/

	add_params_t add_op = {0};
	rmv_params_t rmv_op = {0};
	int to_print_by_added = 0;
	int* to_free_by_added = (int*)malloc(sizeof(int));

	int to_print = 1; 	int* alloc_var = (int*)malloc(sizeof(int));
	int to_print2 = 2;	int* alloc_var2 = (int*)malloc(sizeof(int));
	/*int to_print3 = 3;*/
	int to_print4 = 4;	int* alloc_var4 = (int*)malloc(sizeof(int));
	int to_print5 = 5;	int* alloc_var5= (int*)malloc(sizeof(int));
	int to_print6 = 6;	int* alloc_var6 = (int*)malloc(sizeof(int));
	int to_print7 = 7;	int* alloc_var7 = (int*)malloc(sizeof(int));
	int to_print8 = 8;	int* alloc_var8 = (int*)malloc(sizeof(int));
	/*int to_print9 = 0;*/
	int to_print10 = 10;int* alloc_var10 = (int*)malloc(sizeof(int));
	int to_print11 = 11;int* alloc_var11 = (int*)malloc(sizeof(int));

	/*SPECIAL CASE: 	Ad-Re-Rm-------Re-Re-Stp-Run-Re-------Ru-*/
	/*EXPECTED TASK: 	t5-t2-t8-t7-t4-t2-t6-t3--EX--t2-ta-t1-t6-*/
	/*EXPECTED PRINT:   AD--2-NA-7--4--2--6--NA--NA--2--0--1--6-*/
	/*Seconds:			NA--7-9--10-13-14-15-17--21--21-27-29-30-*/

	add_op.sched = sched;
	add_op.time_to_exec = t_added;
	add_op.op_func = UnschedPrint;
	add_op.op_params = &to_print_by_added;
	add_op.cleanup_func = CleanUp;
	add_op.cleanup_params = to_free_by_added;

	

/*	AddTask(sched, t1, UnschedPrint, &to_print , CleanUp, alloc_var);
	AddTask(sched, t2, ReschedPrint, &to_print2 , CleanUp, alloc_var2);
	AddTask(sched, t3, StopSchedOp, sched , CleanUp, NULL);
	AddTask(sched, t4, UnschedPrint, &to_print4 , CleanUp, alloc_var4);
	AddTask(sched, t5, AddTaskOp, &add_op , CleanUp, alloc_var5);
	AddTask(sched, t6, ReschedPrint, &to_print6 , CleanUp, alloc_var6);
	AddTask(sched, t7, UnschedPrint, &to_print7 , CleanUp, alloc_var7);
	AddTask(sched, t9, ClearSchedOp, sched , CleanUp, NULL);

	SchedRun(sched);*/
	/*retrun after inherent stop*/
	SchedRun(sched);

	AddTask(sched, t8, UnschedPrint, &to_print8 , CleanUp, alloc_var8);
	rmv_op.sched = sched;
	rmv_op.uid =
	AddTask(sched, t11, UnschedPrint, &to_print11 , CleanUp, alloc_var11);
/*	AddTask(sched, t10, RemoveTaskOp, &rmv_op , CleanUp, alloc_var10);
*/
	SchedRun(sched);/*new wave*/

	REPORT(TRUE, SchedIsEmpty(sched), "TestSchedIsEmpty()");
	
	printf("TestSchedRun() tested\n");
	SchedDestroy(sched);
}

/*****************************************************************************/
/******************************* Tester Helper Functions  ********************/
/*****************************************************************************/

/* Function to match data & param */
static int Match(const void* data, void* param)
{
    return *((int*)data) == *((int*)param);
}

/* Function to compare data & param */
static int Compare(const void *data1, const void *data2)
{
	return (*((int*)data1) - *((int*)data2));
}

/* Function to calculate a future time */
static time_t GiveAbsFutureTime(double sec_from_now)
{
    time_t currentTime = time(NULL);
    return currentTime + sec_from_now;
}

/* Function toImplement Wait */
static void Sleep(double remainig)
{
    while(0 < remainig) 
    {
      remainig = sleep(remainig);
    }
}

/*****************************************************************************/
/************************** Sched Wrapper Operations *************************/
/*****************************************************************************/

static int UnschedPrint(void* param)
{
	printf("\tUnschedPrint Func: %d \n", *(int*)param);
	return UNSCHED;
}

static int ReschedPrint(void* param)
{
	printf("\tReschedPrint Func: %d \n", *(int*)param);
	return RESCHED;
}

static int ClearSchedOp(void *param)
{
  SchedClear((sched_t*)param);
  return UNSCHED;
}

static int RunSchedOp(void *sched)
{
  SchedRun((sched_t*)sched);
  return UNSCHED;
}

static int StopSchedOp(void *sched)
{
  SchedStop((sched_t*)sched);
  return UNSCHED;
}

static void CleanUp(void* param)
{
	free(param);
	param = NULL;
}

static int AddTaskOp(void *param)
{
	add_params_t *wrapper = (add_params_t*)param;

	AddTask(wrapper->sched, wrapper->time_to_exec, wrapper->op_func, wrapper->op_params, wrapper->cleanup_func, wrapper->cleanup_params);
	printf("Adding task: \n");
    /*PrintUID(TaskGetUID(to_rmv_task)); fflush(stdout);*/
	return UNSCHED;
}

static int RemoveTaskOp(void *param)
{
	rmv_params_t *wrapper = (rmv_params_t*)param;

	RemoveTask(wrapper->sched, wrapper->uid);
    printf("Removing task: \n");
    /*PrintUID(TaskGetUID(to_rmv_task)); fflush(stdout);*/
	return UNSCHED;
}