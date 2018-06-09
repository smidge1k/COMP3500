
/*****************************************************************************\
* Laboratory Exercises COMP 3500                                              *
* Author: Saad Biaz                                                           *
* Updated 5/22/2017 to distribute to students to do Lab 1                     *
\*****************************************************************************/

/*****************************************************************************\
*                             Global system headers                           *
\*****************************************************************************/


#include "common.h"

/*****************************************************************************\
*                             Global data types                               *
\*****************************************************************************/

typedef enum {TAT,RT,CBT,THGT,WT} Metric;


/*****************************************************************************\
*                             Global definitions                              *
\*****************************************************************************/
#define MAX_QUEUE_SIZE 10
#define FCFS            1
#define SJF             2
#define RR              3


#define MAXMETRICS      5



/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/



/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/

Quantity NumberofJobs[MAXMETRICS]; // Number of Jobs for which metric was collected
Average  SumMetrics[MAXMETRICS]; // Sum for each Metrics
Quantity QueueLength[MAXQUEUES]; // length of each queue

/*****************************************************************************\
*                               Function prototypes                           *
\*****************************************************************************/

void                 ManageProcesses(void);
void                 NewJobIn(ProcessControlBlock whichProcess);
void                 BookKeeping(void);
Flag                 ManagementInitialization(void);
Flag				 RemoveFromQueue(ProcessControlBlock *pcb, Queue whichQueue);
void                 LongtermScheduler(void);
void                 IO();
void                 CPUScheduler(Identifier whichPolicy);
ProcessControlBlock *SJF_Scheduler();
ProcessControlBlock *FCFS_Scheduler();
ProcessControlBlock *RR_Scheduler();
void                 Dispatcher();

/*****************************************************************************\
* function: main()                                                            *
* usage:    Create an artificial environment operating systems. The parent    *
*           process is the "Operating Systems" managing the processes using   *
*           the resources (CPU and Memory) of the system                      *
*******************************************************************************
* Inputs: ANSI flat C command line parameters                                 *
* Output: None                                                                *
*                                                                             *
* INITIALIZE PROGRAM ENVIRONMENT                                              *
* START CONTROL ROUTINE                                                       *
\*****************************************************************************/

int main (int argc, char **argv) {
   if (Initialization(argc,argv)){
     ManageProcesses();
   }
} /* end of main function */

/***********************************************************************\
* Input : none                                                          *
* Output: None                                                          *
* Function: Monitor Sources and process events (written by students)    *
\***********************************************************************/

void ManageProcesses(void){
  ManagementInitialization();
  while (1) {
    IO();
    CPUScheduler(PolicyNumber);
    Dispatcher();
  }
}

/* XXXXXXXXX Do Not Change IO() Routine XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */
/***********************************************************************\
* Input : none                                                          *
* Output: None                                                          *
* Function:                                                             *
*    1) if CPU Burst done, then move process on CPU to Waiting Queue    *
*         otherwise (for RR) return Process to Ready Queue              *
*    2) scan Waiting Queue to find processes with complete I/O          *
*           and move them to Ready Queue                                *
\***********************************************************************/
void IO() {
  ProcessControlBlock *currentProcess = DequeueProcess(RUNNINGQUEUE);
  if (currentProcess){
    if (currentProcess->RemainingCpuBurstTime <= 0) { // Finished current CPU Burst
      currentProcess->TimeEnterWaiting = Now(); // Record when entered the waiting queue
      EnqueueProcess(WAITINGQUEUE, currentProcess); // Move to Waiting Queue
      currentProcess->TimeIOBurstDone = Now() + currentProcess->IOBurstTime; // Record when IO completes
      currentProcess->state = WAITING;
    } else { // Must return to Ready Queue
      currentProcess->JobStartTime = Now();
      EnqueueProcess(READYQUEUE, currentProcess); // Mobe back to Ready Queue
      currentProcess->state = READY; // Update PCB state
    }
  }

  /* Scan Waiting Queue to find processes that got IOs  complete*/
  ProcessControlBlock *ProcessToMove;
  /* Scan Waiting List to find processes that got complete IOs */
  ProcessToMove = DequeueProcess(WAITINGQUEUE);
  if (ProcessToMove){
    Identifier IDFirstProcess =ProcessToMove->ProcessID;
    EnqueueProcess(WAITINGQUEUE,ProcessToMove);
    ProcessToMove = DequeueProcess(WAITINGQUEUE);
    while (ProcessToMove){
      if (Now()>=ProcessToMove->TimeIOBurstDone){
	ProcessToMove->RemainingCpuBurstTime = ProcessToMove->CpuBurstTime;
	ProcessToMove->JobStartTime = Now();
	EnqueueProcess(READYQUEUE,ProcessToMove);
      } else {
	EnqueueProcess(WAITINGQUEUE,ProcessToMove);
      }
      if (ProcessToMove->ProcessID == IDFirstProcess){
	break;
      }
      ProcessToMove =DequeueProcess(WAITINGQUEUE);
    } // while (ProcessToMove)
  } // if (ProcessToMove)
}

/***********************************************************************\
 * Input : whichPolicy (1:FCFS, 2: SJF, and 3:RR)                      *
 * Output: None                                                         *
 * Function: Selects Process from Ready Queue and Puts it on Running Q. *
\***********************************************************************/
void CPUScheduler(Identifier whichPolicy) {
  ProcessControlBlock  *selectedProcess;
  switch(whichPolicy){
    case FCFS : selectedProcess = FCFS_Scheduler();
      break;
    case SJF : selectedProcess = SJF_Scheduler();
      break;
    case RR   : selectedProcess = RR_Scheduler();
  }
  if (selectedProcess) {
    selectedProcess->state = RUNNING; // Process state becomes Running
    EnqueueProcess(RUNNINGQUEUE, selectedProcess); // Put process in Running Queue
  }
}

/***********************************************************************\
 * Input : None                                                         *
 * Output: Pointer to the process based on First Come First Serve (FCFS)*
 * Function: Returns process control block based on FCFS                *
 \***********************************************************************/
ProcessControlBlock *FCFS_Scheduler() {
  /* Select Process based on FCFS */
  ProcessControlBlock *selectedProcess = Queues[READYQUEUE].Tail;
  ProcessControlBlock *topOfRunningQueue = Queues[RUNNINGQUEUE].Tail;
  if (selectedProcess == NULL  || topOfRunningQueue != NULL) {
    return;
  }
  selectedProcess = DequeueProcess(READYQUEUE);
  return(selectedProcess);
}



/***********************************************************************\
 * Input : None                                                         *
 * Output: Pointer to the process with shortest remaining time (SJF)   *
 * Function: Returns process control block with SJF                    *
\***********************************************************************/
ProcessControlBlock *SJF_Scheduler() {  
  /* Select Process with Shortest Remaining Time*/
  ProcessControlBlock *temp = Queues[READYQUEUE].Tail;
  ProcessControlBlock *topOfRunningQueue = Queues[RUNNINGQUEUE].Tail;
  ProcessControlBlock *selectedProcess = temp;
  if (temp == NULL || topOfRunningQueue != NULL) {
	return;
  }
  int minDuration = temp->TotalJobDuration;
  while (temp->previous != NULL) {
    printf("%d\n", temp->ProcessID);
    if (temp->TotalJobDuration < minDuration) {
      minDuration = temp->TotalJobDuration;
      printf("selecting pid: %d duration: %d", temp->ProcessID, temp->TotalJobDuration);
      selectedProcess = temp;
    }
   temp = temp->previous;
  }
  RemoveFromQueue(selectedProcess, READYQUEUE);
  return(selectedProcess);
}


/***********************************************************************\
 * Input : None                                                         *
 * Output: Pointer to the process based on Round Robin (RR)             *
 * Function: Returns process control block based on RR                  *   
 \***********************************************************************/
ProcessControlBlock *RR_Scheduler() {
  /* Select Process based on RR*/
  ProcessControlBlock *selectedProcess = Queues[READYQUEUE].Tail;
  ProcessControlBlock *topOfRunningQueue = Queues[RUNNINGQUEUE].Tail;
  if (selectedProcess == NULL || topOfRunningQueue != NULL) {
      return NULL;
  }
  if (Quantum < selectedProcess->CpuBurstTime) {
      selectedProcess->CpuBurstTime = Quantum;
  }
  EnqueueProcess(RUNNINGQUEUE, selectedProcess);
  DequeueProcess(READYQUEUE);
  
  return(selectedProcess);
}

/***********************************************************************\
 * Input : None                                                         *
 * Output: None                                                         *
 * Function:                                                            *
 *  1)If process in Running Queue needs computation, put it on CPU      *
 *              else move process from running queue to Exit Queue      *
\***********************************************************************/
void Dispatcher() {
  double start;
}

/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This routine is run when a job is added to the Job Queue    *
\***********************************************************************/
void NewJobIn(ProcessControlBlock whichProcess){
  ProcessControlBlock *NewProcess;
  /* Add Job to the Job Queue */
  NewProcess = (ProcessControlBlock *) malloc(sizeof(ProcessControlBlock));
  memcpy(NewProcess,&whichProcess,sizeof(whichProcess));
  NewProcess->TimeInCpu = 0; // Fixes TUX error
  NewProcess->RemainingCpuBurstTime = NewProcess->CpuBurstTime; // SB_ 6/4 Fixes RR
  EnqueueProcess(JOBQUEUE,NewProcess);
  DisplayQueue("Job Queue in NewJobIn",JOBQUEUE);
  LongtermScheduler(); /* Job Admission  */
}

/**********************************************************************\
* Input: None                                                          *
* Output: None                                                         *
* Function:                                                            *
* 1) BookKeeping is called automatically when 250 arrived              *
* 2) Computes and display metrics: average turnaround  time, throughput*
*     average response time, average waiting time in ready queue,      *
*     and CPU Utilization                                              *
\***********************************************************************/
void BookKeeping(void){
  DisplayQueue("Job Queue", JOBQUEUE);
  DisplayQueue("Ready Queue: ", READYQUEUE);
  DisplayQueue("Running Queue: ", RUNNINGQUEUE);
  DisplayQueue("Waiting Queue: ", WAITINGQUEUE);
  DisplayQueue("Exit Queue", EXITQUEUE);
  double end = Now(); // Total time for all processes to arrive
  Metric m;
  // scan and record exit queue
  ProcessControlBlock *temp = Queues[EXITQUEUE].Tail;
  while (temp != NULL && temp->previous != NULL) {
     NumberofJobs[THGT]++;
     SumMetrics[TAT] += temp->JobExitTime - temp->JobArrivalTime;
     SumMetrics[RT] += temp->JobStartTime - temp->JobArrivalTime;
     SumMetrics[CBT] += temp->TimeInCpu;
     temp = temp->previous;
  }
  NumberOfJobs[THGT]++; // record last element of queue
  SumMetrics[TAT] += temp->JobExitTime - temp->JobArrivalTime;
  SumMetrics[RT] += temp->JobStartTime - temp->JobArrivalTime;
  SumMetrics[CBT] += temp->TimeInCpu;
  

  printf("\n********* Processes Managemenent Numbers ******************************\n");
  printf("Policy Number = %d, Quantum = %.6f   Show = %d\n", PolicyNumber, Quantum, Show);
  printf("Number of Completed Processes = %d\n", NumberofJobs[THGT]);
  printf("ATAT=%f   ART=%f  CBT = %f  T=%f AWT=%f\n", 
	 SumMetrics[TAT], SumMetrics[RT], SumMetrics[CBT], 
	 NumberofJobs[THGT]/Now(), SumMetrics[WT]);

  exit(0);
}

/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: Decides which processes should be admitted in Ready Queue   *
*           If enough memory and within multiprogramming limit,         *
*           then move Process from Job Queue to Ready Queue             *
\***********************************************************************/
void LongtermScheduler(void){
  ProcessControlBlock *currentProcess = DequeueProcess(JOBQUEUE);
  while (currentProcess) {
    currentProcess->TimeInJobQueue = Now() - currentProcess->JobArrivalTime; // Set TimeInJobQueue
    currentProcess->JobStartTime = Now(); // Set JobStartTime
    EnqueueProcess(READYQUEUE,currentProcess); // Place process in Ready Queue
    currentProcess->state = READY; // Update process state
    currentProcess = DequeueProcess(JOBQUEUE);
  }
}


/***********************************************************************\
* Input : None                                                          *
* Output: TRUE if Intialization successful                              *
\***********************************************************************/
Flag ManagementInitialization(void){
  Metric m;
  for (m = TAT; m < MAXMETRICS; m++){
     NumberofJobs[m] = 0;
     SumMetrics[m]   = 0.0;
  }
  return TRUE;
}

/***************************************************\
* Input : pointer to selected ProcessControlBlock   *
* 		  Which queue to perform the removal on     *
* Output: TRUE if removal is successful             *
\***************************************************/ 
Flag RemoveFromQueue(ProcessControlBlock *pcb, Queue whichQueue) {
	
	if (pcb != NULL && pcb->previous != NULL) { 
   		if (pcb->next != NULL) { // condition passes if selectedProcess is in middle of queue
        	pcb->previous->next = pcb->next;
        	pcb->next->previous = pcb->previous;
     	} else { // condition passes if selectedProcess is at the tail of the queue
       		Queues[whichQueue].Tail = pcb->previous;
     	}
  	} else if (pcb != NULL) { // condition passes if selectedProcess it at head of queue
    	Queues[whichQueue].Head = pcb->next;
  	} else {
  		return FALSE;
  	}
	
  	return TRUE;
}
