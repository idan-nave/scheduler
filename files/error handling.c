/********************************** ws_x.c  ******************************/
/*
  Name: Idan Nave
  Project: Work Sheet #
  Reviewer: ???
  Date: 00/00/23
  Version: 1.0
*/
/******************************** Libraries  **********************************/

#include <assert.h> 		/* assert() */
#include <stdio.h>  		/* FILEs, stderr, stdin, stdout */
#include <stdlib.h> 		/* malloc(), free() */
#include <string.h>             /* string manipulation */
#include <errno.h>              /* errno global */
#include <time.h>               /* strftime, localtime, difftime */
#include <ctype.h>              /* isalpha, isdigit, isspace */
#include <stddef.h>             /* size_t, NULL */

#include "ws_x.h" 		

/******************************** Definitions  ********************************/

/* Array Lengths' Definition */
#define ARR2_LEN 10	  /* Length of multi-type array */


/******************************************************************************/
/**************************** App's Data-structures ***************************/
/******************************************************************************/
 

  /******************************** TypeDefs  *******************************/


  /******************************** GLobals  ********************************/


/******************************************************************************/
/**************** Forward Declarations of Static Functions ********************/
/******************************************************************************/
 
  /*********************** Memory Allocations Functions *********************/

/* A Function to allocate program resources */
static int AllocateResources();
/* A Function to free allocated program resources */
static void FreeResources();


  /************************ Initialization Functions ************************/



  /****************************** Test Functions ****************************/




/******************************************************************************/
/*************** API Implementation - Functions Definitions *******************/
/******************************************************************************/
 
  /**************** App Manager Function (called from Main) *****************/
 
int ExecuteApp()
{


/* A Function to allocate program resources */
AllocateResources();


const char* errorLogPath = "../files/error_log.txt";
if (redirect_stderr_to_file(errorLogPath))
{
    // Now, standard error output will go to ../files/error_log.txt

    log_error("This is an error message.");
    log_error("Error code: 42");

    fclose(stderr);
}
  
/* A Function to free allocated program resources */
FreeResources();

    
}

  /****************** App Resources-allocating Managers ********************/
  
static int AllocateResources()
{   
   
}

static void FreeResources()
{

}

  /************************ Initialization Functions ************************/
  
  
  
  
  
  
  /****************************** Test Functions ****************************/
  
  
  /*************************** Error Handling Functions *********************/
int RedirectStderrToFile(const char* filepath)
{
    FILE* err_file = fopen(filepath, "a"); /* Append mode */
    if (err_file != NULL)
    {
        int fd = fileno(err_file);
        if (dup2(fd, fileno(stderr)) == -1) 
        {
            perror("Error redirecting stderr");
            fclose(err_file);
            return 0;                    /* Failure */
        }
        return 1;                        /* Success */
    
    else
    {
        perror("Error opening file");
        return 0;                        /* Failure */
    }
}

void LogErr(const char* message
{
    time_t rawtime;
    struct tm* timeinfo;
    char timestamp[20];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S] ", timeinfo);

    fprintf(stderr, "%s%s\n", timestamp, message);
}

