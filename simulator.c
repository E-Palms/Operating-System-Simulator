#include "simulator.h"

// semaphore lock value
static sem_t interruptLock;


//////////////////////////////
//    INTERRUPT MANAGER     //
//////////////////////////////

// adds a blocked process to the interrupt queue
void addToQueue(int *interruptQueue, int processId)
   {
    int index = 0;

    while (interruptQueue[index] != EMPTY_QUEUE_VALUE)
      {
       index++;
      }

    interruptQueue[index] = processId;
   }

// function for running IO on a p_thread
void *handleIO(void *args)
   {    
    // initialize functions and variables
    processType *prcPtr = (processType *) args;
    
    // run the timer for specifed IO time
    runTimer(prcPtr->ioCycleTime * prcPtr->opHeadPtr->intArg2);

    // call the semaphore wait to queue access to critical section
    sem_wait(&interruptLock);
    
    // call the interrupt manager to add an interrupt to the queue
    interruptManager(prcPtr, prcPtr->interruptQueue, IGNORE_INTER_PARAM, ADD_TO_QUEUE);
    
    // call semaphore post to exit critical section
    sem_post(&interruptLock);
    
    // exit the p_thread when finished
    pthread_exit(NULL);
   }

// initializes the interrupt queue
int *initializeQueue(int prcCount)
   {
    // initialize function/variables
    int index, *queue;
    
    // allocate the queue based on the number of processes loaded
    queue = (int *)malloc(prcCount * sizeof(int));
    
    // loop through the interrupt queue
    for (index = 0; index < prcCount; index++)
      {
       // set each value to EMPTY_QUEUE_VALUE constant
       queue[index] = EMPTY_QUEUE_VALUE;
      }
    
    // return the allocated queue
    return queue;
   }

// interrupt manager for handling all interrupt actions
int *interruptManager(processType *prcPtr, int *queue, int queueSize, int interCode)
   {
    // decide which action to execute
    switch (interCode)
      {
       // instructions for initializing the interrupt queue
       case INITIALIZE_QUEUE:
          
          // initialize the queue
          queue = initializeQueue(queueSize);
          
          // loop through the pcb
          while (prcPtr != NULL)
            {
             // point queue pointer in process node to interrupt queue
             prcPtr->interruptQueue = queue;
             
             // point to the next pointer
             prcPtr = prcPtr->nextPtr;
            }
          
          // return the queue
          return queue;
          break;
      
       // instructions for adding to queue
       case ADD_TO_QUEUE:
          
          // call add to queue (a bit redundant but Michael requires a singular handler so here it is)
          addToQueue(queue, prcPtr->processId);

          break;

       // instructions for handling an interrupt
       case HANDLE_INTERRUPT:
          
          // set the queued process state to ready state
          prcPtr->processState = READY_STATE;
          
          // pop the first op command pointed to in the process node
          prcPtr->opHeadPtr =  clearOpCommand(prcPtr->opHeadPtr);
          
          // remove the process from the queue and shift other indexes forward
          popInterruptQueue(queue, queueSize);

          break;
      }

    return NULL;
   }

// removes a blocked process from the interrupt queue
void popInterruptQueue(int *interruptQueue, int queueSize)
   {
    // initialize function/variables
    int index = 0, nextIndex = index +1;
    
    // loop while the end of the array isn't reached
    while (nextIndex < queueSize)
      {
       // set the next index's value to the current index
       interruptQueue[index] = interruptQueue[nextIndex];

       // set the next index's value to EMPTY constant
       interruptQueue[nextIndex] = EMPTY_QUEUE_VALUE;

       // increment both indexes
       index++;
       nextIndex++;
      }
   }


//////////////////////////////
//    LOGFILE FUNCTIONS     //
//////////////////////////////

// adds a node to the log file linked list
logFileType *addLogFileNode(logFileType *ptr, char *command)
   {
    // check if the parameter pointer is null
    if (ptr == NULL)
      {
       // allocate memory for the node
       ptr = (logFileType *)malloc(sizeof(logFileType));
       
       // copy the command parameter into the node
       copyString(ptr->opCommand, command);

       // point the next node pointer to null
       ptr->nextCommand = NULL;
       
       // return the newly allocated node
       return ptr;
      }
    
    // point the next node pointer to recursive function call
    ptr->nextCommand = addLogFileNode(ptr->nextCommand, command);
    
    // return the pointer parameter
    return ptr;
   }

// clears the entire log file linked list
logFileType *clearLogFileStruct(logFileType *logPtr)
   {
    // initialize function/variables
    logFileType *temp;
    
    // check if current pointer points to data
    while (logPtr != NULL)
      {
       // point temp to the next node pointer
       temp = logPtr;
       
       // point to temp to regain linked list head location
       logPtr = logPtr->nextCommand;

       // free the current node
       free(temp);       
      }

    return NULL;
   }

// function for deciding to print or store output
logFileType *displayCommand(logFileType *logPtr, char *toWrite, 
                                                               int displayCode)
   {
    // check the display code type for moitor output
    if (displayCode == LOGTO_MONITOR_CODE || displayCode == LOGTO_BOTH_CODE)
      {
       // print the output
       printf("%s", toWrite);
      }
    
    // check the display code for file output
   if (displayCode == LOGTO_FILE_CODE || displayCode == LOGTO_BOTH_CODE)
      {
       // call function to add command to log file linked list
       logPtr = addLogFileNode(logPtr, toWrite);
      }
    
    // temporary stub return
    return logPtr;
   }

// function for generating OS related output
logFileType *osStringEngine(processType *prcPtr, logFileType *logHeadPtr, 
                            int displayCode, int osCode, int stateCode, int memCode, 
                           char *displayString, bool isPreemptive, bool *lastMsgOS)
   {
    // initialize function/variables
    char currentState[STD_STR_LEN], 
         setState[STD_STR_LEN], 
         outputSegment[MAX_STR_LEN],
         totalTime[MIN_STR_LEN];
    
    // get current runtime of simulation
    accessTimer(LAP_TIMER, totalTime);
    
    // generate timestamp and output type
    sprintf(displayString, "%s, OS: ", totalTime);
    
    // check if the OS output is related to a state change
    if (stateCode != IGNORE_STATE)
      {
       // get the current state of the process
       switch (prcPtr->processState)
         {
          case NEW_STATE:
            copyString(currentState, "NEW");
            break;

          case READY_STATE:
             copyString(currentState, "READY");
             break;

          case RUNNING_STATE:
             copyString(currentState, "RUNNING");
             break;

          case BLOCKED_STATE:
             copyString(currentState, "BLOCKED");
             break;

          case EXIT_STATE:
             copyString(currentState, "EXIT");
             break;
         }
       
       // get the desired state of the process
       switch (stateCode)
         {
          case READY_STATE:
             copyString(setState, "READY");
             break;

          case RUNNING_STATE:
             copyString(setState, "RUNNING");
             break;

          case BLOCKED_STATE:
             copyString(setState, "BLOCKED");
             break;

          case EXIT_STATE:
             copyString(setState, "EXIT");
             break;
         }
       
       // generate state change string
       sprintf(outputSegment, "Process %d set from %s to %s\n", 
                                    prcPtr->processId, currentState, setState);
      }
    // check if OS output is related to the rest of the sim functions
    else if (osCode != IGNORE_OSCODE)
      {
       // determine the type of OS output
       switch (osCode)
         {
          // generate process selection string
          case PROCESS_START:
             sprintf(outputSegment, 
                  "Process %d selected with %d ms remaining\n", 
                                    prcPtr->processId, prcPtr->processRunTime);
             
             break;
          
          // generate process termination string
          case PROCESS_END:
            // check if process termination is related to memory failiure
            if (memCode == ACCESS_FAILIURE || memCode == ALLOCATE_FAILIURE)
               {
                concatenateString(displayString, "Segmentation fault, ");
               }

             sprintf(outputSegment, "Process %d ended\n", prcPtr->processId);
             
             break;
          
          // generate simulation start string
          case SIM_START:
             sprintf(outputSegment, "Simulator Start\n");
             break;
          
          // generate simulation end string
          case SIM_END:
             sprintf(outputSegment, "Simulator End\n");
             break;
          
          // generate system stop string
          case SYS_STOP:
             sprintf(outputSegment, "System Stop\n");
             break;
          
          // generate process interrupt string
          case CPU_INTERRUPT:
             sprintf(outputSegment, 
                        "Interrupted by Process %d, %s %sput operation\n", 
                                    prcPtr->processId, prcPtr->opHeadPtr->strArg1, 
                                                            prcPtr->opHeadPtr->inOutArg);
             break;
          
          // generate process blocked string
          case BLOCKED_IO:
             sprintf(outputSegment, 
                        "Process %d blocked for %sput operation\n",
                              prcPtr->processId, prcPtr->opHeadPtr->inOutArg);
             break;
          
          // generate quantum cycle reached string
          case CPU_QUANTUM:
             sprintf(outputSegment,
                        "Process %d quantum time out, cpu process operation end\n",
                                                                     prcPtr->processId);
             break;
         }
      }
    
    // whatever string is generated, concatenate it to the timestamp part generated at the beginning
    concatenateString(displayString, outputSegment);
    
    // set boolean for last ouput is OS to true. This sets the spacing between OS and process outputs
    *lastMsgOS = true;
    
    // call display command to determine where the generated output goes
    return displayCommand(logHeadPtr, displayString, displayCode);
   }

// function for generating process/op code related output
logFileType *processStringEngine(OpCodeType *opWkgPtr, logFileType *logHeadPtr,
                                          int prcId, int cmdType, int displayCode, 
                                          char *displayString, bool isPreemptive, 
                                                                  bool *lastMsgOS)
   {
    // initialize function/variables
    char outputSegment[STD_STR_LEN], totalTime[MIN_STR_LEN];
    
    // get current runtime of the simulation
    accessTimer(LAP_TIMER, totalTime);
    
    // check if the last output to be generated was OS
    if (*lastMsgOS)
      {
       // if so add an extra \n char for correct spacing between messages
       sprintf(displayString, "\n%s, Process: %d, ", totalTime, prcId);
      }
    // otherwise leave the extra \n out of the string
   else
      {
       sprintf(displayString, "%s, Process: %d, ", totalTime, prcId);
      }
    
    // check for cpu op code
    if (compareString(opWkgPtr->command, "cpu") == STR_EQ)
      {
       // generate cpu start string
       if (cmdType == COMMAND_START)
         {
          sprintf(outputSegment, "cpu process operation start\n");
         }
       // generate cpu end string
       else if (cmdType == COMMAND_END)
         {
          sprintf(outputSegment, "cpu process operation end\n");
         }
      }
    // check for device op code
    else if (compareString(opWkgPtr->command, "dev") == STR_EQ)
      {
       // generate device start string
       if (cmdType == COMMAND_START)
         {
          sprintf(outputSegment, "%s %sput operation start\n", 
                                       opWkgPtr->strArg1, opWkgPtr->inOutArg);
         }
       // check if the scheduling is preemptive
       if (isPreemptive)
         {
          concatenateString(outputSegment, "\n");
         }
       
       // generate device end string
       else if (cmdType == COMMAND_END)
         {
          sprintf(outputSegment, "%s %sput operation end\n", 
                                       opWkgPtr->strArg1, opWkgPtr->inOutArg);
         }
      }
    // otherwise assume memory op code
    else
      {
       // generate the memory allocate/access string
       if (cmdType == COMMAND_START)
         {
          if (compareString(opWkgPtr->strArg1, "allocate") == STR_EQ)
            {
             sprintf(outputSegment, "mem allocate request (%d, %d)\n", 
                                          opWkgPtr->intArg2, opWkgPtr->intArg3);
            }
          else if (compareString(opWkgPtr->strArg1, "access") == STR_EQ)
            {
             sprintf(outputSegment, "mem access request (%d, %d)\n", 
                                          opWkgPtr->intArg2, opWkgPtr->intArg3);
            }
         }
       // generate the memory result string
       else
         {
          switch (cmdType)
            {
             case ALLOCATE_SUCCESS:
                sprintf(outputSegment, "successful mem allocate request\n");
                break;

             case ALLOCATE_FAILIURE:
                sprintf(outputSegment, "failed mem allocate request\n");
                break;

             case ACCESS_SUCCESS:
                sprintf(outputSegment, "successful mem access request\n");
                break;

             case ACCESS_FAILIURE:
                sprintf(outputSegment, "failed mem access request\n");
                break;
            }
         }
      }
    
    // add whatever generated string to the timestamp
    concatenateString(displayString, outputSegment);
    
    // set the last message OS type flag to false
    *lastMsgOS = false;
    
    // call display command to handle string output
    logHeadPtr = displayCommand(logHeadPtr, displayString, displayCode);
    
    // return the log file linked list head pointer
    return logHeadPtr;
   }

// function for writing log file linked list to log file
void writeToFile(logFileType *logPtr, ConfigDataType *cfgPtr, 
                                                            char *displayString)
   {
    // initialize function/variables
    FILE *logFile;
    
    // check for log file output code
    if (cfgPtr->logToCode == LOGTO_FILE_CODE || cfgPtr->logToCode == LOGTO_BOTH_CODE)
      {
       
       // point file pointer to opened/created logfile
       logFile = fopen(cfgPtr->logToFileName, WRITE);
       
       // check if file open was a success
       if (logFile)
         {
          // print all relevant configuration file data
          fprintf(logFile, 
                     "==================================================\n");

          fprintf(logFile, "File Name                       : %s\n", 
                                                      cfgPtr->metaDataFileName);

          configCodeToString(cfgPtr->cpuSchedCode, displayString);

          fprintf(logFile, "CPU Scheduling                  : %s\n", 
                                                               displayString);

          fprintf(logFile, "Quantum Cycles                  : %d\n", 
                                                         cfgPtr->quantumCycles);

          fprintf(logFile, "Memory Available (KB)           : %d\n", 
                                                         cfgPtr->memAvailable);

          fprintf(logFile, "Processor Cycle Rate (ms/cycle) : %d\n", 
                                                         cfgPtr->procCycleRate);

          fprintf(logFile, "I/O Cycle Rate (ms/cycle)       : %d\n", 
                                                         cfgPtr->ioCycleRate);

          fprintf(logFile, "================\n");
          
          // iterate through log file linked list
          while (logPtr != NULL)
            {
             // add each node to the file
             fprintf(logFile, "%s", logPtr->opCommand);
             
             // iterate to the next node
             logPtr = logPtr->nextCommand;
            }
          
          // add simulation end output
          fprintf(logFile, "\nEnd Simulation - Complete\n");

          fprintf(logFile, "=========================\n");
          
          // close the file
          fclose(logFile);
         }
      }
   }


//////////////////////////////
//     MEMORY FUNCTIONS     //
//////////////////////////////

// function for adding a new simulated memory allocation
memoryType *addMemoryNode(memoryType *memPtr, int prcId, int memBase, 
                                                               int memOffset)
   {
    // check if the current parameter pointer is NULL
    if (memPtr == NULL)
      {
       // allocate new memory node
       memPtr = (memoryType *)malloc(sizeof(memoryType));
       
       // assign relevant info from parameters
       memPtr->processId = prcId;

       memPtr->memBase = memBase;

       memPtr->memOffset = memOffset - MEM_SHIFT;
       
       // point the next node pointer to NULL
       memPtr->nextBlock = NULL;
       
       // return the newly allocated node
       return memPtr;
      }
    
    // point the next node pointer to the result of recursive function call
    memPtr->nextBlock = addMemoryNode(memPtr->nextBlock, prcId, memBase, 
                                                                     memOffset);
    
    // return the head pointer
    return memPtr;
   }

// function for clearing entire memory linked list
memoryType *clearMemory(memoryType *memPtr)
   {
    // initialize function/variables
    memoryType *temp;
    
    // iterate through the linked list
    while (memPtr != NULL)
      {
       // point temp pointer to current pointer
       temp = memPtr;

       // point current pointer to next node pointer
       memPtr = memPtr->nextBlock;

       // free the temp pointer
       free(temp);
      }
    
    // return NULL
    return NULL;
   }

// function for identifying valid memory access request
bool findAccess(memoryType *memPtr, int prcId, int accessBase, int accessOffset)
   {
    // initialize function/variables
    int memSize, accessSize = accessBase + accessOffset;

    // iterate through the memory linked list
    while (memPtr != NULL)
      {
       // calculate the size of memory access request
       memSize = memPtr->memBase + memPtr->memOffset;

       // check if the request base is compatible
       if (accessBase >= memPtr->memBase && accessBase < memSize)
         {
          // check if request size is compatible and correct process allocation
          if ((accessSize > accessBase && accessSize <= memSize) 
                                                && prcId == memPtr->processId)
            {
             // return that access request is valid
             return true;
            }
         }
       // point the working pointer to the next node pointer
       memPtr = memPtr->nextBlock;
      }
    
    // return access request not valid
    return false;
   }

// function for identifying valid memory allocation request
bool findConflict(int desiredBase, int desiredOffset, int memAvailable,
                                                         memoryType *memWkgPtr)
   { 
    // initialize function/variables
    int currBase, currSize, 
        desiredSize = desiredBase + desiredOffset - MEM_SHIFT;
    
    // check for allocation request outside of sim memory boundaries
    if (desiredBase >= memAvailable || desiredSize > memAvailable)
      {
       // return conflict found
       return true;
      }      
    
    // loop throught the memory linked list
    while (memWkgPtr != NULL)
      {
       // assign the current base and size to integers for readability
       currBase = memWkgPtr->memBase;
       currSize = currBase + memWkgPtr->memOffset;

       // check if the allocation request conflicts with current node allocation
       if ((desiredBase >= currBase && desiredBase <= (currSize))
                        || (desiredSize >= currBase && desiredSize <= currSize))
         {
          // notify simulation of invalid allocation request
          return true;
         }
       // point the working pointer to the next node pointer
       memWkgPtr = memWkgPtr->nextBlock;
      }
    
    // notify simulation of valid allocation request
    return false;
   }

// function for handling all memory related actions
memoryType *handleMemory(processType *prcCurrent, ConfigDataType *configPtr, 
                                          memoryType *memHeadPtr, int *memCode)
   {
    // determine memory action type
    switch (*memCode)
      {
       // handle allocation request
       case ALLOCATE:
          if (!findConflict(prcCurrent->opHeadPtr->intArg2, 
                                    prcCurrent->opHeadPtr->intArg3,
                                          configPtr->memAvailable, memHeadPtr))
            {
             // store allocation pcb struct as valid request
             memHeadPtr = addMemoryNode(memHeadPtr, prcCurrent->processId,
                                                prcCurrent->opHeadPtr->intArg2,
                                                prcCurrent->opHeadPtr->intArg3);

             // set memory code to ALLOCATE_SUCCESS
             *memCode = ALLOCATE_SUCCESS;
            }
          else
            {
             *memCode = ALLOCATE_FAILIURE;
            }
       break;
       
       // handle access request
       case ACCESS:
          // check if pcb node was allocated
          if (findAccess(memHeadPtr, prcCurrent->processId,
                                             prcCurrent->opHeadPtr->intArg2,
                                                prcCurrent->opHeadPtr->intArg3))
            {
             *memCode = ACCESS_SUCCESS;
            }
          // otherwise
          else
            {
             *memCode = ACCESS_FAILIURE;
            }
       break;
       
       // handle deallocation request
       case DEALLOCATE:
          memHeadPtr = removeBlock(memHeadPtr, prcCurrent);

       break;
       
       // handle deinitialize request
       case DEINITIALIZE:
          memHeadPtr = clearMemory(memHeadPtr);
       break;
      }

    // display output
    if (configPtr->memDisplay && configPtr->logToCode != LOGTO_FILE_CODE)
      {
       memoryStringEngine(memHeadPtr, prcCurrent, configPtr->memAvailable, 
                                                                     memCode);
      }
   
    // return memory head pointer
    return memHeadPtr;
   }

// check if process has memory allocated
bool inMemory(processType *prcPtr, memoryType *memPtr)
   {
    // loop until end of memory linked list
    while (memPtr != NULL)
      {
       // check if memory block is assigned to process
       if (prcPtr->processId == memPtr->processId)
         {
          // notify simulator of valid ownership
          return true;
         }

       // point the working pointer to the next node pointer
       memPtr = memPtr->nextBlock;
      }
    // notify simulation of no ownership found
    return false;
   }

// function for generating memory display
void memoryStringEngine(memoryType *memHeadPtr, processType *prcPtr,
                                                 int totalMemory, int *memCode)
   {
    int memDispBase = 0;

    printf("%s", MEM_LINE);

    switch (*memCode)
      {
       case INITIALIZE:
       printf("After memory initialization\n");
       break;
       
       case ALLOCATE_SUCCESS:
       printf("After allocate success\n");
       break;

       case ALLOCATE_FAILIURE:
       printf("After allocate failiure\n");
       break;

       case ACCESS_SUCCESS:
       printf("After access success\n");
       break;

       case ACCESS_FAILIURE:
       printf("After access failiure\n");
       break;

       case DEALLOCATE:
       printf("After clear process %d success\n", prcPtr->processId);
       break;

       case DEINITIALIZE:
       printf("After clear all process success\nNo memory configured\n");
       break;
      }

    while(memHeadPtr != NULL)
      {
       printf("%d [ Used, P# %d, %d-%d] %d\n", memDispBase, 
                           memHeadPtr->processId, memHeadPtr->memBase, 
                              memHeadPtr->memBase + memHeadPtr->memOffset, 
                                          memDispBase + memHeadPtr->memOffset);

       memDispBase = memDispBase + memHeadPtr->memOffset + MEM_SHIFT;
       memHeadPtr = memHeadPtr->nextBlock;
      }    

    if (*memCode != DEINITIALIZE)
      {
       printf("%d [ Open, P#: x, 0-0 ] %d\n", memDispBase, 
                                                      totalMemory - MEM_SHIFT);
      }
    
    printf("%s", MEM_LINE);
   }

memoryType *removeBlock(memoryType *memHeadPtr, processType *prcPtr)
   {
    memoryType *memWkgPtr, *temp, *prevPtr;

    while (inMemory(prcPtr, memHeadPtr))
      {
       if (memHeadPtr->processId == prcPtr->processId)
         {
          temp = memHeadPtr;
          memHeadPtr = memHeadPtr->nextBlock;
          free(temp);
         }
       else
         {
          memWkgPtr = memHeadPtr->nextBlock;
          prevPtr = memHeadPtr;

          while (memWkgPtr != NULL)
            {
             if (memWkgPtr->processId == prcPtr->processId)
               {
                prevPtr->nextBlock = memWkgPtr->nextBlock;
                free(memWkgPtr);
                memWkgPtr = prevPtr->nextBlock;
               }
             else
               {
                memWkgPtr = memWkgPtr->nextBlock;
                prevPtr = prevPtr->nextBlock;
               }
            }
         }
      }

    return memHeadPtr;
   }


//////////////////////////////
//    PROCESS FUNCTIONS     //
//////////////////////////////
bool allProcessBlocked(processType *prcPtr)
   {
    while (prcPtr != NULL)
      {
       if (prcPtr->processState != BLOCKED_STATE && prcPtr->processState != EXIT_STATE)
         {
          return false;
         }
       prcPtr = prcPtr->nextPtr;
      }
    return true;
   }

bool allProcessExit(processType *prcPtr)
   {
    while (prcPtr != NULL)
      {
       if (prcPtr->processState != EXIT_STATE)
         {
          return false;
         }

       prcPtr = prcPtr->nextPtr;
      }

    return true;
   }

int calculateRunTime(processType *prcPtr, ConfigDataType *cfgPtr)
   {
    int total = 0;
    
    OpCodeType *opPtr = prcPtr->opHeadPtr;

    while (opPtr != NULL)
      {
       if (compareString(opPtr->command, "cpu") == STR_EQ)
         {
          total += cfgPtr->procCycleRate * opPtr->intArg2;
         }
       else if (compareString(opPtr->command, "dev") == STR_EQ)
         {
          total += cfgPtr->ioCycleRate * opPtr->intArg2;
         }
       opPtr = opPtr->nextNode;
      }

    return total;
   }

bool checkPreemptive(ConfigDataType *cfgPtr)
   {
    if (cfgPtr->cpuSchedCode >= CPU_SCHED_SRTF_P_CODE 
                                 && cfgPtr->cpuSchedCode <= CPU_SCHED_RR_P_CODE)
      {
       return true;
      }
    return false;
   }

processType *clearProcesses(processType *prcPtr)
   {
    processType *temp;

    if (prcPtr != NULL)
      {
       temp = prcPtr->nextPtr;

       prcPtr->opHeadPtr = clearMetaDataList(prcPtr->opHeadPtr);

       free(prcPtr->interruptQueue);

       free(prcPtr);

       prcPtr = temp;
      }
    return NULL;
   }

int countProcesses(processType *prcPtr)
   {
    int count = 0;

    while (prcPtr != NULL)
      {
       count++;
       prcPtr = prcPtr->nextPtr;
      }
    return count;
   }

processType *findProcess(processType *prcWkgPtr, int prcToFind)
   {
    while (prcWkgPtr != NULL)
      {
       if (prcWkgPtr->processId == prcToFind)
         {
          return prcWkgPtr;
         }
       prcWkgPtr = prcWkgPtr->nextPtr;
      }
    return NULL;
   }

processType *getProcess(processType *prcPtr, ConfigDataType *cfgPtr, bool *quantumCyclesHit)
   {
    processType *prcToSched = NULL, *prcHead = prcPtr;
    static processType *lastPrc = NULL;

    if (prcPtr != NULL)
      {
       switch (cfgPtr->cpuSchedCode)
         {
          case CPU_SCHED_FCFS_N_CODE:
             while (prcPtr != NULL && prcPtr->processState == EXIT_STATE)
               {
                prcPtr = prcPtr->nextPtr;
               }
             prcToSched = prcPtr;
             break;

          case CPU_SCHED_FCFS_P_CODE:
             while (prcPtr != NULL && prcPtr->processState != READY_STATE && prcPtr->processState != RUNNING_STATE)
               {
                prcPtr = prcPtr->nextPtr;
               }

             prcToSched = prcPtr;

             break;

          case CPU_SCHED_SJF_N_CODE:
             if ( lastPrc != NULL && lastPrc->processState == RUNNING_STATE)
               {
                prcToSched = lastPrc;
               }
             else
               {
                while (prcPtr != NULL)
                  {
                   if (prcPtr->processState == READY_STATE)
                     {
                      prcPtr->processRunTime = calculateRunTime(prcPtr, cfgPtr);

                      if (prcToSched == NULL || prcToSched->processRunTime > 
                                                   prcPtr->processRunTime)
                        {
                         prcToSched = prcPtr;
                        }
                     }
                   prcPtr = prcPtr->nextPtr;
                  }
               }
             break;

          case CPU_SCHED_SRTF_P_CODE:
             while (prcPtr != NULL)
               {
                if (prcPtr->processState == READY_STATE || prcPtr->processState == RUNNING_STATE)
                  {
                   prcPtr->processRunTime = calculateRunTime(prcPtr, cfgPtr);

                   if (prcToSched == NULL || prcToSched->processRunTime > 
                                                   prcPtr->processRunTime)
                     {
                      prcToSched = prcPtr;
                     }
                  }
                prcPtr = prcPtr->nextPtr;
               }

             break;

          case CPU_SCHED_RR_P_CODE:
             if (lastPrc == NULL)
               {
                prcToSched = prcHead;
               }
             else if (*quantumCyclesHit || lastPrc->processState != RUNNING_STATE)
               {
                prcPtr = findProcess(prcHead, lastPrc->processId);

                do
                  {
                   if (prcPtr->nextPtr == NULL)
                     {
                      prcPtr = prcHead;
                     }
                   else
                     {
                      prcPtr = prcPtr->nextPtr;
                     }                   
                  } while (prcPtr->processState != READY_STATE && prcPtr->processState != RUNNING_STATE);

                prcToSched = prcPtr;
                *quantumCyclesHit = false;
               }
             else
               {
                prcToSched = lastPrc;
               }
             break;
         }
      }

    if (lastPrc != NULL)
      {
       if (lastPrc->processId != prcToSched->processId)
         {
          if (lastPrc->processState == RUNNING_STATE)
            {
             lastPrc->processState = READY_STATE;
            }
         }
      }

    lastPrc = prcToSched;
    prcToSched->processRunTime = calculateRunTime(prcToSched, cfgPtr);
    return prcToSched;
   }

processType *initializeProcess(processType *prcPtr, int prcId)
   {
    if (prcPtr == NULL)
      {
       prcPtr = (processType *)malloc(sizeof(processType));
       prcPtr->processId = prcId;
       prcPtr->processRunTime = 0;
       prcPtr->processState = NEW_STATE;
       prcPtr->opHeadPtr = NULL;
       prcPtr->nextPtr = NULL;
       prcPtr->interruptQueue = NULL;
      }
    else
      {
       printf("Error: Process node already occupied.\n");
      }
    return prcPtr;
   }

bool lastProcessAvailable(processType *prcPtr, int totalCount)
   {
    int readyCount = 0, exitCount = 0;

    while (prcPtr != NULL)
      {
       if (prcPtr->processState == READY_STATE || prcPtr->processState == RUNNING_STATE)
         {
          readyCount++;
         }
       else if (prcPtr->processState == EXIT_STATE)
         {
          exitCount++;
         }
      
       prcPtr = prcPtr->nextPtr;
      }
    if (readyCount == 1 && (totalCount - readyCount) == exitCount)
      {
       return true;
      }
    return false;
   }

processType *uploadToPCB(OpCodeType *opWkgPtr, ConfigDataType *cfgPtr, processType *pcbPtr)
   {
    static int processCount = 0;
   
    if (compareString(opWkgPtr->command, "sys") != STR_EQ 
                        && compareString(opWkgPtr->strArg1, "end") != STR_EQ)
      {
       if (pcbPtr == NULL)
         {
          if (compareString(opWkgPtr->command, "app") == STR_EQ 
                        && compareString(opWkgPtr->strArg1, "start")== STR_EQ)
            {
             pcbPtr = initializeProcess(pcbPtr, processCount);

             opWkgPtr = opWkgPtr->nextNode;

             while (compareString(opWkgPtr->command, "app") != STR_EQ 
                        && compareString(opWkgPtr->strArg1, "end") != STR_EQ)
               {
                pcbPtr->opHeadPtr = addNode(pcbPtr->opHeadPtr, opWkgPtr);

                opWkgPtr = opWkgPtr->nextNode;
               }

             pcbPtr->ioCycleTime = cfgPtr->ioCycleRate;

             processCount++;
            }
         }
       pcbPtr->nextPtr = uploadToPCB(opWkgPtr->nextNode, cfgPtr, pcbPtr->nextPtr);
      }
    return pcbPtr;
   }


//////////////////////////////
//     OPCODE FUNCTIONS     //
//////////////////////////////
OpCodeType *clearOpCommand(OpCodeType *opHeadPtr)
   {
    OpCodeType *temp = opHeadPtr;
    if (opHeadPtr != NULL)
      {
       opHeadPtr = opHeadPtr->nextNode;

       free(temp);
      }

    return opHeadPtr;
   }


//////////////////////////////
//     DRIVER FUNCTION      //
//////////////////////////////
void runSim(ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr)
   {
    // initialize function/variables

    // structure head and working ptrs
    logFileType *logFileHeadPtr = NULL;
    processType *prcHeadPtr = NULL, *prcWkgPtr = NULL, *tempPtr = NULL;
    memoryType *memHeadPtr = NULL;
    
    // pthread id
    pthread_t *thread_id;

    // string to store output
    char displayString[MAX_STR_LEN], timeString[MIN_STR_LEN];
    
    // run time in miliseconds, memoryCode storage and number of processes var
    int memoryCode, prcCount, cpuCycleCount = 0;
    
    int *interruptQueue = NULL;
    
    // flags
    bool quantCycleHit = false;

    bool lastMsgOS = false;

    bool isPreemptive = checkPreemptive(configPtr);
    
    // initialize semaphore lock
    sem_init(&interruptLock, 0, 1);

    // check for sytem start command
    if (compareString(metaDataMstrPtr->command, "sys") == STR_EQ 
               && compareString(metaDataMstrPtr->strArg1, "start") == STR_EQ)
      {
       // check for file output and notify user
       if (configPtr->logToCode == LOGTO_FILE_CODE)
         {
          // print file output notice
          printf("Writing output to file!\n");
         }
       
       // display begin simulation output
       sprintf(displayString, "Begin Simulation\n\n");
       logFileHeadPtr = displayCommand(logFileHeadPtr, displayString, 
                                                         configPtr->logToCode);       

       // get op codes and organize them by process
       prcHeadPtr = uploadToPCB(metaDataMstrPtr->nextNode, configPtr, prcHeadPtr);

       // get the number of processes loaded in the pcb
       prcCount = countProcesses(prcHeadPtr);
       
       // allocate the interrupt queue with the number of processes available 
       // as that is the maximum amount
       interruptQueue = interruptManager(prcHeadPtr, interruptQueue, prcCount, INITIALIZE_QUEUE);
       
       thread_id = malloc(sizeof(pthread_t) * prcCount);

       // set a working pointer to the pcb head ptr
       prcWkgPtr = prcHeadPtr;
       
       accessTimer(ZERO_TIMER, timeString);

       // display simulator start
       logFileHeadPtr = osStringEngine(prcWkgPtr, logFileHeadPtr, configPtr->logToCode, 
                                                SIM_START, IGNORE_STATE, IGNORE_MEM_CODE, 
                                                                   displayString, isPreemptive, &lastMsgOS);
       
       // ready all new processes in the pcb
       while (prcWkgPtr != NULL)
         {
          // display process state change to ready
          logFileHeadPtr = osStringEngine(prcWkgPtr, logFileHeadPtr, configPtr->logToCode,
                                                   IGNORE_OSCODE, READY_STATE, IGNORE_MEM_CODE, 
                                                                         displayString, isPreemptive, &lastMsgOS);
          
          // change state value within process node
          prcWkgPtr->processState = READY_STATE;
          
          // point the process working pointer to the next node
          prcWkgPtr = prcWkgPtr->nextPtr;
         }

       // set the memory code to initialize
       memoryCode = INITIALIZE;
       
       // initialize memory
       memHeadPtr = handleMemory(prcHeadPtr, configPtr, memHeadPtr, &memoryCode);
       
       // master loop (loops while all processes are not exit state)
       while (!allProcessExit(prcHeadPtr))
         {
          
          if (allProcessBlocked(prcHeadPtr))
            {
             accessTimer(LAP_TIMER, timeString);

             sprintf(displayString, "%s, OS: CPU idle, all active processes blocked\n", timeString);

             logFileHeadPtr = displayCommand(logFileHeadPtr, displayString, configPtr->logToCode);

             while (interruptQueue[FIRST_INDEX] == EMPTY_QUEUE_VALUE)
               {
                // enjoy the wait, play some checkers really fast or something
               }

             accessTimer(LAP_TIMER, timeString);

             sprintf(displayString, "%s, OS: CPU interrupt, end idle\n", timeString);
             
             logFileHeadPtr = displayCommand(logFileHeadPtr, displayString, configPtr->logToCode);
             
             prcWkgPtr = findProcess(prcHeadPtr, interruptQueue[FIRST_INDEX]);

             logFileHeadPtr = osStringEngine(prcWkgPtr, logFileHeadPtr, configPtr->logToCode, 
                                             CPU_INTERRUPT, IGNORE_STATE, IGNORE_MEM_CODE, displayString, isPreemptive, &lastMsgOS);

             logFileHeadPtr = osStringEngine(prcWkgPtr, logFileHeadPtr, configPtr->logToCode, 
                                             IGNORE_OSCODE, READY_STATE, IGNORE_MEM_CODE, displayString, isPreemptive, &lastMsgOS);

             interruptManager(prcWkgPtr, interruptQueue, prcCount, HANDLE_INTERRUPT);
            }

         
          prcWkgPtr = getProcess(prcHeadPtr, configPtr, &quantCycleHit);


          if ( prcWkgPtr->processState == READY_STATE)
            {
             
             accessTimer(LAP_TIMER, timeString);

             sprintf(displayString, 
                        "%s, OS: Process %d selected with %d ms remaining\n", 
                                                timeString, prcWkgPtr->processId, prcWkgPtr->processRunTime);

             logFileHeadPtr = displayCommand(logFileHeadPtr, displayString, configPtr->logToCode);

             logFileHeadPtr = osStringEngine(prcWkgPtr, logFileHeadPtr, configPtr->logToCode,
                                           IGNORE_OSCODE, RUNNING_STATE, IGNORE_MEM_CODE, 
                                                                     displayString, isPreemptive, &lastMsgOS);
            }

          prcWkgPtr->processState = RUNNING_STATE;


          if (prcWkgPtr->opHeadPtr != NULL)
            {
             logFileHeadPtr = processStringEngine(prcWkgPtr->opHeadPtr, logFileHeadPtr, 
                                                                prcWkgPtr->processId, COMMAND_START, 
                                                                        configPtr->logToCode, displayString, isPreemptive, &lastMsgOS);

             if (compareString(prcWkgPtr->opHeadPtr->command, "dev") == STR_EQ)
               {
                if (isPreemptive)
                  {
                   logFileHeadPtr = osStringEngine(prcWkgPtr, logFileHeadPtr, configPtr->logToCode, 
                                                         BLOCKED_IO, IGNORE_STATE, IGNORE_MEM_CODE, 
                                                                               displayString, isPreemptive, &lastMsgOS);

                   logFileHeadPtr = osStringEngine(prcWkgPtr, logFileHeadPtr, configPtr->logToCode, 
                                                         IGNORE_OSCODE, BLOCKED_STATE, IGNORE_MEM_CODE, 
                                                                               displayString, isPreemptive, &lastMsgOS);
                   prcWkgPtr->processState = BLOCKED_STATE;
                
                   pthread_create(&thread_id[prcWkgPtr->processId], NULL, handleIO, (void *)prcWkgPtr);
                  }
                else
                  {
                   runTimer(prcWkgPtr->ioCycleTime * prcWkgPtr->opHeadPtr->intArg2);

                   logFileHeadPtr = processStringEngine(prcWkgPtr->opHeadPtr, logFileHeadPtr,
                                                                        prcWkgPtr->processId, COMMAND_END,
                                                                                configPtr->logToCode, displayString, isPreemptive, &lastMsgOS);
                   prcWkgPtr->opHeadPtr = clearOpCommand(prcWkgPtr->opHeadPtr);
                  }
               }
             else if (compareString(prcWkgPtr->opHeadPtr->command, "mem") == STR_EQ)
               {
                if(compareString(prcWkgPtr->opHeadPtr->strArg1, "allocate") 
                                                                     == STR_EQ)
                  {
                   memoryCode = ALLOCATE;
                  }

                else if(compareString(prcWkgPtr->opHeadPtr->strArg1, "access") 
                                                                     == STR_EQ)
                  {
                   memoryCode = ACCESS;
                  }

                memHeadPtr = handleMemory(prcWkgPtr, configPtr, memHeadPtr, 
                                                                  &memoryCode);

                logFileHeadPtr = processStringEngine(prcWkgPtr->opHeadPtr, 
                                                     logFileHeadPtr,  prcWkgPtr->processId, memoryCode, 
                                                      configPtr->logToCode, displayString, isPreemptive, &lastMsgOS);

                prcWkgPtr->opHeadPtr = clearOpCommand(prcWkgPtr->opHeadPtr);
               }

             else if (compareString(prcWkgPtr->opHeadPtr->command, "cpu") == STR_EQ)
               {
                //printf("\n-- CPU Cycle Count Before While Loop: %d --\n\n", cpuCycleCount);
                if (isPreemptive)
                  {
                   while (cpuCycleCount != configPtr->quantumCycles 
                     && prcWkgPtr->opHeadPtr->intArg2 != EMPTY_CYCLE_COUNT
                     && interruptQueue[FIRST_INDEX] == EMPTY_QUEUE_VALUE)
                     {
                      runTimer(configPtr->procCycleRate);
                      cpuCycleCount++;
                      prcWkgPtr->opHeadPtr->intArg2--;
                     }
                  }
                else
                  {
                   runTimer(configPtr->procCycleRate * prcWkgPtr->opHeadPtr->intArg2);
                   prcWkgPtr->opHeadPtr->intArg2 = EMPTY_CYCLE_COUNT;
                  }
                   
                //printf("\n-- CPU Cycle Count After While Loop: %d --\n\n", cpuCycleCount);

                //printf("\n-- Process Id: %d, Cycles Left: %d --\n\n", prcWkgPtr->processId, prcWkgPtr->opHeadPtr->intArg2);

                if (interruptQueue[FIRST_INDEX] != EMPTY_QUEUE_VALUE && isPreemptive)
                  {
                   logFileHeadPtr = processStringEngine(prcWkgPtr->opHeadPtr, logFileHeadPtr,  
                                                                    prcWkgPtr->processId, COMMAND_END, 
                                                                        configPtr->logToCode, displayString, isPreemptive, &lastMsgOS);

                   tempPtr = findProcess(prcHeadPtr, interruptQueue[FIRST_INDEX]);
                   
                   logFileHeadPtr = displayCommand(logFileHeadPtr, "\n", configPtr->logToCode);

                   logFileHeadPtr = osStringEngine(tempPtr, logFileHeadPtr, configPtr->logToCode, 
                                                      CPU_INTERRUPT, IGNORE_STATE, IGNORE_MEM_CODE, 
                                                                               displayString, isPreemptive, &lastMsgOS);                     

                   logFileHeadPtr = osStringEngine(tempPtr, logFileHeadPtr, configPtr->logToCode, 
                                                      IGNORE_OSCODE, READY_STATE, IGNORE_MEM_CODE, 
                                                                               displayString, isPreemptive, &lastMsgOS);  
                     
                   interruptManager(tempPtr, interruptQueue, prcCount, HANDLE_INTERRUPT);
                   
                  }
                else if (cpuCycleCount == configPtr->quantumCycles && prcWkgPtr->opHeadPtr->intArg2 != EMPTY_CYCLE_COUNT && isPreemptive)
                  {
                   logFileHeadPtr = displayCommand(logFileHeadPtr, "\n", configPtr->logToCode);
                  
                   logFileHeadPtr = osStringEngine(prcWkgPtr, logFileHeadPtr, configPtr->logToCode, 
                                                         CPU_QUANTUM, IGNORE_STATE, IGNORE_MEM_CODE, 
                                                                            displayString, isPreemptive, &lastMsgOS);
                  }
                else if (prcWkgPtr->opHeadPtr->intArg2 == EMPTY_CYCLE_COUNT)
                  {
                   logFileHeadPtr = processStringEngine(prcWkgPtr->opHeadPtr, logFileHeadPtr, 
                                                                    prcWkgPtr->processId, COMMAND_END, 
                                                                        configPtr->logToCode, displayString, isPreemptive, &lastMsgOS);
                   prcWkgPtr->opHeadPtr = clearOpCommand(prcWkgPtr->opHeadPtr);
                  }
               
                if (isPreemptive && cpuCycleCount == configPtr->quantumCycles)
                  {
                   cpuCycleCount = 0;
                   quantCycleHit = true;
                  }
               }
            }

          if (prcWkgPtr->opHeadPtr == NULL || memoryCode == ALLOCATE_FAILIURE || memoryCode == ACCESS_FAILIURE)
            {
             logFileHeadPtr = displayCommand(logFileHeadPtr, "\n", configPtr->logToCode);

             logFileHeadPtr = osStringEngine(prcWkgPtr, logFileHeadPtr, configPtr->logToCode, 
                                                      PROCESS_END, IGNORE_STATE, memoryCode, 
                                                                            displayString, isPreemptive, &lastMsgOS);
             
             memoryCode = DEALLOCATE;

             memHeadPtr = handleMemory(prcWkgPtr, configPtr, memHeadPtr, &memoryCode);

             
             logFileHeadPtr = osStringEngine(prcWkgPtr, logFileHeadPtr, configPtr->logToCode, 
                                                      IGNORE_OSCODE, EXIT_STATE, IGNORE_MEM_CODE, 
                                                                            displayString, isPreemptive, &lastMsgOS);
             
             prcWkgPtr->processState = EXIT_STATE;
            }         
         }

       // display system stop
       logFileHeadPtr = osStringEngine(prcWkgPtr, logFileHeadPtr, 
                            configPtr->logToCode, SYS_STOP, IGNORE_STATE, 
                                       IGNORE_MEM_CODE,displayString, isPreemptive, &lastMsgOS);

       prcHeadPtr = clearProcesses(prcHeadPtr);

       memoryCode = DEINITIALIZE;

       memHeadPtr = handleMemory(NULL, configPtr, memHeadPtr, &memoryCode);
       
       sem_destroy(&interruptLock);

       free(thread_id);

       // display simulator end
       logFileHeadPtr = osStringEngine(prcHeadPtr, logFileHeadPtr, 
                            configPtr->logToCode, SIM_END, IGNORE_STATE, 
                                    IGNORE_MEM_CODE, displayString, isPreemptive, &lastMsgOS);

       accessTimer(STOP_TIMER, timeString);

       // write stored output to file 
       writeToFile(logFileHeadPtr, configPtr, displayString);

       logFileHeadPtr = clearLogFileStruct(logFileHeadPtr);
      }
   }