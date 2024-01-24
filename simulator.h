#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <semaphore.h>
#include "configops.h"
#include "metadataops.h"
#include "stringUtil.h"
#include "datatypes.h"
#include "simtimer.h"

// process control board
typedef struct processStruct
   {
    // process id
    int processId;

    // process state
    int processState;

    // process run time
    int processRunTime;
    
    // on board io cycle runtime
    int ioCycleTime;

    // points to op code
    OpCodeType *opHeadPtr;

    // next process pointer
    struct processStruct *nextPtr;

    // pointer to the interrupt queue
    int *interruptQueue;

   } processType;

// memory data structure
typedef struct memoryStruct
   {
    // process id
    int processId;
    
    // base value for the memory block
    int memBase;

    // offset value for the size of the memory block
    int memOffset;

    // points to the next memory node in ll
    struct memoryStruct *nextBlock;
    
   } memoryType;


// log file linked list for storing output
typedef struct logFileStruct
   {
    // string storage
    char opCommand[MAX_STR_LEN];
    
    // pointer to next node
    struct logFileStruct *nextCommand;

   } logFileType;

// enum struct for OS related operations/identifiers
typedef enum { PROCESS_START  = -1,
               PROCESS_END    = -2,
               COMMAND_START  = -3,
               COMMAND_END    = -4,
               BLOCKED_IO     = -5,
               SIM_START      = -6,
               SIM_END        = -7,
               SYS_STOP       = -8,
               IGNORE_OSCODE  = -9,
               CPU_INTERRUPT  = -10,
               CPU_QUANTUM    = -11 } OsCodes;

// enum struct for Memory related operations/identifiers
typedef enum { INITIALIZE,
               ALLOCATE,
               ALLOCATE_SUCCESS,
               ALLOCATE_FAILIURE,
               ACCESS,
               ACCESS_SUCCESS,
               ACCESS_FAILIURE,
               DEALLOCATE,
               DEINITIALIZE,
               IGNORE_MEM_CODE } MemCodes;

typedef enum { INITIALIZE_QUEUE,
               ADD_TO_QUEUE,
               HANDLE_INTERRUPT,
               IGNORE_INTER_PARAM } InterrCodes;

// function prototypes

//////////////////////////////
//    INTERRUPT MANAGER     //
//////////////////////////////
void addToQueue(int *interruptQueue, int processId);

void *handleIO(void *args);

int *initializeQueue(int prcCount);

int *interruptManager(processType *prcPtr, int *queue, int queueSize, int interCode);

void popInterruptQueue(int *interruptQueue, int queueSize);


//////////////////////////////
//    LOGFILE FUNCTIONS     //
//////////////////////////////
logFileType *addLogFileNode(logFileType *ptr, char *command);

logFileType *clearLogFileStruct(logFileType *logPtr);

logFileType *displayCommand(logFileType *logPtr, char *toWrite, int displayCode);

logFileType *osStringEngine(processType *prcPtr, logFileType *logHeadPtr, 
                                 int displayCode, int osCode, int stateCode, int memCode,
                                           char *displayString, bool isPreemptive, bool *lastMsgOS);

logFileType *processStringEngine(OpCodeType *opWkgPtr, logFileType *logHeadPtr,
                                          int prcId, int cmdtype, int displayCode, 
                                             char *displayString, bool isPreemptive, bool *lastMsgOS);

void writeToFile(logFileType *logPtr, ConfigDataType *cfgPtr, char *displayString);


//////////////////////////////
//     MEMORY FUNCTIONS     //
//////////////////////////////
memoryType *addMemoryNode(memoryType *memPtr, int prcId, int memBase, int memOffset);

memoryType *clearMemory(memoryType *memPtr);

bool findAccess(memoryType *memPtr, int prcId, int accessBase, int accessOffset);

bool findConflict(int desiredBase, int desiredOffset, int memAvailable, memoryType *memWkgPtr);

memoryType *handleMemory(processType *prcCurrent, ConfigDataType *configPtr, 
                                                  memoryType *memHeadPtr, int *memCode);

bool inMemory(processType *prcPtr, memoryType *memPtr);

void memoryStringEngine(memoryType *memHeadPtr, processType *prcPtr,
                                                 int totalMemory, int *memCode);

memoryType *removeBlock(memoryType *memHeadPtr, processType *prcPtr);


//////////////////////////////
//    PROCESS FUNCTIONS     //
//////////////////////////////
bool allProcessBlocked(processType *prcPtr);

bool allProcessExit(processType *prcPtr);

int calculateRunTime(processType *prcPtr, ConfigDataType *cfgPtr);

bool checkPreemptive(ConfigDataType *cfgPtr);

processType *clearProcesses(processType *prcPtr);

int countProccesses(processType *prcPtr);

processType *findProcess(processType *prcWkgPtr, int prcToFind);

processType *getProcess(processType *prcHeadPtr, ConfigDataType *cfgPtr, bool *quantumCyclesHit);

processType *initializeProcess(processType *prcPtr, int prcId);

bool lastProcessAvailable(processType *prcPtr, int totalCount);

processType *uploadToPCB(OpCodeType *opWkgPtr, ConfigDataType *cfgPtr, processType *pcbPtr);


//////////////////////////////
//     OPCODE FUNCTIONS     //
//////////////////////////////
OpCodeType *clearOpCommand(OpCodeType *opHeadPtr);


//////////////////////////////
//     DRIVER FUNCTION      //
//////////////////////////////
void runSim(ConfigDataType *configPtr, OpCodeType *metaDataMstrPtr);


#endif // SIMULATOR_H